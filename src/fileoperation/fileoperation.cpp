#include "fileoperation.h"
#include "fileconflictresolver.h"
#include "trash.h"
#include "notifier.h"
#include "debug.h"
#include "utils.h"
#include "constants.h"
#include "storagemanager.h"

#include <QDir>
#include <QtConcurrentRun>
#include <QApplication>

#include <array>

namespace
{

QString uniqueFileName (const QFileInfo &info)
{
  auto dir = info.absoluteDir ();
  const auto baseName = info.baseName ();
  const auto extension = info.completeSuffix ().isEmpty () ? QString ()
                                                           : '.' + info.completeSuffix ();;
  auto attempt = 0;
  auto target = info.fileName ();
  while (dir.exists (target))
  {
    target = baseName + '.' + QString::number (++attempt) + extension;
  }
  return target;
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

bool removeFile (const QFileInfo &info)
{
  if (!QFile::remove (info.absoluteFilePath ()))
  {
    Notifier::error (QObject::tr ("Failed to remove file ") + info.absoluteFilePath ());
    return false;
  }
  return true;
}
bool removeInfo (const QFileInfo &info);

bool removeDir (const QFileInfo &info)
{
  for (const auto &i: utils::dirEntries (info))
  {
    if (!removeInfo (i))
    {
      return false;
    }
  }

  auto parent = info.absoluteDir ();
  if (!parent.rmdir (info.fileName ()))
  {
    Notifier::error (QObject::tr ("Failed to remove directory ") + info.absoluteFilePath ());
    return false;
  }

  return true;
}

bool removeInfo (const QFileInfo &info)
{
  return info.isDir () ? removeDir (info) : removeFile (info);
}

}

FileOperation::FileOperation () :
  sources_ (),
  target_ (),
  action_ (),
  resolver_ (nullptr),
  allFileResolution_ (FileConflictResolver::Pending),
  allDirResolution_ (FileConflictResolver::Pending),
  totalSize_ (1),
  doneSize_ (0),
  progress_ (0),
  isAborted_ (false)
{

}

void FileOperation::startAsync (FileConflictResolver *resolver)
{
  ASSERT (resolver);
  resolver_ = resolver;
  if (action_ != FileOperation::Action::Link)
  {
    for (const auto &i: sources_)
    {
      totalSize_ += utils::totalSize (i);
    }
  }
  else
  {
    totalSize_ = sources_.size ();
  }

  switch (action_)
  {
    case FileOperation::Action::Copy:
    case FileOperation::Action::Move:
      if (!target_.exists () && sources_.size () > 1)
      {
        QDir d;
        d.mkpath (target_.absoluteFilePath ());
      }
      QtConcurrent::run (this, &FileOperation::transfer, sources_, target_, 0);
      break;

    case FileOperation::Action::Link:
      if (!target_.exists () && sources_.size () > 1)
      {
        QDir d;
        d.mkpath (target_.absoluteFilePath ());
      }
      QtConcurrent::run (this, &FileOperation::link, sources_, target_);
      break;

    case FileOperation::Action::Remove:
    case FileOperation::Action::Trash:
      QtConcurrent::run (this, &FileOperation::erase, sources_, 0);
      break;
  }
}

void FileOperation::advance (qint64 size)
{
  doneSize_ += size;
  auto newProgress = int (doneSize_ / (totalSize_ / 100.));
  if (newProgress != progress_)
  {
    progress_ = newProgress;
    emit progress (progress_, this);
  }
}

void FileOperation::setCurrent (const QString &name)
{
  emit currentChanged (name, this);
}

void FileOperation::finish (bool ok)
{
  emit finished (ok, this);
}

bool FileOperation::copy (const QString &oldName, const QString &newName)
{
  QFile in (oldName);
  const auto size = in.size ();
  QFile out (newName);
  if (!in.open (QFile::ReadOnly) || !out.open (QFile::WriteOnly | QFile::Truncate))
  {
    return false;
  }

  std::array<char, 4096> block;
  qint64 totalRead = 0;
  while (!in.atEnd ())
  {
    auto read = in.read (block.data (), block.size ());
    if (read <= 0)
    {
      break;
    }

    advance (read);

    totalRead += read;
    if (read != out.write (block.data (), read))
    {
      break;
    }
  }
  in.close ();
  out.close ();

  if (totalRead != size)
  {
    QFile::remove (newName);
    return false;
  }

  return QFile::setPermissions (newName, in.permissions ());
}

bool FileOperation::rename (const QString &oldName, const QString &newName)
{
  auto source = StorageManager::storage (oldName);
  auto target = StorageManager::storage (newName);
  if (source == target)
  {
    QFile in (newName);
    const auto size = in.size ();
    auto result = QFile::rename (oldName, newName);
    advance (size);
    return result;
  }


  QFile in (oldName);
  if (in.isSequential ())
  {
    return false;
  }

  const auto size = in.size ();
  QFile out (newName);
  if (!in.open (QFile::ReadOnly) || !out.open (QFile::WriteOnly | QFile::Truncate))
  {
    return false;
  }

  std::array<char, 4096> block;
  qint64 totalRead = 0;
  while (!in.atEnd ())
  {
    auto read = in.read (block.data (), block.size ());
    if (read <= 0)
    {
      break;
    }

    advance (read);

    totalRead += read;
    if (read != out.write (block.data (), read))
    {
      break;
    }
  }
  in.close ();
  out.close ();

  if (totalRead != size)
  {
    QFile::remove (newName);
    return false;
  }

  if (QFile::setPermissions (newName, in.permissions ()) && QFile::remove (oldName))
  {
    return true;
  }

  QFile::remove (newName);
  return false;
}

bool FileOperation::transfer (const FileOperation::Infos &sources, const QFileInfo &target,
                              int depth)
{
  auto ok = true;
  const auto shouldRename = (depth == 0 && !target.exists () && sources.size () == 1);
  QDir targetDir (target.absoluteFilePath ());
  for (const auto &source: sources)
  {
    if (isAborted_)
    {
      ok = false;
      break;
    }
    auto name = source.fileName ();
    QFileInfo targetFile (targetDir.absoluteFilePath (name));
    if (targetFile.absoluteFilePath () == source.absoluteFilePath ())
    {
      if (action_ == FileOperation::Action::Move)
      {
        continue;
      }
      name = uniqueFileName (targetFile);
    }
    else if (targetFile.exists ())
    {
      const auto resolution = resolveConflict (source, targetFile);
      if (isAborted_)
      {
        ok = false;
        break;
      }
      if (resolution & FileConflictResolver::Target)
      {
        continue;
      }
      if (resolution & FileConflictResolver::Source && !removeInfo (targetFile))
      {
        ok = false;
        continue;
      }
      if (resolution & FileConflictResolver::Rename)
      {
        name = uniqueFileName (targetFile);
      }
    }
    else if (shouldRename)
    {
      targetDir.mkpath (target.absolutePath ());
      targetDir.setPath (target.absolutePath ());
      if (!target.fileName ().isEmpty ())
      {
        name = target.fileName ();
      }
    }

    const auto targetFileName = targetDir.absoluteFilePath (name);
    if (source.isDir ())
    {
      const auto removeSource = action_ == FileOperation::Action::Move;
      if (createDir (targetDir, name)
          && transfer (utils::dirEntries (source), targetFileName, depth + 1)
          && ((removeSource && removeInfo (source)) || !removeSource))
      {
        continue;
      }
      ok = false;
      continue;
    }

    setCurrent (source.fileName ());

    switch (action_)
    {
      case FileOperation::Action::Copy:
        if (!copy (source.absoluteFilePath (), targetFileName))
        {
          ok = false;
          Notifier::error (tr ("Failed to copy file %1 to %2")
                           .arg (source.fileName (), targetDir.absolutePath ()));
        }
        break;

      case FileOperation::Action::Move:
        if (!rename (source.absoluteFilePath (), targetFileName))
        {
          ok = false;
          Notifier::error (tr ("Failed to move file %1 to %2")
                           .arg (source.fileName (), targetDir.absolutePath ()));
        }
        break;

      default:
        ASSERT_X (false, "wrong switch");
    }
  }

  if (!depth)
  {
    finish (ok);
  }
  return ok;
}

bool FileOperation::link (const FileOperation::Infos &sources, const QFileInfo &target)
{
  auto ok = true;
  const auto shouldRename = (!target.exists () && sources.size () == 1);
  QDir targetDir (target.absoluteFilePath ());
  for (const auto &source: sources)
  {
    if (isAborted_)
    {
      ok = false;
      break;
    }
    auto name = source.fileName ();
    setCurrent (name);

    if (shouldRename)
    {
      targetDir.mkpath (target.absolutePath ());
      targetDir.setPath (target.absolutePath ());
      if (!target.fileName ().isEmpty ())
      {
        name = target.fileName ();
      }
    }

    const auto targetFileName = targetDir.absoluteFilePath (name);
    if (targetDir.exists (name))
    {
      ok = false;
      Notifier::error (tr ("Target already exists ") + targetFileName);
      continue;
    }

    if (!QFile::link (source.absoluteFilePath (), targetFileName))
    {
      ok = false;
      Notifier::error (tr ("Failed to link file %1 to %2")
                       .arg (source.fileName (), targetDir.absolutePath ()));
    }
    advance (1);
  }
  finish (ok);
  return ok;
}

bool FileOperation::erase (const FileOperation::Infos &infos, int depth)
{
  auto ok = true;
  for (const auto &i: infos)
  {
    if (isAborted_)
    {
      ok = false;
      break;
    }
    const auto size = i.size ();
    setCurrent (i.fileName ());
    switch (action_)
    {
      case FileOperation::Action::Remove:
        if (!i.isDir ())
        {
          if (!QFile::remove (i.absoluteFilePath ()))
          {
            ok = false;
            Notifier::error (tr ("Failed to remove file ") + i.absoluteFilePath ());
          }
        }
        else
        {
          const auto erased = erase (utils::dirEntries (i), depth + 1);
          ok &= (erased ? removeDir (i) : false);
        }
        break;

      case FileOperation::Action::Trash:
        if (!Trash::trash (i))
        {
          ok = false;
          Notifier::error (tr ("Failed to trash ") + i.absoluteFilePath ());
        }
        break;

      default:
        ASSERT_X (false, "wrong switch");
    }
    advance (size);
  }

  if (!depth)
  {
    finish (ok);
  }
  return ok;
}

int FileOperation::resolveConflict (const QFileInfo &source, const QFileInfo &target)
{
  static QMutex mutex;
  QMutexLocker locker (&mutex); // only one request for all threads

  auto resolution = target.isDir () ? allDirResolution_ : allFileResolution_;
  if (resolution == FileConflictResolver::Pending)
  {
    auto connection = (QThread::currentThread () == qApp->thread ())
                      ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    QMetaObject::invokeMethod (resolver_, "resolve", connection,
                               Q_ARG (QFileInfo, source),
                               Q_ARG (QFileInfo, target),
                               Q_ARG (int, int(action_)),
                               Q_ARG (int *, &resolution));
    if (resolution & FileConflictResolver::Abort)
    {
      isAborted_ = true;
    }
    if (resolution & FileConflictResolver::All)
    {
      if (target.isDir ())
      {
        allDirResolution_ = resolution;
      }
      else
      {
        allFileResolution_ = resolution;
      }
    }
  }

  return resolution;
}

void FileOperation::abort ()
{
  isAborted_ = true;
}

#include "moc_fileoperation.cpp"
