#pragma once

#include <QWidget>

class ProxyModel;

class QTableView;
class QFileSystemModel;
class QLabel;
class QMenu;
class QToolButton;
class QSettings;

class DirWidget : public QWidget
{
Q_OBJECT
public:
  DirWidget (QFileSystemModel *model, QWidget *parent = nullptr);
  ~DirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setPath (const QString &path);
  QString path () const;

  void setNameFilter (const QString &filter);

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);

private:
  void setIsLocked (bool isLocked);
  void moveUp ();
  void toggleShowDirs (bool show);
  void showContextMenu ();
  void openPath (const QModelIndex &index);
  QString path (const QModelIndex &index) const;
  bool isLocked () const;

  QFileSystemModel *model_;
  ProxyModel *proxy_;
  QTableView *view_;
  QMenu *menu_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QAction *isLocked_;
  QToolButton *up_;
  QToolButton *showDirs_;
};
