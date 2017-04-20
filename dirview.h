#pragma once

#include <QWidget>
#include <QModelIndex>

class QTableView;
class QListView;
class QAbstractItemView;
class QAbstractItemModel;
class QSettings;

class DirView : public QWidget
{
Q_OBJECT
public:
  enum class Mode
  {
    List, Table
  };

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  DirView (QAbstractItemModel &model, QWidget *parent = nullptr);

  QModelIndex currentIndex () const;
  void setCurrentIndex (const QModelIndex &index);
  QModelIndex rootIndex () const;
  void setRootIndex (const QModelIndex &index);

  QModelIndexList selectedRows () const;

  void edit (const QModelIndex &index);

  Mode mode () const;
  void setMode (Mode mode);

  bool isLocked () const;
  void setLocked (bool isLocked);

  bool isExtensive () const;
  void setExtensive (bool isExtensive);

signals:
  void doubleClicked (const QModelIndex &index);
  void contextMenuRequested ();

private:
  void showHeaderContextMenu ();
  QAbstractItemView * view () const;
  void initTable ();
  void initList ();

  Mode mode_;
  bool isLocked_;
  bool isExtensive_;
  QAbstractItemModel *model_;
  QTableView *table_;
  QListView *list_;
};
