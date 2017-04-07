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
  widgets_ (),
  layout_ (new QGridLayout),
  menu_ (new QMenu (tr ("File"), this))
{
  model_->setRootPath (QDir::rootPath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs);

  setWindowFlags (Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

  auto toolbar = new QToolBar (this);
  auto add = toolbar->addAction (QIcon::fromTheme ("add"), tr ("add"));
  connect (add, &QAction::triggered, this, &MultiDirWidget::addWidget);

  menu_->addAction (add);
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &MultiDirWidget::showContextMenu);


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
  auto *w = new DirWidget (model_, this);
  widgets_ << w;
  connect (w, &DirWidget::closeRequested,
           this, &MultiDirWidget::close);
  connect (w, &DirWidget::cloneRequested,
           this, &MultiDirWidget::clone);
  addToLayout (w);
  return w;
}

void MultiDirWidget::addToLayout (DirWidget *widget)
{
  const auto cols = layout_->columnCount ();
  const auto rows = layout_->rowCount ();
  for (auto row = 0; row < rows; ++row)
  {
    for (auto col = 0; col < cols; ++col)
    {
      auto item = layout_->itemAtPosition (row, col);
      if (!item)
      {
        layout_->addWidget (widget, row, col);
        return;
      }
    }
  }

  if (cols > rows)
  {
    layout_->addWidget (widget, rows, 0);
  }
  else
  {
    layout_->addWidget (widget, 0, cols);
  }
}

void MultiDirWidget::showContextMenu ()
{
  menu_->exec (QCursor::pos ());
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
