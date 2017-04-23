#include "multidirwidget.h"
#include "dirwidget.h"
#include "filesystemmodel.h"

#include <QGridLayout>
#include <QBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>
#include <QKeyEvent>
#include <QLineEdit>
#include <QApplication>
#include <QMessageBox>

#include <QDebug>

namespace
{
const QString qs_dirs = "dirs";
const QString qs_geometry = "geometry";
}


MultiDirWidget::MultiDirWidget (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  widgets_ (),
  layout_ (new QGridLayout),
  contextMenu_ (new QMenu (tr ("Context"), this)),
  findEdit_ (new QLineEdit (this))
{
  setWindowTitle (tr ("MultiDir"));
  setWindowIcon (QIcon (":/app.png"));

  model_->setRootPath (QDir::homePath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs);
  model_->setReadOnly (false);


  auto add = contextMenu_->addAction (QIcon (":/add.png"), tr ("Add"));
  add->setShortcut (QKeySequence::AddTab);
  connect (add, &QAction::triggered, this, &MultiDirWidget::addWidget);

  auto find = contextMenu_->addAction (QIcon (":/find.png"), tr ("Find"));
  find->setShortcut (QKeySequence::Find);
  connect (find, &QAction::triggered, this, &MultiDirWidget::activateFindMode);

  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &MultiDirWidget::showContextMenu);


  auto menuBar = new QMenuBar (this);
  auto fileMenu = menuBar->addMenu (tr ("File"));
  fileMenu->addAction (add);
  fileMenu->addAction (find);
  fileMenu->addSeparator ();
  auto settings = fileMenu->addAction (QIcon (":/settings.png"), tr ("Settings"));
  connect (settings, &QAction::triggered, this, &MultiDirWidget::settingsRequested);

  auto quit = fileMenu->addAction (QIcon (":/quit.png"), tr ("Quit"));
  connect (quit, &QAction::triggered, qApp, &QApplication::quit);


  auto helpMenu = menuBar->addMenu (tr ("Help"));
  auto about = helpMenu->addAction (QIcon::fromTheme ("about"), tr ("About"));
  connect (about, &QAction::triggered, this, &MultiDirWidget::showAbout);



  findEdit_->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
  findEdit_->setPlaceholderText (tr ("Name pattern"));
  findEdit_->setVisible (false);

  auto menuBarLayout = new QHBoxLayout;
  menuBarLayout->addWidget (menuBar);
  menuBarLayout->addWidget (findEdit_);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (menuBarLayout);
  layout->addLayout (layout_);
}

MultiDirWidget::~MultiDirWidget ()
{

}

void MultiDirWidget::save (QSettings &settings) const
{
  settings.setValue (qs_geometry, saveGeometry ());

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
  contextMenu_->exec (QCursor::pos ());
}

void MultiDirWidget::activateFindMode ()
{
  findEdit_->show ();
  findEdit_->setFocus ();
}

void MultiDirWidget::showAbout ()
{
#define STR2(XXX) #XXX
#define STR(XXX) STR2 (XXX)
  QStringList lines {
    tr ("<b>%1</b> version %2").arg (windowTitle (), STR (APP_VERSION)),
    tr ("Author: Gres (<a href='mailto:%1'>%1</a>)").arg ("multidir@gres.biz"),
    tr ("Homepage: <a href='%1'>%1</a>").arg ("https://gres.biz/multidir"),
    tr ("Sources: <a href='%1'>%1</a>").arg ("https://github.com/onemoregres/multidir"),
    "",
    tr ("Icons designed by Madebyoliver from Flaticon"),
    "",
    tr ("This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.")
  };
  QMessageBox about (QMessageBox::Information, windowTitle (),
                     lines.join ("<br>"), QMessageBox::Ok);
  about.setIconPixmap (QPixmap (":/app.png").scaledToHeight (100, Qt::SmoothTransformation));
  about.exec ();
#undef STR
#undef STR
}

void MultiDirWidget::keyPressEvent (QKeyEvent *event)
{
  if (event->key () == Qt::Key_Escape)
  {
    if (findEdit_->isVisible ())
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

void MultiDirWidget::add (const QFileInfo &path)
{
  auto w = addWidget ();
  w->setPath (path);
}
