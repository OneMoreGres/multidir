#include "groupwidget.h"
#include "dirwidget.h"
#include "tiledview.h"
#include "backport.h"
#include "fileoperation.h"
#include "debug.h"
#include "shortcutmanager.h"
#include "utils.h"

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
  view_ (new TiledView (this)),
  ids_ (utils::uniqueChars (QLatin1String ("1234567890QWERTYUIOPASDFGHJKLZXCVBNM")))
{
  auto layout = new QVBoxLayout (this);
  layout->setMargin (0);
  layout->addWidget (view_);

  connect (view_, &TiledView::tileSwapped,
           this, &GroupWidget::updateWidgetShortcuts);
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
  for (auto &i: widgets_)
  {
    if (i.widget)
    {
      view_->remove (*i.widget);
      i.widget->deleteLater ();
    }
    if (i.action)
    {
      i.action->deleteLater ();
    }
  }
  widgets_.clear ();


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

  action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  addAction (action);

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
  const auto count = ids_.length ();
  const auto order = view_->widgets ();
  const auto commonPart = ShortcutManager::get (ShortcutManager::SwitchTab).toString ();
  for (auto &i: widgets_)
  {
    const auto index = order.indexOf (i.widget);
    const auto key = (index < count) ? ids_.at (index) : QChar ();
    i.widget->setIndex (key);
    if (!key.isNull () && !commonPart.isEmpty ())
    {
      i.action->setShortcut (QString ("%1,%2").arg (commonPart).arg (ids_.at (index)));
    }
    else
    {
      i.action->setShortcut ({});
    }
  }
}

const QString &GroupWidget::ids () const
{
  return ids_;
}

void GroupWidget::setIds (const QString &ids)
{
  ids_ = utils::uniqueChars (ids);
  updateWidgetShortcuts ();
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

  if (widgets_.isEmpty ())
  {
    addWidget ();
  }
}

void GroupWidget::clone (DirWidget *widget)
{
  auto w = addWidget ();
  w->setPath (widget->path ());
  w->activate ();
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
