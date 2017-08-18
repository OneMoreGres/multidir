#pragma once

#include <QWidget>
#include <QModelIndex>

class FileDelegate;

class QTableView;
class QListView;
class QAbstractItemView;
class QAbstractItemModel;
class QSettings;

class DirView : public QWidget
{
Q_OBJECT
public:
  DirView (QAbstractItemModel &model, QWidget *parent = nullptr);

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void activate ();
  void adjustItems ();

  QModelIndex firstItem () const;
  QModelIndex currentIndex () const;
  void setCurrentIndex (const QModelIndex &index);

  QModelIndex rootIndex () const;
  void setRootIndex (const QModelIndex &index);

  QModelIndexList selectedRows () const;

  void renameCurrent ();
  void changeCurrentPermissions ();

  bool isList () const;
  void setIsList (bool isList);

  bool isLocked () const;
  void setLocked (bool isLocked);

  bool isExtensive () const;
  void setExtensive (bool isExtensive);

  bool eventFilter (QObject *watched, QEvent *event) override;

signals:
  void selectionChanged ();
  void currentChanged (const QModelIndex &index);
  void movedBackward ();
  void activated (const QModelIndex &index);
  void backgroundActivated (const QModelIndex &index);
  void contextMenuRequested ();

private:
  void showHeaderContextMenu ();
  QAbstractItemView * view () const;
  void selectFirst ();
  void initTable ();
  void initList ();
  void updateStyle ();
  void setGlowColor (const QColor &color);

  bool isList_;
  bool isLocked_;
  bool isExtensive_;
  FileDelegate *delegate_;
  QAbstractItemModel *model_;
  QTableView *table_;
  QListView *list_;
  QColor glowColor_;
};
