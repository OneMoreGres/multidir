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

  return CopyPaste ().paste (data->urls (), fileInfo (parent), action);
}
