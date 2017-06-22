#include "fileoperation.h"
#include "fileconflictresolver.h"
#include "trash.h"
#include "notifier.h"
#include "debug.h"

#include <QDir>
#include <QtConcurrentRun>
#include <QApplication>

namespace
{

QString uniqueFileName (const QFileInfo &info)
{
  auto dir = info.absoluteDir ();
  const auto baseName = info.baseName ();
  const auto extension = info.completeSuffix ();
  auto attempt = 0;
  auto target = info.fileName ();
  while (dir.exists (target))
  {
    target = baseName + '.' + QString::number (++attempt) + '.' + extension;
  }
  return target;
}

FileOperation::Infos dirEntries (const QFileInfo &info)
{
  QDir dir (info.absoluteFilePath ());
  return dir.entryInfoList (QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
}

bool createDir (QDir parent, const QString &name)
{
  if (!parent.exists (name) && !parent.mkdir (name))
  {
    Notifier::error (QObject::tr ("Failed to create directory ") + parent.absoluteFilePath (name));
    return false;
  }
  return true;
}

bool removeDir (const QFileInfo &info)
{
  auto parent = info.absoluteDir ();
  if (!parent.rmdir (info.fileName ()))
  {
    Notifier::error (QObject::tr ("Failed to remove directory ") + info.absoluteFilePath ());
    return false;
  }
  return true;
}

}

FileOperation::FileOperation () :
  sources_ (),
  target_ (),
  action_ (),
  resolver_ (nullptr),
  allResolution_ (FileConflictResolver::Pending)
{

}

FileOperation::Ptr FileOperation::paste (const QList<QUrl> &urls, const QFileInfo &target,
                                         Qt::DropAction action)
{
  auto result = QSharedPointer<FileOperation>::create ();
  for (const auto &i: urls)
  {
    result->sources_ << QFileInfo (i.toLocalFile ());
  }
  result->target_ = target;
  result->action_ = QMap<Qt::DropAction,Action>{
    {Qt::CopyAction, Action::Copy}, {Qt::MoveAction, Action::Move}, {Qt::LinkAction, Action::Link}
  }.value (action, Action::Copy);
  return result;
}

FileOperation::Ptr FileOperation::remove (const QList<QFileInfo> &infos)
{
  auto result = QSharedPointer<FileOperation>::create ();
  result->sources_ = infos;
  result->action_ = Action::Remove;
  return result;
}

FileOperation::Ptr FileOperation::trash (const QList<QFileInfo> &infos)
{
  auto result = QSharedPointer<FileOperation>::create ();
  result->sources_ = infos;
  result->action_ = Action::Trash;
  return result;
}

const QList<QFileInfo> &FileOperation::sources () const
{
  return sources_;
}

const QFileInfo &FileOperation::target () const
{
  return target_;
}

const FileOperation::Action &FileOperation::action () const
{
  return action_;
}

void FileOperation::startAsync (FileConflictResolver *resolver)
{
  ASSERT (resolver);
  resolver_ = resolver;
  switch (action_)
  {
    case FileOperation::Action::Copy:
    case FileOperation::Action::Move:
      QtConcurrent::run (this, &FileOperation::transfer, sources_, target_, 0);
      break;

    case FileOperation::Action::Link:
      QtConcurrent::run (this, &FileOperation::link, sources_, target_);
      break;

    case FileOperation::Action::Remove:
    case FileOperation::Action::Trash:
      QtConcurrent::run (this, &FileOperation::erase, sources_, 0);
      break;
  }
}

bool FileOperation::transfer (const FileOperation::Infos &sources, const QFileInfo &target,
                              int depth)
{
  auto ok = true;
  QDir targetDir (target.absoluteFilePath ());
  for (const auto &source: sources)
  {
    auto name = source.fileName ();
    QFileInfo targetFile (targetDir.absoluteFilePath (name));
    if (targetFile.exists () && !targetFile.isDir ())
    {
      const auto resolution = resolveConflict (source, targetFile);
      if (resolution & FileConflictResolver::Abort)
      {
        ok = false;
        break;
      }
      if (resolution & FileConflictResolver::Target)
      {
        continue;
      }
      if (resolution & FileConflictResolver::Source && !removeDir (targetFile))
      {
        ok = false;
        continue;
      }
      if (resolution & FileConflictResolver::Rename)
      {
        name = uniqueFileName (targetFile);
      }
    }

    if (source.isDir ())
    {
      const auto removeSource = action_ == FileOperation::Action::Move;
      if (createDir (targetDir, name)
          && transfer (dirEntries (source), targetDir.absoluteFilePath (name), depth + 1)
          && ((removeSource && removeDir (source)) || !removeSource))
      {
        continue;
      }
      ok = false;
      if (allResolution_ & FileConflictResolver::Abort)
      {
        break;
      }
      continue;
    }

    switch (action_)
    {
      case FileOperation::Action::Copy:
        ok &= QFile::copy (source.absoluteFilePath (), targetDir.absoluteFilePath (name));
        break;

      case FileOperation::Action::Move:
        ok &= QFile::rename (source.absoluteFilePath (), targetDir.absoluteFilePath (name));
        break;

      default:
        ASSERT_X (false, "wrong switch");
    }
  }

  if (!depth)
  {
    emit finished (ok);
  }
  return ok;
}

bool FileOperation::link (const FileOperation::Infos &sources, const QFileInfo &target)
{
  auto ok = true;
  QDir targetDir (target.absoluteFilePath ());
  for (const auto &source: sources)
  {
    auto name = source.fileName ();
    if (targetDir.exists (name))
    {
      ok = false;
      Notifier::error (tr ("Target already exists ") + targetDir.absoluteFilePath (name));
      continue;
    }

    ok &= QFile::link (source.absoluteFilePath (), targetDir.absoluteFilePath (name));
  }
  emit finished (ok);
  return ok;
}

bool FileOperation::erase (const FileOperation::Infos &infos, int depth)
{
  auto ok = true;
  for (const auto &i: infos)
  {
    switch (action_)
    {
      case FileOperation::Action::Remove:
        if (!i.isDir ())
        {
          ok &= QFile::remove (i.absoluteFilePath ());
        }
        else
        {
          const auto erased = erase (dirEntries (i), depth + 1);
          ok &= (erased ? removeDir (i) : false);
        }
        break;

      case FileOperation::Action::Trash:
        ok &= Trash::trash (i);
        break;

      default:
        ASSERT_X (false, "wrong switch");
    }
  }

  if (!depth)
  {
    emit finished (ok);
  }
  return ok;
}

int FileOperation::resolveConflict (const QFileInfo &source, const QFileInfo &target)
{
  static QMutex mutex;
  QMutexLocker locker (&mutex); // only one request for all threads

  auto resolution = allResolution_;
  if (resolution == FileConflictResolver::Pending)
  {
    auto connection = (QThread::currentThread () == qApp->thread ())
                      ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    QMetaObject::invokeMethod (resolver_, "resolve", connection,
                               Q_ARG (QFileInfo, source),
                               Q_ARG (QFileInfo, target),
                               Q_ARG (int, int(action_)),
                               Q_ARG (int *, &resolution));
    if (resolution & FileConflictResolver::All || resolution & FileConflictResolver::Abort)
    {
      allResolution_ = resolution;
    }
  }

  return resolution;
}
