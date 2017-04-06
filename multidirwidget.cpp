#include "multidirwidget.h"
#include "dirwidget.h"

#include <QFileSystemModel>
#include <QGridLayout>
#include <QBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QSettings>
#include <QKeyEvent>

#include <QDebug>

#include <math.h>

namespace
{
const QString qs_dirs = "dirs";
}

MultiDirWidget::MultiDirWidget (QWidget *parent) :
  QWidget (parent),
  model_ (new QFileSystemModel (this)),
  layout_ (new QGridLayout)
{
  model_->setRootPath (QDir::rootPath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs);

  setWindowFlags (Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

  auto toolbar = new QToolBar (this);
  auto add = toolbar->addAction (QIcon::fromTheme ("add"), tr ("add"));
  connect (add, &QAction::triggered, this, &MultiDirWidget::addWidget);

  auto layout = new QVBoxLayout (this);
  layout->addWidget (toolbar);
  layout->addLayout (layout_);
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
}

void MultiDirWidget::restore (QSettings &settings)
{
  qDeleteAll (widgets_);
  widgets_.clear ();

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
    widget->setPath (model_->rootPath ());
  }
}

DirWidget * MultiDirWidget::addWidget ()
{
  auto i = widgets_.size ();
  auto *w = new DirWidget (model_, this);
  layout_->addWidget (w, i / 2, i % 2);
  widgets_ << w;
  connect (w, &DirWidget::closeRequested,
           this, &MultiDirWidget::close);
  connect (w, &DirWidget::cloneRequested,
           this, &MultiDirWidget::clone);
  return w;
}

void MultiDirWidget::keyPressEvent (QKeyEvent *event)
{
  if (event->key () == Qt::Key_Escape)
  {
    hide ();
  }
}

void MultiDirWidget::close (DirWidget *widget)
{
  widgets_.removeAll (widget);
  widget->deleteLater ();
}

void MultiDirWidget::clone (DirWidget *widget)
{
  auto w = addWidget ();
  w->setPath (widget->path ());
}
