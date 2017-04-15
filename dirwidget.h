#pragma once

#include <QWidget>

class ProxyModel;
class FileSystemModel;

class QTableView;
class QLabel;
class QMenu;
class QToolButton;
class QSettings;
class QBoxLayout;

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

  void setIsExtensiveView (bool isExtensive);

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  void setIsLocked (bool isLocked);
  void moveUp ();
  void toggleShowDirs (bool show);
  void showContextMenu ();
  void showHeaderContextMenu ();
  void openPath (const QModelIndex &index);
  QString path (const QModelIndex &index) const;
  bool isLocked () const;
  QString fittedPath () const;
  void updateMenu ();
  void startRenaming ();
  void promptClose ();
  void promptRemove ();

  FileSystemModel *model_;
  ProxyModel *proxy_;
  QTableView *view_;
  QMenu *menu_;
  QAction *openAction_;
  QAction *renameAction_;
  QAction *removeAction_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QAction *isLocked_;
  QToolButton *up_;
  QToolButton *showDirs_;
  QBoxLayout *controlsLayout_;
};
