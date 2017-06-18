#include "groupcontrol.h"
#include "groupview.h"
#include "groupwidget.h"
#include "debug.h"

#include <QMenu>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>

namespace
{
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
}

GroupControl::GroupControl (GroupView &view, QObject *parent) :
  QObject (parent),
  view_ (view),
  menu_ (new QMenu (tr ("Groups"))),
  actions_ (new QActionGroup (this))
{
  connect (actions_, &QActionGroup::triggered,
           this, &GroupControl::setCurrent);

  auto addGroup = menu_->addAction (tr ("Add"));
  connect (addGroup, &QAction::triggered, this, &GroupControl::add);

  renameAction_ = menu_->addAction (tr ("Rename..."));
  connect (renameAction_, &QAction::triggered, this, &GroupControl::renameCurrent);

  closeAction_ = menu_->addAction (tr ("Close..."));
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
  const QString chars = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
  const auto count = chars.length ();
  for (auto &i: actions_->actions ())
  {
    if (++index < count)
    {
      i->setShortcut (QString ("Alt+G,%1").arg (chars.at (index)));
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
