#include "groupwidget.h"
#include "dirwidget.h"
#include "tiledview.h"
#include "backport.h"
#include "fileoperation.h"
#include "debug.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDir>

using namespace nonstd;

namespace
{
const QString qs_dirs = "dirs";
const QString qs_view = "view";
}


GroupWidget::GroupWidget (FileSystemModel &model, QWidget *parent) :
  QWidget (parent),
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
  settings.beginWriteArray (qs_dirs, widgets_.size ());
  for (auto i = 0, end = widgets_.size (); i < end; ++i)
  {
    settings.setArrayIndex (i);
    widgets_[i]->save (settings);
  }
  settings.endArray ();

  view_->save (settings);
}

void GroupWidget::restore (QSettings &settings)
{
  ASSERT (widgets_.isEmpty ());

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
    i->setNameFilter (filter);
  }
}

DirWidget * GroupWidget::addWidget ()
{
  auto *w = new DirWidget (model_, this);
  widgets_ << w;
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
  return w;
}

void GroupWidget::updateWidgetNames ()
{
  auto i = 0;
  for (auto &w: as_const (widgets_))
  {
    w->setObjectName (QString::number (++i));
  }
}

void GroupWidget::close (DirWidget *widget)
{
  widgets_.removeAll (widget);
  view_->remove (*widget);
  widget->deleteLater ();
  updateWidgetNames ();
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
