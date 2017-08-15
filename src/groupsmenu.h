#pragma once

#include <QMenu>

class GroupsView;

class GroupsMenu : public QMenu
{
Q_OBJECT
public:
  explicit GroupsMenu (GroupsView *view, QWidget *parent = nullptr);
  ~GroupsMenu ();

public slots:
  void updateSettings ();

private:
  void add ();
  void renameCurrent ();
  void removeCurrent ();

  void handleViewRestored ();
  void updateMenuState ();
  void populateMenuActions ();
  void trigger (QAction *action);
  void setCurrent (QAction *action);
  void updateShortcuts ();
  QAction * actionAt (int index) const;
  void setIds (const QString &ids);

  GroupsView *view_;
  QAction *renameAction_;
  QAction *closeAction_;
  QActionGroup *actions_;
  QString ids_;
};
