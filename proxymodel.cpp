#include "proxymodel.h"

#include <QFileSystemModel>

#include <QDebug>

ProxyModel::ProxyModel (QFileSystemModel *model, QObject *parent) :
  QSortFilterProxyModel (parent),
  model_ (model),
  showDirs_ (true),
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

void ProxyModel::setCurrent (const QModelIndex &current)
{
  current_ = mapToSource (current);
  invalidateFilter ();
}

bool ProxyModel::filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const
{
  if (sourceParent == current_ && !showDirs_)
  {
    auto index = model_->index (sourceRow, 0, sourceParent);
    if (model_->isDir (index))
    {
      return false;
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow (sourceRow, sourceParent);
}


QVariant ProxyModel::headerData (int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical && role == Qt::DisplayRole)
  {
    return section + 1;
  }
  return QSortFilterProxyModel::headerData (section, orientation, role);
}
