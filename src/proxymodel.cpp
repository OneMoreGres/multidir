#include "proxymodel.h"
#include "filesystemmodel.h"
#include "constants.h"
#include "backgroundreader.h"
#include "styleoptionsproxy.h"
#include "debug.h"

#include <QDateTime>
#include <QPixmapCache>
#include <QImageReader>
#include <QThread>
#include <QRegExp>


ProxyModel::ProxyModel (FileSystemModel *model, QObject *parent) :
  QSortFilterProxyModel (parent),
  model_ (model),
  showDirs_ (true),
  showFiles_ (true),
  showDotDot_ (true),
  showHidden_ (false),
  showThumbnails_ (false),
  nameFilter_ (),
  current_ (),
  iconReaderThread_ (new QThread (this)),
  dirColor_ (),
  inaccessibleDirColor_ (),
  executableColor_ (),
  unreadableFileColor_ ()
{
  setSourceModel (model);

  connect (model, &FileSystemModel::rowsInserted,
           this, &ProxyModel::detectContentsChange);
  connect (model, &FileSystemModel::rowsRemoved,
           this, &ProxyModel::detectContentsChange);

  auto *reader = new BackgroundReader;
  connect (this, &ProxyModel::iconRequested,
           reader, &BackgroundReader::readIcon);
  connect (reader, &BackgroundReader::iconRead,
           this, &ProxyModel::updateIcon);

  reader->moveToThread (iconReaderThread_);
  connect (iconReaderThread_, &QThread::finished,
           reader, &QObject::deleteLater);
  iconReaderThread_->start ();

  connect (&StyleOptionsProxy::instance (), &StyleOptionsProxy::changed,
           this, &ProxyModel::updateStyle);
  updateStyle ();
}

ProxyModel::~ProxyModel ()
{
  iconReaderThread_->quit ();
  iconReaderThread_->wait (2000);
}

bool ProxyModel::showDirs () const
{
  return showDirs_;
}

void ProxyModel::setShowDirs (bool showDirs)
{
  if (showDirs_ == showDirs)
  {
    return;
  }
  showDirs_ = showDirs;
  invalidateFilter ();
  emit contentsChanged ();
}

bool ProxyModel::showFiles () const
{
  return showFiles_;
}

void ProxyModel::setShowFiles (bool showFiles)
{
  if (showFiles_ == showFiles)
  {
    return;
  }
  showFiles_ = showFiles;
  invalidateFilter ();
  emit contentsChanged ();
}

bool ProxyModel::showDotDot () const
{
  return showDotDot_;
}

void ProxyModel::setShowDotDot (bool showDotDot)
{
  if (showDotDot_ == showDotDot)
  {
    return;
  }
  showDotDot_ = showDotDot;
  invalidateFilter ();
  emit contentsChanged ();
}

bool ProxyModel::showHidden () const
{
  return showHidden_;
}

void ProxyModel::setShowHidden (bool showHidden)
{
  if (showHidden_ == showHidden)
  {
    return;
  }
  showHidden_ = showHidden;
  invalidateFilter ();
  emit contentsChanged ();
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

bool ProxyModel::isDir (int row) const
{
  return model_->isDir (mapToSource (index (row, 0, current ())));
}

qint64 ProxyModel::fileSize (int row) const
{
  return model_->size (mapToSource (index (row, 0, current ())));
}

QString ProxyModel::fileName (int row) const
{
  return model_->fileName (mapToSource (index (row, 0, current ())));
}

int ProxyModel::count () const
{
  return rowCount (current ());
}

bool ProxyModel::isDotDot (const QModelIndex &index) const
{
  return model_->fileName (mapToSource (index.sibling (index.row (), 0))) == constants::dotdot;
}

QFileInfo ProxyModel::currentPath () const
{
  return model_->fileInfo (current_);
}

void ProxyModel::setNameFilter (const QString &name)
{
  if (nameFilter_ == name)
  {
    return;
  }
  nameFilter_ = name;
  invalidateFilter ();
  emit contentsChanged ();
}

void ProxyModel::setCurrent (const QModelIndex &current)
{
  const auto mapped = mapToSource (current);
  if (current_ == mapped)
  {
    return;
  }
  current_ = mapToSource (current);
  invalidateFilter ();
  emit currentChanged (current_);
  emit contentsChanged ();
}

QModelIndex ProxyModel::current () const
{
  return mapFromSource (current_);
}

bool ProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
#ifdef Q_OS_WIN
  if (!sourceParent.isValid ())
  {
    const auto name = model_->fileName (model_->index (sourceRow, 0, sourceParent));
    if (name.startsWith (constants::networkDirStart))
    {
      return false;
    }
  }
#endif
  if (sourceParent == current_)
  {
    const auto canFilter = !showDirs_ || !showFiles_ || !showDotDot_ ||
                           !showHidden_ || !nameFilter_.isEmpty ();
    if (canFilter)
    {
      const auto info = model_->fileInfo (model_->index (sourceRow, 0, sourceParent));
      if (!showDirs_ && info.isDir ())
      {
        return false;
      }
      if (!showFiles_ && info.isFile ())
      {
        return false;
      }
      if (!showDotDot_ && info.fileName () == constants::dotdot)
      {
        return false;
      }
      if (!showHidden_ && info.isHidden () && info.fileName () != constants::dotdot)
      {
        return false;
      }
      if (!nameFilter_.isEmpty () )
      {
        QRegExp re (nameFilter_);
        re.setPatternSyntax (QRegExp::Wildcard);
        re.setCaseSensitivity (Qt::CaseInsensitive);
        if (!re.exactMatch (info.fileName ()))
        {
          return false;
        }
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
      if (!info.isExecutable () || !info.isReadable ())
      {
        return inaccessibleDirColor_;
      }
      return dirColor_;
    }
    if (info.isExecutable ())
    {
      return executableColor_;
    }
    else if (!info.isReadable ())
    {
      return unreadableFileColor_;
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
      emit const_cast<ProxyModel *>(this)->iconRequested (path);
    }
    return QSortFilterProxyModel::data (index, role);
  }

  if (role == Qt::TextAlignmentRole && index.column () == FileSystemModel::Size)
  {
    return int (Qt::AlignVCenter) | Qt::AlignRight;
  }

  return QSortFilterProxyModel::data (index, role);
}


void ProxyModel::updateIcon (const QString &fileName, const QPixmap &pixmap)
{
  auto index = mapFromSource (model_->index (fileName));
  if (index.isValid ())
  {
    QPixmapCache::insert (fileName, pixmap);
    emit dataChanged (index, index, {Qt::DecorationRole});
  }
}

void ProxyModel::updateStyle ()
{
  const auto &options = StyleOptionsProxy::instance ();
  dirColor_ =  options.dirColor ();
  inaccessibleDirColor_ =  options.inaccessibleDirColor ();
  executableColor_ =  options.executableColor ();
  unreadableFileColor_ =  options.unreadableFileColor ();

  if (current_.isValid ())
  {
    const auto parent = mapFromSource (current_);
    const auto rows = rowCount (parent);
    if (rows > 0)
    {
      emit dataChanged (index (0,0, parent), index (rows - 1, 0, parent), {Qt::BackgroundRole});
    }
  }
}

QVariant ProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return section + 1;
  }
  return QSortFilterProxyModel::headerData (section, orientation, role);
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

void ProxyModel::detectContentsChange (const QModelIndex &parent)
{
  if (parent == current_)
  {
    emit contentsChanged ();
  }
}

#include "moc_proxymodel.cpp"
