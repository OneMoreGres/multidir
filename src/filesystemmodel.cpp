#include "filesystemmodel.h"
#include "fileoperation.h"
#include "constants.h"
#include "debug.h"

#include <QMimeData>
#include <QUrl>

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

  emit fileOperation (FileOperation::paste (data->urls (), fileInfo (parent), action));
  return true;
}

int FileSystemModel::columnCount (const QModelIndex & /*parent*/) const
{
  return Column::ColumnCount;
}

QVariant FileSystemModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && section >= Column::Permissions)
  {
    if (role == Qt::DisplayRole)
    {
      switch (section)
      {
        case Column::Owner: return tr ("Owner");
        case Column::Group: return tr ("Group");
        case Column::Permissions: return tr ("Permissions");
        case Column::LinkTarget: return tr ("Link target");
      }
    }
    return {};
  }
  return QFileSystemModel::headerData (section, orientation, role);
}

QVariant FileSystemModel::data (const QModelIndex &index, int role) const
{
  if (index.isValid () && index.column () >= Column::Permissions)
  {
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      const auto info = fileInfo (index);
      switch (index.column ())
      {
        case Column::Owner: return info.owner ();
        case Column::Group: return info.group ();
        case Column::Permissions: return int (info.permissions ());
        case Column::LinkTarget: return info.isSymLink () ? info.symLinkTarget () : QString ();
      }
    }
    return {};
  }
  return QFileSystemModel::data (index, role);
}

bool FileSystemModel::setData (const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole)
  {
    return false;
  }

  const auto column = index.column ();
  if (column == Column::Name)
  {
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

  if (column == Column::Permissions)
  {
    auto permissions = QFile::Permissions (value.toInt ());
    QFile file (fileInfo (index).absoluteFilePath ());
    auto ok = file.setPermissions (permissions);
    LWARNING_IF (!ok) << "Failed to set permissions for" << LARG (file.fileName ())
                      << "to" << LARG (permissions);
    return ok;
  }

  return false;
}

Qt::ItemFlags FileSystemModel::flags (const QModelIndex &index) const
{
  const auto nameIndex = index.sibling (index.row (), Column::Name);
  Qt::ItemFlags result = QFileSystemModel::flags (nameIndex);
  const auto isDotDot = (nameIndex.data ().toString () == constants::dotdot);
  if (!(index.column () == Column::Name || index.column () == Column::Permissions) ||
      isDotDot)
  {
    result &= ~Qt::ItemIsEditable;
  }
  return result;
}
