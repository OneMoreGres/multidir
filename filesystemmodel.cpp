#include "filesystemmodel.h"

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

  auto success = true;
  auto to = filePath (parent) + QDir::separator ();

  auto urls = data->urls ();
  auto it = urls.constBegin ();

  switch (action)
  {
    case Qt::CopyAction:
      for (; it != urls.constEnd (); ++it)
      {
        auto path = (*it).toLocalFile ();
        auto target = QFileInfo (path).fileName ();
        while (QFile::exists (to + target))
        {
          target = tr ("copy_") + target;
        }
        success = QFile::copy (path, to + target) && success;
      }
      break;

    case Qt::LinkAction:
      for (; it != urls.constEnd (); ++it)
      {
        auto path = (*it).toLocalFile ();
        success = QFile::link (path, to + QFileInfo (path).fileName ()) && success;
      }
      break;

    case Qt::MoveAction:
      for (; it != urls.constEnd (); ++it)
      {
        auto path = (*it).toLocalFile ();
        success = QFile::rename (path, to + QFileInfo (path).fileName ()) && success;
      }
      break;

    default:
      return false;
  }

  return success;
}
