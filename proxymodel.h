#pragma once

#include <QSortFilterProxyModel>

class QFileSystemModel;

class ProxyModel : public QSortFilterProxyModel
{
public:
  ProxyModel (QFileSystemModel *model, QObject *parent = nullptr);

  bool showDirs () const;
  void setShowDirs (bool showDirs);

  void setNameFilter (const QString &name);

  void setCurrent (const QModelIndex &current);

  QVariant data (const QModelIndex &index, int role) const override;
  QVariant headerData (int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags (const QModelIndex &index) const override;

protected:
  bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const override;
  bool lessThan (const QModelIndex &left, const QModelIndex &right) const override;

private:
  QFileSystemModel *model_;
  bool showDirs_;
  QString nameFilter_;
  QPersistentModelIndex current_;
};
