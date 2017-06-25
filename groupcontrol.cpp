#include "groupcontrol.h"
#include "groupholder.h"
#include "groupwidget.h"
#include "debug.h"
#include "shortcutmanager.h"
#include "utils.h"

#include <QMenu>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>

namespace
{
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
}

GroupControl::GroupControl (GroupHolder &view, QObject *parent) :
  QObject (parent),
  view_ (view),
  menu_ (new QMenu (tr ("Groups"))),
  actions_ (new QActionGroup (this)),
  ids_ (utils::uniqueChars (QLatin1String ("1234567890QWERTYUIOPASDFGHJKLZXCVBNM")))
{
  connect (actions_, &QActionGroup::triggered,
           this, &GroupControl::setCurrent);

  auto addGroup = menu_->addAction (tr ("Add"));
  ShortcutManager::add (ShortcutManager::AddGroup, addGroup);
  connect (addGroup, &QAction::triggered, this, &GroupControl::add);

  renameAction_ = menu_->addAction (tr ("Rename..."));
  ShortcutManager::add (ShortcutManager::RenameGroup, renameAction_);
  connect (renameAction_, &QAction::triggered, this, &GroupControl::renameCurrent);

  closeAction_ = menu_->addAction (tr ("Close..."));
  ShortcutManager::add (ShortcutManager::RemoveGroup, closeAction_);
  connect (closeAction_, &QAction::triggered, this, &GroupControl::removeCurrent);

  menu_->addSeparator ();
}

GroupControl::~GroupControl ()
{
  menu_->deleteLater ();
}

QMenu * GroupControl::menu () const
{
  return menu_;
}

void GroupControl::save (QSettings &settings) const
{
  view_.save (settings);
}

void GroupControl::restore (QSettings &settings)
{
  view_.restore (settings);

  populateMenuActions ();
  updateMenuState ();
  updateShortcuts ();

  const auto current = view_.currentIndex ();
  trigger (actionAt (current));
}

void GroupControl::populateMenuActions ()
{
  for (auto i = 0, end = view_.count (); i < end; ++i)
  {
    auto &group = view_.at (i);
    auto action = menu_->addAction (group.name ());
    action->setCheckable (true);
    actions_->addAction (action);
  }
}

void GroupControl::add ()
{
  auto &group = view_.add ();

  auto action = menu_->addAction (group.name ());
  action->setCheckable (true);
  actions_->addAction (action);
  trigger (action);

  updateMenuState ();
  updateShortcuts ();
}

void GroupControl::renameCurrent ()
{
  auto &group = view_.current ();
  const auto newName = QInputDialog::getText (&view_, {}, tr ("Group title"),
                                              QLineEdit::Normal, group.name ());

  if (newName.isEmpty ())
  {
    return;
  }

  view_.renameCurrent (newName);
  const auto index = view_.currentIndex ();
  actionAt (index)->setText (group.name ());
}

void GroupControl::removeCurrent ()
{
  auto &group = view_.current ();
  const auto res = QMessageBox::question (&view_, {}, tr ("Close group \"%1\"?").arg (group.name ()),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res != QMessageBox::Yes)
  {
    return;
  }

  const auto index = view_.currentIndex ();
  auto action = actionAt (index);
  actions_->removeAction (action);
  menu_->removeAction (action);
  view_.removeCurrent ();

  trigger (actionAt (0));

  updateMenuState ();
  updateShortcuts ();
}

void GroupControl::updateMenuState ()
{
  closeAction_->setEnabled (view_.count () > 1);
}

void GroupControl::trigger (QAction *action)
{
  action->setChecked (true);
  action->trigger ();
}

void GroupControl::setCurrent (QAction *action)
{
  ASSERT (action);
  auto index = actions_->actions ().indexOf (action);
  ASSERT (index < view_.count ());
  view_.setCurrentIndex (index);
}

void GroupControl::updateShortcuts ()
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

QAction * GroupControl::actionAt (int index) const
{
  const auto actions = menu_->actions ();
  const auto menuIndex = actions.size () - (view_.count () - index);
  ASSERT (menuIndex >= 0);
  ASSERT (menuIndex < actions.size ());
  return actions[menuIndex];
}

int GroupControl::index (QAction *action) const
{
  return actions_->actions ().indexOf (action);
}

const QString &GroupControl::ids () const
{
  return ids_;
}

void GroupControl::setIds (const QString &ids)
{
  ids_ = utils::uniqueChars (ids);
  updateShortcuts ();
}
