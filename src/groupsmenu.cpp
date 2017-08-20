#include "groupsmenu.h"
#include "groupsview.h"
#include "groupwidget.h"
#include "debug.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "settingsmanager.h"

#include <QInputDialog>
#include <QMessageBox>


GroupsMenu::GroupsMenu (GroupsView *view, QWidget *parent) :
  QMenu (tr ("Groups"), parent),
  view_ (view),
  actions_ (new QActionGroup (this)),
  ids_ ()
{
  connect (view_, &GroupsView::restored,
           this, &GroupsMenu::handleViewRestored);

  connect (actions_, &QActionGroup::triggered,
           this, &GroupsMenu::setCurrent);

  auto addGroup = ShortcutManager::create (this, ShortcutManager::AddGroup);
  connect (addGroup, &QAction::triggered, this, &GroupsMenu::add);

  renameAction_ = ShortcutManager::create (this, ShortcutManager::RenameGroup);
  connect (renameAction_, &QAction::triggered, this, &GroupsMenu::renameCurrent);

  closeAction_ = ShortcutManager::create (this, ShortcutManager::RemoveGroup);
  connect (closeAction_, &QAction::triggered, this, &GroupsMenu::removeCurrent);

  addSeparator ();

  SettingsManager::subscribeForUpdates (this);
  updateSettings ();
}

GroupsMenu::~GroupsMenu ()
{
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
    auto action = addAction (group.name ());
    action->setCheckable (true);
    actions_->addAction (action);
  }
}

void GroupsMenu::add ()
{
  auto &group = view_->add ();

  auto action = addAction (group.name ());
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
  removeAction (action);
  view_->removeCurrent ();

  trigger (actionAt (0));

  updateMenuState ();
  updateShortcuts ();
}

void GroupsMenu::handleViewRestored ()
{

  populateMenuActions ();
  updateMenuState ();
  updateShortcuts ();

  const auto current = view_->currentIndex ();
  trigger (actionAt (current));
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
  const auto actions = this->actions ();
  const auto menuIndex = actions.size () - (view_->count () - index);
  ASSERT (menuIndex >= 0);
  ASSERT (menuIndex < actions.size ());
  return actions[menuIndex];
}

void GroupsMenu::setIds (const QString &ids)
{
  ids_ = utils::uniqueChars (ids);
  updateShortcuts ();
}

#include "moc_groupsmenu.cpp"
