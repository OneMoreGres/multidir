#include "proxymodel.h"
#include "filesystemmodel.h"
#include "constants.h"

#include <QDateTime>
#include <QPixmapCache>
#include <QImageReader>

#include <QDebug>


ProxyModel::ProxyModel (QFileSystemModel *model, QObject *parent) :
  QSortFilterProxyModel (parent),
  model_ (model),
  showDirs_ (true),
  showThumbnails_ (false),
  nameFilter_ (),
  current_ ()
{
  setSourceModel (model);
}

bool ProxyModel::showDirs () const
{
  return showDirs_;
}

void ProxyModel::setShowDirs (bool showDirs)
{
  showDirs_ = showDirs;
  invalidateFilter ();
}

bool ProxyModel::showThumbnails () const
{
  return showThumbnails_;
}

void ProxyModel::setShowThumbnails (bool isOn)
{
  showThumbnails_ = isOn;
  emit dataChanged (index (0, FileSystemModel::Name), index (rowCount () - 1, FileSystemModel::Size),
                    {Qt::DecorationRole});
}

void ProxyModel::setNameFilter (const QString &name)
{
  nameFilter_ = name;
  invalidateFilter ();
}

void ProxyModel::setCurrent (const QModelIndex &current)
{
  current_ = mapToSource (current);
  invalidateFilter ();
}

bool ProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
  if (sourceParent == current_)
  {
    if (!showDirs_ || !nameFilter_.isEmpty ())
    {
      auto index = model_->index (sourceRow, 0, sourceParent);
      if (!showDirs_ && model_->isDir (index))
      {
        return false;
      }
      if (!nameFilter_.isEmpty () && !QDir::match (nameFilter_, index.data ().toString ()))
      {
        return false;
      }
    }
  }
  return true;
}


QVariant ProxyModel::data (const QModelIndex &index, int role) const
{
  if (role == Qt::BackgroundRole)
  {
    const auto info = model_->fileInfo (mapToSource (index));
    if (info.isDir ())
    {
      return QColor (204,255,255);
    }
    if (info.isExecutable ())
    {
      return QColor (255,204,153);
    }
    return {};
  }

  if (showThumbnails_ && role == Qt::DecorationRole && index.column () == FileSystemModel::Name)
  {
    const auto info = model_->fileInfo (mapToSource (index));
    if (QImageReader::supportedImageFormats ().contains (info.suffix ().toUtf8 ()))
    {
      const auto path = info.absoluteFilePath ();
      if (auto cached = QPixmapCache::find (path))
      {
        return QIcon (*cached);
      }
      QImageReader reader (path);
      reader.setScaledSize ({constants::iconSize, constants::iconSize});
      const auto image = reader.read ();
      if (!image.isNull ())
      {
        const auto pixmap = QPixmap::fromImage (image);
        QPixmapCache::insert (path, pixmap);
        return QIcon (pixmap);
      }
    }
    return QSortFilterProxyModel::data (index, role);
  }

  if (role == Qt::TextAlignmentRole && index.column () == FileSystemModel::Size)
  {
    return int (Qt::AlignVCenter) | Qt::AlignRight;
  }

  return QSortFilterProxyModel::data (index, role);
}


QVariant ProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return section + 1;
  }
  return QSortFilterProxyModel::headerData (section, orientation, role);
}


Qt::ItemFlags ProxyModel::flags (const QModelIndex &index) const
{
  auto flags = QSortFilterProxyModel::flags (index);
  if (index.column () != 0 || index.data ().toString () == constants::dotdot)
  {
    flags.setFlag (Qt::ItemIsEditable, false);
  }
  return flags;
}

bool ProxyModel::lessThan (const QModelIndex &left, const QModelIndex &right) const
{
  // keep .. on top
  const auto l = model_->fileInfo (left);
  if (l.fileName () == constants::dotdot)
  {
    return sortOrder () == Qt::AscendingOrder;
  }
  const auto r = model_->fileInfo (right);
  if (r.fileName () == constants::dotdot)
  {
    return sortOrder () != Qt::AscendingOrder;
  }

  // keep folders on top
  if (l.isDir () != r.isDir ())
  {
    return (sortOrder () == Qt::AscendingOrder ? l.isDir () : !l.isDir ());
  }


  switch (sortColumn ())
  {
    case FileSystemModel::Column::Name:
      return QString::localeAwareCompare (l.fileName (),r.fileName ()) < 0;

    case FileSystemModel::Column::Size:
      return model_->size (left) < model_->size (right);

    case FileSystemModel::Column::Type:
      return model_->type (left) < model_->type (right);

    case FileSystemModel::Column::Date:
      return model_->lastModified (left) < model_->lastModified (right);
  }
  return QSortFilterProxyModel::lessThan (left, right);
}
