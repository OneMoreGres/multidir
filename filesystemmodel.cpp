#include "filesystemmodel.h"
#include "copypaste.h"

#include <QMimeData>
#include <QUrl>

#include <QDebug>

FileSystemModel::FileSystemModel (QObject *parent) :
  QFileSystemModel (parent)
{

}


bool FileSystemModel::dropMimeData (const QMimeData *data, Qt::DropAction action, int row,
                                    int column, const QModelIndex &parent)
{
  Q_UNUSED (row);
  Q_UNUSED (column);
  if (!parent.isValid () || isReadOnly ())
  {
    return false;
  }

  return CopyPaste::paste (data->urls (), fileInfo (parent), action);
}

bool FileSystemModel::setData (const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole || index.column () != Column::Name)
  {
    return false;
  }
  const auto name = value.toString ();
  auto dir = fileInfo (index).absoluteDir ();
  if (name.isEmpty () || !dir.exists ())
  {
    return false;
  }

  const auto old = index.data ().toString ();
  auto ok = dir.rename (old, name);
  if (ok)
  {
    emit fileRenamed (dir.absolutePath (), old, name);
  }
  return ok;
}
