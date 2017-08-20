#include "mainwindow.h"
#include "groupsview.h"
#include "groupsmenu.h"
#include "settingseditor.h"
#include "updatechecker.h"
#include "filesystemmodel.h"
#include "fileoperation.h"
#include "fileoperationwidget.h"
#include "fileconflictresolver.h"
#include "constants.h"
#include "debug.h"
#include "notifier.h"
#include "shortcutmanager.h"
#include "settingsmanager.h"

#include <QSystemTrayIcon>
#include <QBoxLayout>
#include <QApplication>
#include <QTimer>
#include <QLineEdit>
#include <QMenuBar>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStatusBar>

namespace
{
const QString qs_geometry = "geometry";
}

MainWindow::MainWindow (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  groups_ (new GroupsView (model_, this)),
  conflictResolver_ (new FileConflictResolver),
  findEdit_ (new QLineEdit (this)),
  fileOperationsLayout_ (new QHBoxLayout),
  tray_ (new QSystemTrayIcon (this)),
  toggleAction_ (nullptr),
  checkUpdates_ (false),
  startInBackground_ (false)
{
  setObjectName ("main");
  setWindowIcon (QIcon (":/app.png"));

  auto status = new QStatusBar (this);
  Notifier::setMain (status);

  model_->setRootPath (QDir::rootPath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs | QDir::Hidden);
  model_->setReadOnly (false);
  connect (model_, &FileSystemModel::fileOperation,
           this, &MainWindow::showFileOperation);


  connect (findEdit_, &QLineEdit::textChanged,
           groups_, &GroupsView::setNameFilter);
  connect (groups_, &GroupsView::currentChanged,
           this, &MainWindow::updateWindowTitle);


  //menu bar
  auto menuBar = new QMenuBar (this);

  auto fileMenu = menuBar->addMenu (tr ("File"));

  auto add = ShortcutManager::create (this, ShortcutManager::AddTab, fileMenu);
  connect (add, &QAction::triggered, groups_, &GroupsView::addWidgetToCurrent);

  auto find = ShortcutManager::create (this, ShortcutManager::Find, fileMenu);
  connect (find, &QAction::triggered, this, &MainWindow::activateFindMode);

  fileMenu->addSeparator ();

  auto settings = ShortcutManager::create (this, ShortcutManager::Settings, fileMenu);
  connect (settings, &QAction::triggered, this, &MainWindow::editSettings);

  auto quit = ShortcutManager::create (this, ShortcutManager::Quit, fileMenu);
  connect (quit, &QAction::triggered, qApp, &QApplication::quit);


  auto groupsMenu = new GroupsMenu (groups_, this);
  menuBar->addMenu (groupsMenu);


  auto helpMenu = menuBar->addMenu (tr ("Help"));

  auto debug = ShortcutManager::create (this, ShortcutManager::Debug, helpMenu, true);
  connect (debug, &QAction::toggled, this, &debug::setDebugMode);

  auto about = ShortcutManager::create (this, ShortcutManager::About, helpMenu);
  connect (about, &QAction::triggered, this, &MainWindow::showAbout);


  findEdit_->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
  findEdit_->setPlaceholderText (tr ("Name pattern"));
  findEdit_->setVisible (false);


  //tray menu
  auto trayMenu = new QMenu;
  connect (trayMenu, &QMenu::aboutToShow,
           this, &MainWindow::updateTrayMenu);

  toggleAction_ = ShortcutManager::create (this, ShortcutManager::ToggleGui, trayMenu, true);
  toggleAction_->setChecked (true);

  trayMenu->addAction (quit);

  tray_->setContextMenu (trayMenu);
  tray_->setToolTip (constants::appName);
  tray_->setIcon (QIcon (":/app.png"));
  tray_->show ();
  connect (tray_, &QSystemTrayIcon::activated,
           this, &MainWindow::handleTrayClick);

  updateTrayMenu ();


  // layout
  auto menuBarLayout = new QHBoxLayout;
  menuBarLayout->addWidget (menuBar);
  menuBarLayout->addLayout (fileOperationsLayout_);
  menuBarLayout->addWidget (findEdit_);

  auto layout = new QVBoxLayout (this);
  layout->setMargin (3);
  layout->setSpacing (0);
  layout->addLayout (menuBarLayout);
  layout->addWidget (groups_);
  layout->addWidget (status);


  {
    QSettings settings;
    restore (settings);
  }

  SettingsManager::subscribeForUpdates (this);
  updateSettings ();

  if (!startInBackground_)
  {
    show ();
  }
}

MainWindow::~MainWindow ()
{
  conflictResolver_->deleteLater ();

  Notifier::setMain (nullptr);

#ifndef DEVELOPMENT
  QSettings settings;
  save (settings);
#endif
}

void MainWindow::save (QSettings &settings) const
{
  settings.setValue (qs_geometry, saveGeometry ());

  groups_->save (settings);
}

void MainWindow::restore (QSettings &settings)
{
  restoreGeometry (settings.value (qs_geometry, saveGeometry ()).toByteArray ());

  groups_->restore (settings);
}

void MainWindow::updateSettings ()
{
  SettingsManager settings;
  using Type = SettingsManager::Type;
  setCheckUpdates (settings.get (Type::CheckUpdates).toBool ());
  startInBackground_ = settings.get (Type::StartInBackground).toBool ();
}

void MainWindow::updateTrayMenu ()
{
  disconnect (toggleAction_, &QAction::toggled,
              this, &MainWindow::toggleVisible);
  toggleAction_->setChecked (isVisible ());
  connect (toggleAction_, &QAction::toggled,
           this, &MainWindow::toggleVisible);
}

void MainWindow::handleTrayClick (QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::ActivationReason::Trigger)
  {
    toggleVisible ();
  }
}

