#include "groupwidget.h"
#include "dirwidget.h"
#include "tiledview.h"
#include "backport.h"
#include "fileoperation.h"
#include "debug.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDir>
#include <QAction>

using namespace nonstd;

namespace
{
const QString qs_dirs = "dirs";
const QString qs_view = "view";
const QString qs_name = "name";
}


GroupWidget::GroupWidget (FileSystemModel &model, QWidget *parent) :
  QWidget (parent),
  name_ (),
  model_ (&model),
  widgets_ (),
  view_ (new TiledView (this))
{
  auto layout = new QVBoxLayout (this);
  layout->addWidget (view_);
}

GroupWidget::~GroupWidget ()
{

}

void GroupWidget::save (QSettings &settings) const
{
  settings.setValue (qs_name, name_);
  settings.beginWriteArray (qs_dirs, widgets_.size ());
  for (auto i = 0, end = widgets_.size (); i < end; ++i)
  {
    settings.setArrayIndex (i);
    widgets_[i].widget->save (settings);
  }
  settings.endArray ();

  view_->save (settings);
}

void GroupWidget::restore (QSettings &settings)
{
  ASSERT (widgets_.isEmpty ());

  name_ = settings.value (qs_name, name_).toString ();

  auto size = settings.beginReadArray (qs_dirs);
  for (auto i = 0; i < size; ++i)
  {
    settings.setArrayIndex (i);
    auto widget = addWidget ();
    widget->restore (settings);
  }
  settings.endArray ();

  if (widgets_.isEmpty ())
  {
    auto widget = addWidget ();
    widget->setPath (QDir::homePath ());
  }

  view_->restore (settings);
}

void GroupWidget::setNameFilter (const QString &filter)
{
  for (auto &i: as_const (widgets_))
  {
    i.widget->setNameFilter (filter);
  }
}

DirWidget * GroupWidget::addWidget ()
{
  auto *w = new DirWidget (model_, this);
  auto *action = new QAction (this);
  widgets_ << Widget {w, action};

  connect (w, &DirWidget::closeRequested,
           this, &GroupWidget::close);
  connect (w, &DirWidget::cloneRequested,
           this, &GroupWidget::clone);
  connect (w, &DirWidget::newTabRequested,
           this, &GroupWidget::add);
  connect (w, &DirWidget::consoleRequested,
           this, &GroupWidget::consoleRequested);
  connect (w, &DirWidget::editorRequested,
           this, &GroupWidget::editorRequested);
  connect (w, &DirWidget::fileOperation,
           this, &GroupWidget::fileOperation);

  w->setPath (QDir::homePath ());
  view_->add (*w);
  updateWidgetNames ();
  updateWidgetShortcuts ();

  this->addAction (action);
  connect (action, &QAction::triggered,
           this, [w] {w->activate ();});

  return w;
}

void GroupWidget::updateWidgetNames ()
{
  auto i = 0;
  for (auto &w: as_const (widgets_))
  {
    w.widget->setObjectName (QString::number (++i));
  }
}

void GroupWidget::updateWidgetShortcuts ()
{
  auto index = -1;
  const QString chars = "1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
  const auto count = chars.length ();
  for (auto &i: widgets_)
  {
    if (++index < count)
    {
      i.action->setShortcut (QString ("Alt+T,%1").arg (chars.at (index)));
    }
    else
    {
      i.action->setShortcut ({});
    }
  }
}

QString GroupWidget::name () const
{
  return name_;
}

void GroupWidget::setName (const QString &name)
{
  name_ = name;
}

void GroupWidget::close (DirWidget *widget)
{
  widgets_.removeAll ({widget, nullptr});
  view_->remove (*widget);
  widget->deleteLater ();
  updateWidgetNames ();
  updateWidgetShortcuts ();
}

void GroupWidget::clone (DirWidget *widget)
{
  auto w = addWidget ();
  w->setPath (widget->path ());
}

void GroupWidget::add (const QFileInfo &path)
{
  auto w = addWidget ();
  w->setPath (path);
}

bool GroupWidget::Widget::operator== (const GroupWidget::Widget &r) const
{
  return widget == r.widget;
}
