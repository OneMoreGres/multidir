#pragma once

#include <QObject>

class GroupView;

class QAction;
class QMenu;
class QActionGroup;
class QSettings;

class GroupControl : public QObject
{
Q_OBJECT
public:
  explicit GroupControl (GroupView &view, QObject *parent = nullptr);
  ~GroupControl ();

  QMenu * menu () const;

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

private:
  void add ();
  void renameCurrent ();
  void removeCurrent ();

  void updateMenuState ();
  void populateMenuActions ();
  void trigger (QAction *action);
  void setCurrent (QAction *action);
  void updateShortcuts ();
  QAction * actionAt (int index) const;
  int index (QAction *action) const;

  GroupView &view_;
  QMenu *menu_;
  QAction *renameAction_;
  QAction *closeAction_;
  QActionGroup *actions_;
};
