#include "fileoperation.h"
#include "copypaste.h"
#include "trash.h"

#include <QtConcurrent>
#include <QFileInfo>

FileOperation::FileOperation () :
  sources_ (),
  target_ (),
  action_ (),
  future_ ()
{

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

const QFuture<bool> &FileOperation::future () const
{
  return future_;
}

bool FileOperation::isFinished () const
{
  return future_.isFinished ();
}

QSharedPointer<FileOperation> FileOperation::paste (const QList<QUrl> &urls, const QFileInfo &target,
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
  result->future_ = QtConcurrent::run (CopyPaste::paste, urls, target, action);
  return result;
}

QSharedPointer<FileOperation> FileOperation::remove (const QList<QFileInfo> &infos)
{
  auto result = QSharedPointer<FileOperation>::create ();
  result->sources_ = infos;
  result->action_ = Action::Remove;
  result->future_ = QtConcurrent::run (+[](const QList<QFileInfo> &infos) {
    for (const auto &i: infos)
    {
      QFile::remove (i.absoluteFilePath ());
    }
    return true;
  }, infos);
  return result;
}

QSharedPointer<FileOperation> FileOperation::trash (const QList<QFileInfo> &infos)
{
  auto result = QSharedPointer<FileOperation>::create ();
  result->sources_ = infos;
  result->action_ = Action::Trash;
  result->future_ = QtConcurrent::run (+[](const QList<QFileInfo> &infos) {
    for (const auto &i: infos)
    {
      Trash::trash (i);
    }
    return true;
  }, infos);
  return result;
}
