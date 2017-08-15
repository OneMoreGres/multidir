#pragma once

#include <QSortFilterProxyModel>
#include <QFileInfo>

class FileSystemModel;

class ProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
  ProxyModel (FileSystemModel *model, QObject *parent = nullptr);
  ~ProxyModel ();

  bool showDirs () const;
  void setShowDirs (bool showDirs);

  bool showFiles () const;
  void setShowFiles (bool showFiles);

  bool showDotDot () const;
  void setShowDotDot (bool showDotDot);

  bool showHidden () const;
  void setShowHidden (bool showHidden);

  void setNameFilter (const QString &name);

  void setCurrent (const QModelIndex &current);
  QModelIndex current () const;

  QVariant data (const QModelIndex &index, int role) const override;
  QVariant headerData (int section, Qt::Orientation orientation,
                       int role = Qt::DisplayRole) const override;

  bool showThumbnails () const;
  void setShowThumbnails (bool isOn);


  bool isDir (int row) const;
  qint64 fileSize (int row) const;
  QString fileName (int row) const;
  int count () const;
  bool isDotDot (const QModelIndex &index) const;

  QFileInfo currentPath () const;

signals:
  void contentsChanged ();
  void currentChanged (const QModelIndex &index);
  void iconRequested (const QString &fileName);

protected:
  bool filterAcceptsRow (int sourceRow, const QModelIndex &sourceParent) const override;
  bool lessThan (const QModelIndex &left, const QModelIndex &right) const override;

private:
  void detectContentsChange (const QModelIndex &parent);
  void updateIcon (const QString &fileName, const QPixmap &pixmap);

  FileSystemModel *model_;
  bool showDirs_;
  bool showFiles_;
  bool showDotDot_;
  bool showHidden_;
  bool showThumbnails_;
  QString nameFilter_;
  QPersistentModelIndex current_;
  QThread *iconReaderThread_;
};
