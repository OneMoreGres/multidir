#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class GroupView;
class GroupControl;
class FileSystemModel;
class FileOperation;
class FileConflictResolver;

class QSettings;
class QAction;
class QLineEdit;

class MainWindow : public QWidget
{
Q_OBJECT
public:
  explicit MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

protected:
  void keyPressEvent (QKeyEvent *event) override;

private:
  void updateTrayMenu ();
  void trayClicked (QSystemTrayIcon::ActivationReason reason);
  void toggleVisible ();
  void editSettings ();
  void openConsole (const QString &path);
  void openInEditor (const QString &path);
  void setCheckUpdates (bool isOn);
  void addWidget ();
  void activateFindMode ();
  void showAbout ();
  void showFileOperation (QSharedPointer<FileOperation> operation);
  void updateWindowTitle (const QString &groupName);

  FileSystemModel *model_;
  GroupView *groupView_;
  GroupControl *groupControl_;
  FileConflictResolver *conflictResolver_;
  QLineEdit *findEdit_;
  QLayout *fileOperationsLayout_;
  QSystemTrayIcon *tray_;
  QAction *toggleAction_;
  QString consoleCommand_;
  QString editorCommand_;
  bool checkUpdates_;
};