void MainWindow::toggleVisible ()
{
  if (isVisible () && isActiveWindow ())
  {
    hide ();
  }
  else
  {
    hide ();
    show ();
    raise ();
    activateWindow ();
  }
}

void MainWindow::editSettings ()
{
  SettingsEditor settings;
  settings.exec ();
}

void MainWindow::setCheckUpdates (bool isOn)
{
  if (checkUpdates_ == isOn)
  {
    return;
  }
  checkUpdates_ = isOn;

  if (isOn)
  {
    auto updater = new UpdateChecker (this);

    connect (updater, &UpdateChecker::updateAvailable,
             this, [this](const QString &version) {
               tray_->showMessage (tr ("Multidir update available"), tr ("New version: %1").arg (version));
             });
    connect (updater, &UpdateChecker::updateAvailable,
             updater, &QObject::deleteLater);
    connect (updater, &UpdateChecker::noUpdates,
             updater, &QObject::deleteLater);
    QTimer::singleShot (3000, updater, &UpdateChecker::check);
  }
}

void MainWindow::keyPressEvent (QKeyEvent *event)
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

void MainWindow::activateFindMode ()
{
  findEdit_->show ();
  findEdit_->setFocus ();
}

void MainWindow::showAbout ()
{
  QStringList lines {
    tr ("<b>%1</b> version %2").arg (constants::appName, constants::version),
    tr ("Author: Gres (<a href='mailto:%1'>%1</a>)").arg ("multidir@gres.biz"),
    tr ("Homepage: <a href='https://%1'>%1</a>").arg ("gres.biz/multidir"),
    tr ("Issues: <a href='https://%1'>%1</a>").arg ("github.com/onemoregres/multidir/issues"),
    tr ("Sources: <a href='https://%1'>%1</a>").arg ("github.com/onemoregres/multidir"),
    "",
    tr ("Icons designed by Madebyoliver from Flaticon"),
    "",
    tr ("This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.")
  };
  QMessageBox about (QMessageBox::Information, tr ("About"), lines.join ("<br>"));
  about.setIconPixmap (QPixmap (":/app.png").scaledToHeight (100, Qt::SmoothTransformation));
  about.exec ();
}

void MainWindow::showFileOperation (QSharedPointer<FileOperation> operation)
{
  ASSERT (operation);

  auto widget = new FileOperationWidget (operation);
  widget->hide ();
  fileOperationsLayout_->addWidget (widget);
  QTimer::singleShot (1000, widget, &QWidget::show);

  operation->startAsync (conflictResolver_);
}

void MainWindow::updateWindowTitle (const QString &groupName)
{
  setWindowTitle (constants::appName + QString (" - ") + groupName);
}

#include "moc_mainwindow.cpp"
