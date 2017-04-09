#include "multidirwidget.h"
#include "dirwidget.h"
#include "filesystemmodel.h"

#include <QGridLayout>
#include <QBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QSettings>
#include <QKeyEvent>
#include <QLineEdit>

#include <QDebug>

#include <math.h>

namespace
{
const QString qs_dirs = "dirs";
}

MultiDirWidget::MultiDirWidget (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  widgets_ (),
  layout_ (new QGridLayout),
  menu_ (new QMenu (tr ("File"), this)),
  findEdit_ (new QLineEdit (this))
{
  model_->setRootPath (QDir::homePath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs);
  model_->setReadOnly (false);

  setWindowFlags (Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

  auto toolbar = new QToolBar (this);
  auto add = toolbar->addAction (QIcon (":/add.png"), tr ("Add"));
  add->setShortcut (QKeySequence::AddTab);
  connect (add, &QAction::triggered, this, &MultiDirWidget::addWidget);

  auto find = toolbar->addAction (QIcon (":/find.png"), tr ("Find"));
  find->setShortcut (QKeySequence::Find);
  connect (find, &QAction::triggered, this, &MultiDirWidget::activateFindMode);

  menu_->addAction (add);
  menu_->addAction (find);
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &MultiDirWidget::showContextMenu);

  findEdit_->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
  findEdit_->setPlaceholderText (tr ("Name pattern"));
  findEdit_->setVisible (false);

  auto toolBarLayout = new QHBoxLayout;
  toolBarLayout->addWidget (toolbar);
  toolBarLayout->addWidget (findEdit_);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (toolBarLayout);
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
    widget->setPath (QDir::homePath ());
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
  connect (findEdit_, &QLineEdit::textChanged,
           w, &DirWidget::setNameFilter);
  w->setPath (QDir::homePath ());
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

void MultiDirWidget::activateFindMode ()
{
  findEdit_->show ();
  findEdit_->setFocus ();
}

void MultiDirWidget::keyPressEvent (QKeyEvent *event)
{
  if (event->key () == Qt::Key_Escape)
  {
    if (findEdit_->hasFocus ())
    {
      findEdit_->clear ();
      findEdit_->hide ();
    }
    else
    {
      hide ();
    }
    event->accept ();
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
