#pragma once

#include <QWidget>

class ProxyModel;
class FileSystemModel;

class QAbstractItemView;
class QTableView;
class QListView;
class QLabel;
class QMenu;
class QToolButton;
class QSettings;
class QBoxLayout;
class QLineEdit;

class DirWidget : public QWidget
{
Q_OBJECT
public:
  DirWidget (FileSystemModel *model, QWidget *parent = nullptr);
  ~DirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setPath (const QString &path);
  QString path () const;

  void setNameFilter (const QString &filter);

  bool eventFilter (QObject *watched, QEvent *event) override;

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  enum class ViewMode : int
  {
    List, Table
  };

  void openPath (const QModelIndex &index);
  QString fittedPath () const;
  void moveUp ();

  void togglePathEdition (bool isOn);
  void startRenaming ();
  void promptClose ();
  void promptRemove ();

  void showViewContextMenu ();
  void showHeaderContextMenu ();

  bool isLocked () const;
  void setLocked (bool isLocked);

  bool isShowDirs () const;
  void setShowDirs (bool show);

  bool isExtensive () const;
  void setExtensive (bool isExtensive);

  ViewMode viewMode () const;
  void setViewMode (ViewMode mode);
  QAbstractItemView * view () const;

  FileSystemModel *model_;
  ProxyModel *proxy_;
  QTableView *tableView_;
  QListView *listView_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QLineEdit *pathEdit_;

  QMenu *menu_;
  QAction *isLocked_;
  QAction *showDirs_;
  QAction *extensiveAction_;
  QAction *listMode_;

  QMenu *viewMenu_;
  QAction *openAction_;
  QAction *renameAction_;
  QAction *removeAction_;

  QToolButton *up_;
  QBoxLayout *controlsLayout_;
};
