#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class GroupsView;
class FileSystemModel;
class ShellCommandModel;
class FileOperationModel;

class QSettings;
class QAction;
class QLineEdit;
class QListView;

class MainWindow : public QWidget
{
Q_OBJECT
public:
  explicit MainWindow (QWidget *parent = nullptr);
  ~MainWindow ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

signals:
  void nameFilterChanged (const QString &filter);

public slots:
  void updateSettings ();

protected:
  void keyPressEvent (QKeyEvent *event) override;
  void closeEvent (QCloseEvent *event) override;

private:
  void toggleVisible ();
  void editSettings ();
  void showAbout ();
  void activateFindMode ();

  void updateTrayMenu ();
  void handleTrayClick (QSystemTrayIcon::ActivationReason reason);
  void setCheckUpdates (bool isOn);
  void updateWindowTitle (const QString &groupName);
  void showFileOperationsMenu ();

  FileOperationModel *fileOperationModel_;
  QListView *fileOperationView_;
  FileSystemModel *model_;
  GroupsView *groups_;
  QLineEdit *findEdit_;
  QSystemTrayIcon *tray_;
  QAction *toggleAction_;
  ShellCommandModel *commandsModel_;
  QListView *commandsView_;
  bool checkUpdates_;
  bool startInBackground_;
};
