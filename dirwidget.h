#pragma once

#include <QWidget>

class ProxyModel;
class FileSystemModel;
class DirView;

class QLabel;
class QMenu;
class QToolButton;
class QSettings;
class QBoxLayout;
class QLineEdit;
class QFileInfo;

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
  void newTabRequested (const QString &path);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  void openPath (const QModelIndex &index);
  void moveUp ();
  void newFolder ();

  QString fittedPath () const;
  QList<QFileInfo> selected () const;
  QFileInfo current () const;

  void togglePathEdition (bool isOn);
  void startRenaming ();
  void promptClose ();
  void promptTrash ();
  void promptRemove ();
  void cut ();
  void copy ();
  void paste ();

  void showViewContextMenu ();

  bool isLocked () const;
  void setLocked (bool isLocked);

  void setShowDirs (bool on);

  void updateActions ();


  FileSystemModel *model_;
  ProxyModel *proxy_;
  DirView *view_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QLineEdit *pathEdit_;

  QMenu *menu_;
  QAction *isLocked_;
  QAction *showDirs_;
  QAction *extensiveAction_;
  QAction *listMode_;
  QAction *showThumbs_;

  QMenu *viewMenu_;
  QAction *openAction_;
  QAction *openInTabAction_;
  QAction *renameAction_;
  QAction *trashAction_;
  QAction *removeAction_;
  QAction *cutAction_;
  QAction *copyAction_;
  QAction *pasteAction_;

  QToolButton *up_;
  QToolButton *newFolder_;
  QBoxLayout *controlsLayout_;
};
