#include "multidirwidget.h"
#include "dirwidget.h"
#include "filesystemmodel.h"

#include <QGridLayout>
#include <QBoxLayout>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QKeyEvent>
#include <QLineEdit>
#include <QApplication>

#include <QDebug>

namespace
{
const QString qs_dirs = "dirs";
const QString qs_overlay = "overlay";
const QString qs_extensive = "extensive";
const QString qs_geometry = "geometry";
}

#define STR2(XXX) #XXX
#define STR(XXX) STR2 (XXX)

MultiDirWidget::MultiDirWidget (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  widgets_ (),
  layout_ (new QGridLayout),
  contextMenu_ (new QMenu (tr ("Context"), this)),
  overlayAction_ (nullptr),
  extensiveAction_ (nullptr),
  findEdit_ (new QLineEdit (this))
{
  setWindowTitle (tr ("MultiDir") + " - v." STR (APP_VERSION));

  model_->setRootPath (QDir::homePath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs);
  model_->setReadOnly (false);


  auto toolbar = new QToolBar (this);
  auto add = toolbar->addAction (QIcon (":/add.png"), tr ("Add"));
  add->setShortcut (QKeySequence::AddTab);
  connect (add, &QAction::triggered, this, &MultiDirWidget::addWidget);

  auto find = toolbar->addAction (QIcon (":/find.png"), tr ("Find"));
  find->setShortcut (QKeySequence::Find);
  connect (find, &QAction::triggered, this, &MultiDirWidget::activateFindMode);

  toolbar->addSeparator ();

  auto settings = toolbar->addAction (QIcon (":/settings.png"), tr ("Settings"));
  connect (settings, &QAction::triggered, this, &MultiDirWidget::settingsRequested);

  overlayAction_ = toolbar->addAction (QIcon (":/overlayed.png"), tr ("Overlay mode"));
  overlayAction_->setCheckable (true);
  overlayAction_->setChecked (false);
  connect (overlayAction_, &QAction::toggled, this, &MultiDirWidget::setIsOverlay);

  extensiveAction_ = toolbar->addAction (QIcon (":/extensive.png"), tr ("Extensive mode"));
  extensiveAction_->setCheckable (true);
  extensiveAction_->setChecked (true);

  auto quit = toolbar->addAction (QIcon (":/quit.png"), tr ("Quit"));
  connect (quit, &QAction::triggered, qApp, &QApplication::quit);


  contextMenu_->addAction (add);
  contextMenu_->addAction (find);
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &MultiDirWidget::showContextMenu);


  auto menuBar = new QMenuBar (this);
  auto fileMenu = menuBar->addMenu (tr ("File"));
  fileMenu->addAction (add);
  fileMenu->addAction (find);
  fileMenu->addSeparator ();
  fileMenu->addAction (settings);
  fileMenu->addAction (quit);


  findEdit_->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
  findEdit_->setPlaceholderText (tr ("Name pattern"));
  findEdit_->setVisible (false);

  auto toolBarLayout = new QHBoxLayout;
  toolBarLayout->addWidget (toolbar);
  toolBarLayout->addWidget (findEdit_);

  auto layout = new QVBoxLayout (this);
  layout->addWidget (menuBar);
  layout->addLayout (toolBarLayout);
  layout->addLayout (layout_);
}

MultiDirWidget::~MultiDirWidget ()
{

}

void MultiDirWidget::save (QSettings &settings) const
{
  settings.setValue (qs_overlay, isOverlay ());
  settings.setValue (qs_geometry, saveGeometry ());
  settings.setValue (qs_extensive, extensiveAction_->isChecked ());

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
  overlayAction_->setChecked (settings.value (qs_overlay, isOverlay ()).toBool ());
  restoreGeometry (settings.value (qs_geometry, saveGeometry ()).toByteArray ());

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

  extensiveAction_->setChecked (settings.value (qs_extensive, false).toBool ());

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
  connect (extensiveAction_, &QAction::toggled,
           w, &DirWidget::setIsExtensiveView);
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
  contextMenu_->exec (QCursor::pos ());
}

void MultiDirWidget::activateFindMode ()
{
  findEdit_->show ();
  findEdit_->setFocus ();
}

void MultiDirWidget::setIsOverlay (bool isOn)
{
  const auto wasVisible = isVisible ();
  QByteArray state;
  if (wasVisible)
  {
    hide ();
    state = saveGeometry ();
  }
  auto flags = windowFlags ();
  flags.setFlag (Qt::Tool, isOn);
  flags.setFlag (Qt::WindowStaysOnTopHint, isOn);
  setWindowFlags (flags);
  if (wasVisible)
  {
    restoreGeometry (state);
    show ();
    activateWindow ();
  }
}

bool MultiDirWidget::isOverlay () const
{
  return overlayAction_->isChecked ();
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
