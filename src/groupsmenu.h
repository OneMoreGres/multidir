#pragma once

#include <QMenu>

class GroupsView;

class QAction;
class QMenu;
class QActionGroup;
class QSettings;

class GroupsMenu : public QMenu
{
Q_OBJECT
public:
  explicit GroupsMenu (GroupsView *view, QWidget *parent = nullptr);
  ~GroupsMenu ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  const QString &ids () const;
  void setIds (const QString &ids);

public slots:
  void updateSettings ();

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

  GroupsView *view_;
  QAction *renameAction_;
  QAction *closeAction_;
  QActionGroup *actions_;
  QString ids_;
};
