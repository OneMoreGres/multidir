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

  void setIsLocked (bool isLocked);
  void moveUp ();
  void toggleShowDirs (bool show);
  void showViewContextMenu ();
  void showHeaderContextMenu ();
  void openPath (const QModelIndex &index);
  bool isLocked () const;
  QString fittedPath () const;
  void startRenaming ();
  void promptClose ();
  void promptRemove ();
  void togglePathEdition (bool isOn);

  bool isExtensive () const;
  void setExtensive (bool isExtensive);

  ViewMode viewMode () const;
  void setViewMode (ViewMode mode);
  QAbstractItemView * view () const;

  QMenu *menu_;
  FileSystemModel *model_;
  ProxyModel *proxy_;
  QTableView *tableView_;
  QListView *listView_;
  QMenu *viewMenu_;
  QAction *openAction_;
  QAction *renameAction_;
  QAction *removeAction_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QLineEdit *pathEdit_;
  QAction *isLocked_;
  QToolButton *up_;
  QAction *showDirs_;
  QAction *listMode_;
  QAction *extensiveAction_;
  QBoxLayout *controlsLayout_;
};
