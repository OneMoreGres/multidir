#pragma once

#include <QSortFilterProxyModel>

class QFileSystemModel;

class ProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
  ProxyModel (QFileSystemModel *model, QObject *parent = nullptr);
  ~ProxyModel ();

  bool showDirs () const;
  void setShowDirs (bool showDirs);

  bool showHidden () const;
  void setShowHidden (bool showHidden);

  void setNameFilter (const QString &name);

  void setCurrent (const QModelIndex &current);

  QVariant data (const QModelIndex &index, int role) const override;
  QVariant headerData (int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags (const QModelIndex &index) const override;

  bool showThumbnails () const;
  void setShowThumbnails (bool isOn);

signals:
  void iconRequested (const QString &fileName);

protected:
  bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const override;
  bool lessThan (const QModelIndex &left, const QModelIndex &right) const override;

private:
  void updateIcon (const QString &fileName, const QPixmap &pixmap);

  QFileSystemModel *model_;
  bool showDirs_;
  bool showHidden_;
  bool showThumbnails_;
  QString nameFilter_;
  QPersistentModelIndex current_;
  QThread *iconReaderThread_;
};
