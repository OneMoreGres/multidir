#include "fileoperationmodel.h"
#include "fileoperation.h"
#include "debug.h"
#include "constants.h"
#include "fileconflictresolver.h"

FileOperationModel::FileOperationModel (QObject *parent) :
  QAbstractListModel (parent),
  operations_ (),
  conflictResolver_ (new FileConflictResolver)
{

}

void FileOperationModel::paste (const QList<QFileInfo> &infos, const QFileInfo &target,
                                Qt::DropAction action)
{
  auto fileAction = QMap<Qt::DropAction,FileOperation::Action>{
    {Qt::CopyAction, FileOperation::Action::Copy},
    {Qt::MoveAction, FileOperation::Action::Move},
    {Qt::LinkAction, FileOperation::Action::Link}
  }.value (action, FileOperation::Action::Copy);

  add (infos, target, int (fileAction));
}

void FileOperationModel::paste (const QList<QUrl> &urls, const QFileInfo &target,
                                Qt::DropAction action)
{
  QList<QFileInfo> sources;
  for (const auto &i: urls)
  {
    if (!i.toString ().endsWith (constants::dotdot))
    {
      sources << QFileInfo (i.toLocalFile ());
    }
  }

  auto fileAction = QMap<Qt::DropAction,FileOperation::Action>{
    {Qt::CopyAction, FileOperation::Action::Copy},
    {Qt::MoveAction, FileOperation::Action::Move},
    {Qt::LinkAction, FileOperation::Action::Link}
  }.value (action, FileOperation::Action::Copy);

  add (sources, target, int (fileAction));
}

void FileOperationModel::remove (const QList<QFileInfo> &infos)
{
  add (infos, {}, int (FileOperation::Action::Remove));
}

void FileOperationModel::trash (const QList<QFileInfo> &infos)
{
  add (infos, {}, int (FileOperation::Action::Trash));
}

void FileOperationModel::add (const QList<QFileInfo> &sources, const QFileInfo &target,
                              int action)
{
  std::unique_ptr<FileOperation> operation {new FileOperation};
  operation->sources_ = sources;
  operation->target_ = target;
  operation->action_ = FileOperation::Action (action);

  connect (operation.get (), &FileOperation::progress,
           this, &FileOperationModel::setCurrentProgress);
  connect (operation.get (), &FileOperation::currentChanged,
           this, &FileOperationModel::setCurrentFile);
  connect (operation.get (), &FileOperation::finished,
           this, &FileOperationModel::setFinished);

  const auto row = rowCount ({});
  beginInsertRows ({}, row, row);
  operations_.emplace_back (std::move (operation));
  endInsertRows ();

  operations_.back ().operation->startAsync (conflictResolver_.get ());

  if (row == 0)
  {
    emit filled ();
  }
}

void FileOperationModel::remove (FileOperation *operation)
{
  auto row = toIndex (operation).row ();
  beginRemoveRows ({}, row, row);
  operations_.erase (operations_.begin () + row);
  endRemoveRows ();

  if (operations_.empty ())
  {
    emit emptied ();
  }
}

int FileOperationModel::rowCount (const QModelIndex & /*parent*/) const
{
  return int (operations_.size ());
}

QVariant FileOperationModel::data (const QModelIndex &index, int role) const
{
  if (index.isValid () && role == Qt::DisplayRole)
  {
    const auto bundle = toBundle (index);
    return QVariant::fromValue (bundle);
  }
  return {};
}

QModelIndex FileOperationModel::toIndex (FileOperation *operation) const
{
  const auto it = std::find_if (operations_.cbegin (), operations_.cend (),
                                [operation](const Bundle &i) {
                                  return i.operation.get () == operation;
                                });
  ASSERT (it != operations_.cend ());
  auto row = int (std::distance (operations_.cbegin (), it));
  return index (row, 0);
}

FileOperationModel::Bundle * FileOperationModel::toBundle (const QModelIndex &index)
{
  ASSERT (index.row () >= 0);
  ASSERT (index.row () < int (operations_.size ()));
  return &operations_[size_t (index.row ())];
}

const FileOperationModel::Bundle * FileOperationModel::toBundle (const QModelIndex &index) const
{
  ASSERT (index.row () >= 0);
  ASSERT (index.row () < int (operations_.size ()));
  return &operations_[size_t (index.row ())];
}

void FileOperationModel::abort (const QModelIndex &index)
{
  auto bundle = toBundle (index);
  bundle->operation->abort ();
}

void FileOperationModel::setCurrentFile (const QString &current, FileOperation *operation)
{
  const auto index = toIndex (operation);
  ASSERT (index.isValid ());

  auto bundle = toBundle (index);
  bundle->current = current;

  const auto sibling = index.sibling (index.row (), 0);
  emit dataChanged (sibling, sibling, {Qt::DisplayRole});
}

void FileOperationModel::setCurrentProgress (int progress, FileOperation *operation)
{
  const auto index = toIndex (operation);
  ASSERT (index.isValid ());

  auto bundle = toBundle (index);
  bundle->progress = progress;

  const auto sibling = index.sibling (index.row (), 0);
  emit dataChanged (sibling, sibling, {Qt::DisplayRole});
}

void FileOperationModel::setFinished (bool /*ok*/, FileOperation *operation)
{
  remove (operation);
}


FileOperationModel::Bundle::Bundle (std::unique_ptr<FileOperation> &&operation) :
  target (operation->target_.fileName ()),
  current (),
  action (int (operation->action_)),
  progress (0),
  operation (std::move (operation))
{

}

#include "moc_fileoperationmodel.cpp"
