#pragma once

#include <QSystemTrayIcon>

class MultiDirWidget;

class QSettings;
class QAction;

class Controller : public QObject
{
Q_OBJECT
public:
  explicit Controller (QObject *parent = 0);
  ~Controller ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

private:
  void updateMenu ();
  void trayClicked (QSystemTrayIcon::ActivationReason reason);
  void toggleWidget ();
  void editSettings ();

  QSystemTrayIcon *tray_;
  QScopedPointer<MultiDirWidget> widget_;
  QAction *toggleAction_;
};
