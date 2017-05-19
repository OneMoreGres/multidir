#include "mainwindow.h"
#include "multidirwidget.h"
#include "globalaction.h"
#include "settings.h"
#include "updatechecker.h"
#include "filesystemmodel.h"
#include "constants.h"

#include <QSystemTrayIcon>
#include <QBoxLayout>
#include <QMenu>
#include <QApplication>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QLineEdit>
#include <QMenuBar>
#include <QKeyEvent>
#include <QMessageBox>
#include <QApplication>

#include <QDebug>

namespace
{
const QString qs_geometry = "geometry";
const QString qs_hotkey = "hotkey";
const QString qs_console = "console";
const QString qs_updates = "checkUpdates";
}

MainWindow::MainWindow (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  findEdit_ (new QLineEdit (this)),
  tray_ (new QSystemTrayIcon (this)),
  widget_ (new MultiDirWidget (*model_, this)),
  toggleAction_ (nullptr),
  consoleCommand_ (),
  checkUpdates_ (false)
{
  setWindowTitle (tr ("MultiDir"));
  setWindowIcon (QIcon (":/app.png"));

  model_->setRootPath (QDir::homePath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs | QDir::Hidden);
  model_->setReadOnly (false);


  //menu bar
  auto menuBar = new QMenuBar (this);

  auto fileMenu = menuBar->addMenu (tr ("File"));

  auto add = fileMenu->addAction (QIcon (":/add.png"), tr ("Add"));
  add->setShortcut (QKeySequence::AddTab);
  connect (add, &QAction::triggered, this, &MainWindow::addWidget);

  auto find = fileMenu->addAction (QIcon (":/find.png"), tr ("Find"));
  find->setShortcut (QKeySequence::Find);
  connect (find, &QAction::triggered, this, &MainWindow::activateFindMode);

  fileMenu->addSeparator ();

  auto settings = fileMenu->addAction (QIcon (":/settings.png"), tr ("Settings"));
  connect (settings, &QAction::triggered, this, &MainWindow::editSettings);

  auto quit = fileMenu->addAction (QIcon (":/quit.png"), tr ("Quit"));
  connect (quit, &QAction::triggered, qApp, &QApplication::quit);


  auto helpMenu = menuBar->addMenu (tr ("Help"));
  auto about = helpMenu->addAction (QIcon::fromTheme ("about"), tr ("About"));
  connect (about, &QAction::triggered, this, &MainWindow::showAbout);


  findEdit_->setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Fixed);
  findEdit_->setPlaceholderText (tr ("Name pattern"));
  findEdit_->setVisible (false);


  //tray menu
  auto trayMenu = new QMenu;
  connect (trayMenu, &QMenu::aboutToShow,
           this, &MainWindow::updateTrayMenu);

  toggleAction_ = trayMenu->addAction (QIcon (":/popup.png"), tr ("Toggle"));
  toggleAction_->setCheckable (true);
  toggleAction_->setChecked (true);
  GlobalAction::init ();

  trayMenu->addAction (quit);

  tray_->setContextMenu (trayMenu);
  tray_->setToolTip (tr ("MultiDir"));
  tray_->setIcon (QIcon (":/app.png"));
  tray_->show ();
  connect (tray_, &QSystemTrayIcon::activated,
           this, &MainWindow::trayClicked);

  updateTrayMenu ();


  auto layout = new QVBoxLayout (this);
  auto menuBarLayout = new QHBoxLayout;
  menuBarLayout->addWidget (menuBar);
  menuBarLayout->addWidget (findEdit_);
  layout->addLayout (menuBarLayout);
  layout->addWidget (widget_.data ());


  connect (widget_.data (), &MultiDirWidget::consoleRequested,
           this, &MainWindow::openConsole);
  connect (findEdit_, &QLineEdit::textChanged,
           widget_.data (), &MultiDirWidget::setNameFilter);

  QSettings qsettings;
  restore (qsettings);
}

MainWindow::~MainWindow ()
{
  QSettings settings;
  save (settings);
}

void MainWindow::save (QSettings &settings) const
{
  settings.setValue (qs_geometry, saveGeometry ());

  settings.setValue (qs_hotkey, toggleAction_->shortcut ().toString ());
  settings.setValue (qs_console, consoleCommand_);
  settings.setValue (qs_updates, checkUpdates_);

  widget_->save (settings);
}

void MainWindow::restore (QSettings &settings)
{
  restoreGeometry (settings.value (qs_geometry, saveGeometry ()).toByteArray ());

  QKeySequence hotkey (settings.value (qs_hotkey, QLatin1String ("Ctrl+Alt+D")).toString ());
  toggleAction_->setShortcut (hotkey);
  GlobalAction::makeGlobal (toggleAction_);

#ifdef Q_OS_LINUX
  const auto console = QString ("xterm");
#endif
#ifdef Q_OS_WIN
  const auto console = QString ("cmd");
#endif
  consoleCommand_ = settings.value (qs_console, console).toString ();

  setCheckUpdates (settings.value (qs_updates, checkUpdates_).toBool ());

  widget_->restore (settings);
  widget_->show ();
  widget_->activateWindow ();
}

void MainWindow::updateTrayMenu ()
{
  disconnect (toggleAction_, &QAction::toggled,
              this, &MainWindow::toggleVisible);
  toggleAction_->setChecked (widget_->isVisible ());
  connect (toggleAction_, &QAction::toggled,
           this, &MainWindow::toggleVisible);
}

void MainWindow::trayClicked (QSystemTrayIcon::ActivationReason reason)
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
    show ();
    activateWindow ();
  }
}

void MainWindow::editSettings ()
{
  Settings settings;
  settings.setHotkey (toggleAction_->shortcut ());
  settings.setConsole (consoleCommand_);
  settings.setCheckUpdates (checkUpdates_);

  if (settings.exec () == QDialog::Accepted)
  {
    GlobalAction::removeGlobal (toggleAction_);
    toggleAction_->setShortcut (settings.hotkey ());
    consoleCommand_ = settings.console ();
    setCheckUpdates (settings.checkUpdates ());
    GlobalAction::makeGlobal (toggleAction_);
  }
}

void MainWindow::openConsole (const QString &path)
{
  if (!consoleCommand_.isEmpty () && !path.isEmpty ())
  {
    auto command = consoleCommand_ + ' ';
    command.replace ("%d", path);
    QStringList parts;
    QChar separator;
    auto startIndex = -1;
    for (auto i = 0, end = command.size (); i < end; ++i)
    {
      if (separator.isNull ())
      {
        separator = (command[i] == '"' ? '"' : ' ');
        startIndex = i + int (separator == '"');
      }
      else
      {
        if (command[i] == separator)
        {
          parts << command.mid (startIndex, i - startIndex);
          separator = QChar ();
        }
      }
    }
    QProcess::startDetached (parts[0], parts.mid (1), path);
  }
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

void MainWindow::addWidget ()
{
  widget_->addWidget ();
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
    tr ("<b>%1</b> version %2").arg (windowTitle (), constants::version),
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
}
