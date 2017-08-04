#include "filesystemcompleter.h"
#include "filesystemmodel.h"
#include "proxymodel.h"
#include "debug.h"

#include <QAbstractItemView>
#include <QApplication>

FileSystemCompleter::FileSystemCompleter (FileSystemModel *model, QObject *parent) :
  QCompleter (parent),
  proxy_ (new ProxyModel (model, this))
{
  proxy_->setShowFiles (false);
  proxy_->setShowDotDot (false);

  setCompletionRole (FileSystemModel::FileNameRole);
#ifdef Q_OS_WIN
  setCaseSensitivity (Qt::CaseInsensitive);
#endif
  setModel (proxy_);

  connect (model, &QFileSystemModel::directoryLoaded, this, [this](const QString &path) {
             if (lastPath_ == path && completionCount () > 0)
             {
               QApplication::instance ()->processEvents (QEventLoop::ExcludeUserInputEvents, 100);
               complete ();
             }
           });
}

QString FileSystemCompleter::pathFromIndex (const QModelIndex &index) const
{
  if (!index.isValid ())
  {
    return {};
  }

  const auto source = static_cast<FileSystemModel *>(proxy_->sourceModel ());
  ASSERT (source);
  const auto mapped = proxy_->mapToSource (index);
  const auto path = source->filePath (mapped);
  return QDir::toNativeSeparators (path);
}

QStringList FileSystemCompleter::splitPath (const QString &path) const
{
  auto native = QDir::toNativeSeparators (path);
#ifdef Q_OS_WIN
  if (native == QLatin1String ("\\"))
  {
    return {};
  }
  if (native == QLatin1String ("\\\\"))
  {
    return QStringList (native);
  }
  const auto startsWithDoubleSlash = native.startsWith (QLatin1String ("\\\\"));
  if (startsWithDoubleSlash)
  {
    native = native.mid (2);
  }
#endif
  auto parts = native.split (QDir::separator ());
#ifdef Q_OS_WIN
  if (startsWithDoubleSlash)
  {
    parts[0].prepend (QLatin1String ("\\\\"));
  }
#else
  if (native[0] == QDir::separator ()) // readd the "/" at the beginning as the split removed it
  {
    parts[0] = QLatin1Char ('/');
  }
#endif

#ifdef Q_OS_WIN
  if (startsWithDoubleSlash) // skip network address checks
  {
    return parts;
  }
#endif

  if (!QFileInfo::exists (path))
  {
    return parts;
  }

  const auto source = static_cast<FileSystemModel *>(proxy_->sourceModel ());
  ASSERT (source);
  auto index = source->index (path);
  if (index.isValid () && !native.endsWith (QDir::separator ()))
  {
    index = index.parent ();
  }
  if (index.isValid ())
  {
    lastPath_ = source->filePath (index);
    proxy_->setCurrent (proxy_->mapFromSource (index));
  }

  return parts;
}
