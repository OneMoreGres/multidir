#pragma once

#include <QWidget>
#include <QFileInfo>

class ProxyModel;
class FileSystemModel;
class DirView;
class FileOperation;
class PathWidget;
class DirStatusWidget;
class NavigationHistory;
class ShellCommandModel;

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
  DirWidget (FileSystemModel *model, ShellCommandModel *commands, QWidget *parent = nullptr);
  ~DirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setPath (const QFileInfo &path);
  QFileInfo path () const;

  void setSiblings (const QList<DirWidget *> siblings);

  QString index () const;
  void setIndex (const QString &index);
  QString fullName (int preferredWidth) const;

  QList<QFileInfo> selected () const;
  QFileInfo current () const;

  void setNameFilter (const QString &filter);

  bool eventFilter (QObject *watched, QEvent *event) override;

  void activate ();
  void adjustItems ();

signals:
  void closeRequested (DirWidget *widget);
  void cloneRequested (DirWidget *widget);
  void nextTabRequested (DirWidget *widget);
  void previousTabRequested (DirWidget *widget);
  void newTabRequested (const QFileInfo &path);
  void fileOperation (QSharedPointer<FileOperation> operation);

protected:
  void resizeEvent (QResizeEvent *event) override;

private:
  void newFolder ();

  QFileInfo fileInfo (const QModelIndex &index) const;
  QStringList names (const QList<QModelIndex> &indexes) const;

  void openSelected ();
  void openPath (const QModelIndex &index);
  void openFile (const QFileInfo &info);

  void promptClose ();
  void promptTrash ();
  void promptRemove ();
  void cut ();
  void copy ();
  void paste ();
  void copyPath ();
  void openInBackground (const QModelIndex &index);
  void showProperties ();
  void viewCurrent ();
  void moveUp ();

  void openConsole ();
  void openInEditor ();

  bool isMinSizeFixed () const;
  void fixateSizeAsMinimal (bool isOn);

  void showViewContextMenu ();
  void createSiblingActions ();
  void updateSiblingActions ();

  void showCommandPrompt ();
  void execCommandPrompt ();

  bool isLocked () const;
  void setLocked (bool isLocked);

  void setShowDirs (bool on);

  void updateStatusSelection ();
  void updateActions ();
  void checkDirExistence ();
  void updateCurrentFile ();
  void handleDirRename (const QString &path, const QString &old, const QString &now);
  void handleFileRename (const QString &path, const QString &old, const QString &now);


  FileSystemModel *model_;
  ProxyModel *proxy_;
  DirView *view_;
  QString index_;
  QFileInfo path_;
  QString lastCurrentFile_;
  PathWidget *pathWidget_;
  DirStatusWidget *status_;
  QLineEdit *commandPrompt_;
  ShellCommandModel *commandRunner_;
  QList<DirWidget *> siblings_;
  NavigationHistory *navigationHistory_;

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
  QAction *viewAction_;
  QAction *openInEditorAction_;
  QAction *openInTabAction_;
  QAction *renameAction_;
  QAction *permissionsAction_;
  QAction *trashAction_;
  QAction *removeAction_;
  QAction *showPropertiesAction_;
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
