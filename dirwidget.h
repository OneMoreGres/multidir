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

  void setSiblings (const QList<DirWidget *> siblings);

  QString index () const;
  void setIndex (const QString &index);

  void setNameFilter (const QString &filter);

  bool eventFilter (QObject *watched, QEvent *event) override;

  void activate ();
  void adjustItems ();

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);
  void nextTabRequested (DirWidget *widget);
  void newTabRequested (const QFileInfo &path);
  void consoleRequested (const QString &path);
  void editorRequested (const QString &path);
  void fileOperation (QSharedPointer<FileOperation> operation);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  void openPath (const QModelIndex &index);
  void newFolder ();
  QAction * makeShortcut (int shortcutType, QMenu *menu, bool isCheckable = false);

  QString fittedPath (int maxWidth) const;
  QList<QFileInfo> selected () const;
  QFileInfo current () const;
  QFileInfo fileInfo (const QModelIndex &index) const;
  QStringList names (const QList<QModelIndex> &indexes) const;

  void togglePathEdition (bool isOn);
  void handleEditedPath ();
  void startRenaming ();
  void promptClose ();
  void promptTrash ();
  void promptRemove ();
  void cut ();
  void copy ();
  void paste ();
  void copyPath ();
  void openInBackground (const QModelIndex &index);

  bool isMinSizeFixed () const;
  void fixMinSize (bool isOn);

  void showViewContextMenu ();
  void updateSiblingActions ();

  bool isLocked () const;
  void setLocked (bool isLocked);

  void setShowDirs (bool on);

  void updateActions ();
  void updatePathLabel ();
  void checkDirExistence ();
  void handleDirRename (const QString &path, const QString &old, const QString &now);


  FileSystemModel *model_;
  ProxyModel *proxy_;
  DirView *view_;
  QFileInfo path_;
  QLabel *indexLabel_;
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
  QAction *isMinSizeFixed_;

  QMenu *viewMenu_;
  QAction *openAction_;
  QMenu *openWith_;
  QAction *openInEditorAction_;
  QAction *openInTabAction_;
  QAction *renameAction_;
  QAction *trashAction_;
  QAction *removeAction_;
  QAction *cutAction_;
  QAction *copyAction_;
  QAction *pasteAction_;
  QAction *copyPathAction_;
  QMenu *copyToMenu_;
  QMenu *moveToMenu_;
  QMenu *linkToMenu_;

  QAction *upAction_;
  QAction *newFolderAction_;
  QBoxLayout *controlsLayout_;
};

Q_DECLARE_METATYPE (DirWidget *)
