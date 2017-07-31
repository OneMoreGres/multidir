#pragma once

#include <QObject>

class GroupHolder;

class QAction;
class QMenu;
class QActionGroup;
class QSettings;

class GroupControl : public QObject
{
Q_OBJECT
public:
  explicit GroupControl (GroupHolder &view, QObject *parent = nullptr);
  ~GroupControl ();

  QMenu * menu () const;

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  const QString &ids () const;
  void setIds (const QString &ids);

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

  GroupHolder &view_;
  QMenu *menu_;
  QAction *renameAction_;
  QAction *closeAction_;
  QActionGroup *actions_;
  QString ids_;
};
