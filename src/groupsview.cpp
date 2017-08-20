#include "groupsview.h"
#include "groupwidget.h"
#include "debug.h"

#include <QStackedWidget>
#include <QSettings>
#include <QBoxLayout>

namespace
{
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
}

GroupsView::GroupsView (QSharedPointer<DirWidgetFactory> widgetFactory, QWidget *parent) :
  QWidget (parent),
  widgetFactory_ (widgetFactory),
  groups_ (new QStackedWidget (this))
{
  auto layout = new QVBoxLayout (this);
  layout->setMargin (0);
  layout->addWidget (groups_);
}

void GroupsView::save (QSettings &settings) const
{
  settings.beginWriteArray (qs_groups, groups_->count ());
  for (auto i = 0, end = groups_->count (); i < end; ++i)
  {
    settings.setArrayIndex (i);
    at (i).save (settings);
  }
  settings.endArray ();

  settings.setValue (qs_currentGroup, groups_->currentIndex ());
}

void GroupsView::restore (QSettings &settings)
{
  auto size = settings.beginReadArray (qs_groups);
  for (auto i = 0; i < size; ++i)
  {
    settings.setArrayIndex (i);
    auto &group = add ();
    group.restore (settings);
  }
  settings.endArray ();

  if (count () == 0)
  {
    add ();
  }
  else
  {
    const auto current = settings.value (qs_currentGroup, 0).toInt ();
    if (current >= 0 && current < count ())
    {
      setCurrentIndex (current);
    }
  }

  emit restored ();
}

void GroupsView::addWidgetToCurrent ()
{
  current ().addWidget ();
}

int GroupsView::count () const
{
  return groups_->count ();
}

int GroupsView::currentIndex () const
{
  return groups_->currentIndex ();
}

void GroupsView::setCurrentIndex (int index)
{
  ASSERT (index < groups_->count ());
  groups_->setCurrentIndex (index);
  emit currentChanged (current ().name ());
}

GroupWidget &GroupsView::current () const
{
  return at (currentIndex ());
}

GroupWidget &GroupsView::at (int index) const
{
  ASSERT (index > -1);
  ASSERT (index < groups_->count ());

  return static_cast<GroupWidget &>(*groups_->widget (index));
}

GroupWidget &GroupsView::add ()
{
  auto group = new GroupWidget (widgetFactory_, this);
  const auto index = groups_->addWidget (group);
  groups_->setCurrentIndex (index);
  group->setName (tr ("Group %1").arg (index + 1));

  group->addWidget ();

  return *group;
}

void GroupsView::removeCurrent ()
{
  auto &group = current ();
  groups_->removeWidget (&group);
  group.deleteLater ();
}

void GroupsView::renameCurrent (const QString &newName)
{
  ASSERT (!newName.isEmpty ());
  auto &group = current ();
  group.setName (newName);

  emit currentChanged (newName);
}

#include "moc_groupsview.cpp"
