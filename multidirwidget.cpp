#include "multidirwidget.h"
#include "dirwidget.h"
#include "tiledview.h"
#include "debug.h"

#include <QBoxLayout>
#include <QSettings>
#include <QDir>

namespace
{
const QString qs_dirs = "dirs";
const QString qs_view = "view";
}


MultiDirWidget::MultiDirWidget (FileSystemModel &model, QWidget *parent) :
  QWidget (parent),
  model_ (&model),
  widgets_ (),
  view_ (new TiledView (this))
{
  auto layout = new QVBoxLayout (this);
  layout->addWidget (view_);
}

MultiDirWidget::~MultiDirWidget ()
{

}

void MultiDirWidget::save (QSettings &settings) const
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

void MultiDirWidget::restore (QSettings &settings)
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

void MultiDirWidget::setNameFilter (const QString &filter)
{
  for (auto &i: qAsConst (widgets_))
  {
    i->setNameFilter (filter);
  }
}

DirWidget * MultiDirWidget::addWidget ()
{
  auto *w = new DirWidget (model_, this);
  widgets_ << w;
  connect (w, &DirWidget::closeRequested,
           this, &MultiDirWidget::close);
  connect (w, &DirWidget::cloneRequested,
           this, &MultiDirWidget::clone);
  connect (w, &DirWidget::newTabRequested,
           this, &MultiDirWidget::add);
  connect (w, &DirWidget::consoleRequested,
           this, &MultiDirWidget::consoleRequested);
  w->setPath (QDir::homePath ());
  view_->add (*w);
  updateWidgetNames ();
  return w;
}

void MultiDirWidget::updateWidgetNames ()
{
  auto i = 0;
  for (auto &w: qAsConst (widgets_))
  {
    w->setObjectName (QString::number (++i));
  }
}

void MultiDirWidget::close (DirWidget *widget)
{
  widgets_.removeAll (widget);
  view_->remove (*widget);
  widget->deleteLater ();
  updateWidgetNames ();
}

void MultiDirWidget::clone (DirWidget *widget)
{
  auto w = addWidget ();
  w->setPath (widget->path ());
}

void MultiDirWidget::add (const QFileInfo &path)
{
  auto w = addWidget ();
  w->setPath (path);
}
