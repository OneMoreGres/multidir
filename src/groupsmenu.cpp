#include "groupsmenu.h"
#include "groupsview.h"
#include "groupwidget.h"
#include "debug.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "settingsmanager.h"

#include <QMenu>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>

namespace
{
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
}

GroupsMenu::GroupsMenu (GroupsView *view, QObject *parent) :
  QObject (parent),
  view_ (view),
  menu_ (new QMenu (tr ("Groups"))),
  actions_ (new QActionGroup (this)),
  ids_ ()
{
  connect (actions_, &QActionGroup::triggered,
           this, &GroupsMenu::setCurrent);

  auto addGroup = new QAction (this);
  menu_->addAction (addGroup);
  ShortcutManager::add (ShortcutManager::AddGroup, addGroup);
  connect (addGroup, &QAction::triggered, this, &GroupsMenu::add);

  renameAction_ = new QAction (this);
  menu_->addAction (renameAction_);
  ShortcutManager::add (ShortcutManager::RenameGroup, renameAction_);
  connect (renameAction_, &QAction::triggered, this, &GroupsMenu::renameCurrent);

  closeAction_ = new QAction (this);
  menu_->addAction (closeAction_);
  ShortcutManager::add (ShortcutManager::RemoveGroup, closeAction_);
  connect (closeAction_, &QAction::triggered, this, &GroupsMenu::removeCurrent);

  menu_->addSeparator ();

  SettingsManager::subscribeForUpdates (this);
  updateSettings ();
}

GroupsMenu::~GroupsMenu ()
{
  menu_->deleteLater ();
}

QMenu * GroupsMenu::menu () const
{
  return menu_;
}

GroupsView * GroupsMenu::view () const
{
  return view_;
}

void GroupsMenu::save (QSettings &settings) const
{
  view_->save (settings);
}

void GroupsMenu::restore (QSettings &settings)
{
  view_->restore (settings);

  populateMenuActions ();
  updateMenuState ();
  updateShortcuts ();

  const auto current = view_->currentIndex ();
  trigger (actionAt (current));
}

void GroupsMenu::updateSettings ()
{
  SettingsManager settings;
  setIds (settings.get (SettingsManager::GroupIds).toString ());
}

void GroupsMenu::populateMenuActions ()
{
  for (auto i = 0, end = view_->count (); i < end; ++i)
  {
    auto &group = view_->at (i);
    auto action = menu_->addAction (group.name ());
    action->setCheckable (true);
    actions_->addAction (action);
  }
}

void GroupsMenu::add ()
{
  auto &group = view_->add ();

  auto action = menu_->addAction (group.name ());
  action->setCheckable (true);
  actions_->addAction (action);
  trigger (action);

  updateMenuState ();
  updateShortcuts ();
}

void GroupsMenu::renameCurrent ()
{
  auto &group = view_->current ();
  const auto newName = QInputDialog::getText (view_, {}, tr ("Group title"),
                                              QLineEdit::Normal, group.name ());

  if (newName.isEmpty ())
  {
    return;
  }

  view_->renameCurrent (newName);
  const auto index = view_->currentIndex ();
  actionAt (index)->setText (group.name ());
}

void GroupsMenu::removeCurrent ()
{
  auto &group = view_->current ();
  const auto res = QMessageBox::question (view_, {}, tr ("Close group \"%1\"?")
                                          .arg (group.name ()));
  if (res != QMessageBox::Yes)
  {
    return;
  }

  const auto index = view_->currentIndex ();
  auto action = actionAt (index);
  actions_->removeAction (action);
  menu_->removeAction (action);
  view_->removeCurrent ();

  trigger (actionAt (0));

  updateMenuState ();
  updateShortcuts ();
}

void GroupsMenu::updateMenuState ()
{
  closeAction_->setEnabled (view_->count () > 1);
}

void GroupsMenu::trigger (QAction *action)
{
  action->setChecked (true);
  action->trigger ();
}

void GroupsMenu::setCurrent (QAction *action)
{
  ASSERT (action);
  auto index = actions_->actions ().indexOf (action);
  ASSERT (index < view_->count ());
  view_->setCurrentIndex (index);
}

void GroupsMenu::updateShortcuts ()
{
  auto index = -1;
  const auto count = ids_.length ();
  const auto commonPart = ShortcutManager::get (ShortcutManager::SwitchGroup).toString ();
  for (auto &i: actions_->actions ())
  {
    if (++index < count && !commonPart.isEmpty ())
    {
      i->setShortcut (QString ("%1,%2").arg (commonPart).arg (ids_.at (index)));
    }
    else
    {
      i->setShortcut ({});
    }
  }
}

QAction * GroupsMenu::actionAt (int index) const
{
  const auto actions = menu_->actions ();
  const auto menuIndex = actions.size () - (view_->count () - index);
  ASSERT (menuIndex >= 0);
  ASSERT (menuIndex < actions.size ());
  return actions[menuIndex];
}

int GroupsMenu::index (QAction *action) const
{
  return actions_->actions ().indexOf (action);
}

const QString &GroupsMenu::ids () const
{
  return ids_;
}

void GroupsMenu::setIds (const QString &ids)
{
  ids_ = utils::uniqueChars (ids);
  updateShortcuts ();
}

#include "moc_groupsmenu.cpp"
