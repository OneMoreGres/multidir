#include "groupholder.h"
#include "groupwidget.h"
#include "fileoperation.h"
#include "debug.h"

#include <QMenu>
#include <QStackedWidget>
#include <QSettings>
#include <QBoxLayout>

namespace
{
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
}

GroupHolder::GroupHolder (FileSystemModel &model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  groups_ (new QStackedWidget (this))
{
  auto layout = new QVBoxLayout (this);
  layout->setMargin (0);
  layout->addWidget (groups_);
}

void GroupHolder::save (QSettings &settings) const
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

void GroupHolder::restore (QSettings &settings)
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
}

void GroupHolder::addWidgetToCurrent ()
{
  current ().addWidget ();
}

int GroupHolder::count () const
{
  return groups_->count ();
}

int GroupHolder::currentIndex () const
{
  return groups_->currentIndex ();
}

void GroupHolder::setCurrentIndex (int index)
{
  ASSERT (index < groups_->count ());
  groups_->setCurrentIndex (index);
  emit currentChanged (current ().name ());
}

GroupWidget &GroupHolder::current () const
{
  return at (currentIndex ());
}

GroupWidget &GroupHolder::at (int index) const
{
  ASSERT (index > -1);
  ASSERT (index < groups_->count ());

  return static_cast<GroupWidget &>(*groups_->widget (index));
}

GroupWidget &GroupHolder::add ()
{
  auto group = new GroupWidget (model_, this);
  connect (group, &GroupWidget::consoleRequested,
           this, &GroupHolder::consoleRequested);
  connect (group, &GroupWidget::editorRequested,
           this, &GroupHolder::editorRequested);
  connect (this, &GroupHolder::setNameFilter,
           group, &GroupWidget::setNameFilter);
  connect (group, &GroupWidget::fileOperation,
           this, &GroupHolder::fileOperation);

  const auto index = groups_->addWidget (group);
  groups_->setCurrentIndex (index);
  group->setName (tr ("Group %1").arg (index + 1));

  group->addWidget ();

  return *group;
}

void GroupHolder::removeCurrent ()
{
  auto &group = current ();
  groups_->removeWidget (&group);
  group.deleteLater ();
}

const QString &GroupHolder::widgetIds () const
{
  return current ().ids ();
}

void GroupHolder::setWidgetIds (const QString &ids)
{
  for (auto i = 0, end = count (); i < end; ++i)
  {
    at (i).setIds (ids);
  }
}

void GroupHolder::renameCurrent (const QString &newName)
{
  ASSERT (!newName.isEmpty ());
  auto &group = current ();
  group.setName (newName);

  emit currentChanged (newName);
}
