#include "groupview.h"
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

GroupView::GroupView (FileSystemModel &model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  groups_ (new QStackedWidget (this))
{
  auto layout = new QVBoxLayout (this);
  layout->addWidget (groups_);
}

void GroupView::save (QSettings &settings) const
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

void GroupView::restore (QSettings &settings)
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

void GroupView::addWidgetToCurrent ()
{
  current ().addWidget ();
}

int GroupView::count () const
{
  return groups_->count ();
}

int GroupView::currentIndex () const
{
  return groups_->currentIndex ();
}

void GroupView::setCurrentIndex (int index)
{
  ASSERT (index < groups_->count ());
  groups_->setCurrentIndex (index);
  emit currentChanged (current ().name ());
}

GroupWidget &GroupView::current () const
{
  return at (currentIndex ());
}

GroupWidget &GroupView::at (int index) const
{
  ASSERT (index > -1);
  ASSERT (index < groups_->count ());

  return static_cast<GroupWidget &>(*groups_->widget (index));
}

GroupWidget &GroupView::add ()
{
  auto group = new GroupWidget (model_, this);
  connect (group, &GroupWidget::consoleRequested,
           this, &GroupView::consoleRequested);
  connect (group, &GroupWidget::editorRequested,
           this, &GroupView::editorRequested);
  connect (this, &GroupView::setNameFilter,
           group, &GroupWidget::setNameFilter);
  connect (group, &GroupWidget::fileOperation,
           this, &GroupView::fileOperation);

  const auto index = groups_->addWidget (group);
  groups_->setCurrentIndex (index);
  group->setName (tr ("Group %1").arg (index + 1));

  group->addWidget ();

  return *group;
}

void GroupView::removeCurrent ()
{
  auto &group = current ();
  groups_->removeWidget (&group);
  group.deleteLater ();
}

void GroupView::renameCurrent (const QString &newName)
{
  ASSERT (!newName.isEmpty ());
  auto &group = current ();
  group.setName (newName);

  emit currentChanged (newName);
}
