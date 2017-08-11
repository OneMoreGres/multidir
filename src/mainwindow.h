#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class GroupHolder;
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

public slots:
  void updateSettings ();

protected:
  void keyPressEvent (QKeyEvent *event) override;

private:
  void updateTrayMenu ();
  void trayClicked (QSystemTrayIcon::ActivationReason reason);
  void toggleVisible ();
  void editSettings ();
  void setCheckUpdates (bool isOn);
  void addWidget ();
  void activateFindMode ();
  void showAbout ();
  void showFileOperation (QSharedPointer<FileOperation> operation);
  void updateWindowTitle (const QString &groupName);

  FileSystemModel *model_;
  GroupHolder *groups_;
  GroupControl *groupControl_;
  FileConflictResolver *conflictResolver_;
  QLineEdit *findEdit_;
  QLayout *fileOperationsLayout_;
  QSystemTrayIcon *tray_;
  QAction *toggleAction_;
  bool checkUpdates_;
  bool startInBackground_;
};
