#pragma once

#include <QWidget>
#include <QFileInfo>

class ProxyModel;
class FileSystemModel;
class DirView;
class FileOperation;

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

  void setPath (const QFileInfo &path);
  QFileInfo path () const;

  void setNameFilter (const QString &filter);

  bool eventFilter (QObject *watched, QEvent *event) override;

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);
  void newTabRequested (const QFileInfo &path);
  void consoleRequested (const QString &path);
  void fileOperation (QSharedPointer<FileOperation> operation);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  void openPath (const QModelIndex &index);
  void newFolder ();

  QString fittedPath () const;
  QList<QFileInfo> selected () const;
  QFileInfo current () const;
  QFileInfo fileInfo (const QModelIndex &index) const;
  QStringList names (const QList<QModelIndex> &indexes) const;

  void togglePathEdition (bool isOn);
  void startRenaming ();
  void promptClose ();
  void promptTrash ();
  void promptRemove ();
  void cut ();
  void copy ();
  void paste ();
  void copyPath ();

  void showViewContextMenu ();

  bool isLocked () const;
  void setLocked (bool isLocked);

  void setShowDirs (bool on);

  void updateActions ();
  void checkDirExistence ();
  void handleDirRename (const QString &path, const QString &old, const QString &now);


  FileSystemModel *model_;
  ProxyModel *proxy_;
  DirView *view_;
  QFileInfo path_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QLineEdit *pathEdit_;

  QMenu *menu_;
  QAction *isLocked_;
  QAction *showDirs_;
  QAction *showHidden_;
  QAction *extensiveAction_;
  QAction *listMode_;
  QAction *showThumbs_;

  QMenu *viewMenu_;
  QAction *openAction_;
  QMenu *openWith_;
  QAction *openInTabAction_;
  QAction *renameAction_;
  QAction *trashAction_;
  QAction *removeAction_;
  QAction *cutAction_;
  QAction *copyAction_;
  QAction *pasteAction_;
  QAction *copyPathAction_;

  QToolButton *up_;
  QToolButton *newFolder_;
  QBoxLayout *controlsLayout_;
};
