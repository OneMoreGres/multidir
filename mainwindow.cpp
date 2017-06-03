#include "mainwindow.h"
#include "groupwidget.h"
#include "globalaction.h"
#include "settings.h"
#include "updatechecker.h"
#include "filesystemmodel.h"
#include "fileoperation.h"
#include "fileoperationwidget.h"
#include "constants.h"
#include "debug.h"
#include "notifier.h"

#include <QSystemTrayIcon>
#include <QStackedWidget>
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
#include <QPixmapCache>
#include <QStatusBar>
#include <QInputDialog>

#include <QDebug>

namespace
{
const QString qs_geometry = "geometry";
const QString qs_hotkey = "hotkey";
const QString qs_console = "console";
const QString qs_editor = "editor";
const QString qs_updates = "checkUpdates";
const QString qs_groups = "groups";
const QString qs_currentGroup = "currentGroup";
const QString qs_imageCacheSize = "imageCacheSize";
}

MainWindow::MainWindow (QWidget *parent) :
  QWidget (parent),
  model_ (new FileSystemModel (this)),
  findEdit_ (new QLineEdit (this)),
  fileOperationsLayout_ (new QHBoxLayout),
  tray_ (new QSystemTrayIcon (this)),
  groups_ (new QStackedWidget (this)),
  toggleAction_ (nullptr),
  groupsMenu_ (nullptr),
  renameGroupAction_ (nullptr),
  closeGroupAction_ (nullptr),
  groupsActions_ (new QActionGroup (this)),
  consoleCommand_ (),
  editorCommand_ (),
  checkUpdates_ (false)
{
  setWindowTitle (tr ("MultiDir"));
  setWindowIcon (QIcon (":/app.png"));

  auto status = new QStatusBar (this);
  Notifier::setMain (status);

  model_->setRootPath (QDir::homePath ());
  model_->setFilter (QDir::AllEntries | QDir::NoDot | QDir::AllDirs | QDir::Hidden);
  model_->setReadOnly (false);
  connect (model_, &FileSystemModel::fileOperation,
           this, &MainWindow::showFileOperation);


  connect (groupsActions_, &QActionGroup::triggered,
           this, &MainWindow::updateCurrentGroup);


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


  groupsMenu_ = menuBar->addMenu (tr ("Groups"));

  auto addGroup = groupsMenu_->addAction (tr ("Add"));
  connect (addGroup, &QAction::triggered, this, [this] {
    auto group = this->addGroup ();
    group->addWidget ();
  });

  renameGroupAction_ = groupsMenu_->addAction (tr ("Rename..."));
  connect (renameGroupAction_, &QAction::triggered, this, &MainWindow::renameGroup);

  closeGroupAction_ = groupsMenu_->addAction (tr ("Close..."));
  connect (closeGroupAction_, &QAction::triggered, this, &MainWindow::removeGroup);

  groupsMenu_->addSeparator ();


  auto helpMenu = menuBar->addMenu (tr ("Help"));

  auto debug = helpMenu->addAction (tr ("Debug mode"));
  debug->setCheckable (true);
  connect (debug, &QAction::toggled, this, [](bool isOn) {debug::setDebugMode (isOn);});

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


  auto menuBarLayout = new QHBoxLayout;
  menuBarLayout->addWidget (menuBar);
  menuBarLayout->addLayout (fileOperationsLayout_);
  menuBarLayout->addWidget (findEdit_);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (menuBarLayout);
  layout->addWidget (groups_);
  layout->addWidget (status);


  QSettings qsettings;
  restore (qsettings);
}

MainWindow::~MainWindow ()
{
  Notifier::setMain (nullptr);

#ifndef DEVELOPMENT
  QSettings settings;
  save (settings);
#endif
}

void MainWindow::save (QSettings &settings) const
{
  settings.setValue (qs_geometry, saveGeometry ());

  settings.setValue (qs_hotkey, toggleAction_->shortcut ().toString ());
  settings.setValue (qs_console, consoleCommand_);
  settings.setValue (qs_editor, editorCommand_);
  settings.setValue (qs_updates, checkUpdates_);
  settings.setValue (qs_imageCacheSize, QPixmapCache::cacheLimit ());

  settings.beginWriteArray (qs_groups, groups_->count ());
  for (auto i = 0, end = groups_->count (); i < end; ++i)
  {
    settings.setArrayIndex (i);
    group (i)->save (settings);
  }
  settings.endArray ();

  settings.setValue (qs_currentGroup, groups_->currentIndex ());
}

void MainWindow::restore (QSettings &settings)
{
  restoreGeometry (settings.value (qs_geometry, saveGeometry ()).toByteArray ());

  QKeySequence hotkey (settings.value (qs_hotkey, QLatin1String ("Ctrl+Alt+D")).toString ());
  toggleAction_->setShortcut (hotkey);
  GlobalAction::makeGlobal (toggleAction_);

#ifdef Q_OS_LINUX
  const auto console = QString ("xterm");
  const auto editor = QString ("gedit");
#endif
#ifdef Q_OS_WIN
  const auto console = QString ("cmd");
  const auto editor = QString ("notepad.exe");
#endif
  consoleCommand_ = settings.value (qs_console, console).toString ();
  editorCommand_ = settings.value (qs_editor, editor).toString ();

  setCheckUpdates (settings.value (qs_updates, checkUpdates_).toBool ());

  auto cacheSize = settings.value (qs_imageCacheSize, QPixmapCache::cacheLimit ()).toInt ();
  QPixmapCache::setCacheLimit (std::max (1, cacheSize));

  auto size = settings.beginReadArray (qs_groups);
  for (auto i = 0; i < size; ++i)
  {
    settings.setArrayIndex (i);
    auto group = addGroup ();
    group->restore (settings);
    auto action = groupsActions_->actions ().back ();
    action->setText (group->name ());
  }
  settings.endArray ();

  if (groups_->count () == 0)
  {
    auto group = addGroup ();
    group->addWidget ();
  }
  else
  {
    auto action = groupAction (settings.value (qs_currentGroup, 0).toInt ());
    action->setChecked (true);
    action->trigger ();
  }
}

void MainWindow::updateTrayMenu ()
{
  disconnect (toggleAction_, &QAction::toggled,
              this, &MainWindow::toggleVisible);
  toggleAction_->setChecked (groups_->isVisible ());
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
  settings.setEditor (editorCommand_);
  settings.setImageCacheSize (QPixmapCache::cacheLimit ());

  if (settings.exec () == QDialog::Accepted)
  {
    GlobalAction::removeGlobal (toggleAction_);
    toggleAction_->setShortcut (settings.hotkey ());
    consoleCommand_ = settings.console ();
    editorCommand_ = settings.editor ();
    setCheckUpdates (settings.checkUpdates ());
    QPixmapCache::setCacheLimit (settings.imageCacheSizeKb ());
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
    if (!QProcess::startDetached (parts[0], parts.mid (1), path))
    {
      Notifier::error (tr ("Failed to open console '%1' in '%2'")
                       .arg (consoleCommand_, path));
    }
  }
}

void MainWindow::openInEditor (const QString &path)
{
  if (!editorCommand_.isEmpty () && !path.isEmpty ())
  {
    auto command = editorCommand_ + ' ';
    if (command.contains ("%p"))
    {
      command.replace ("%p", path);
    }
    else
    {
      command += path;
    }
    if (QProcess::startDetached (command))
    {
      Notifier::error (tr ("Failed to open editor '%1'").arg (editorCommand_));
    }
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
  group (groups_->currentIndex ())->addWidget ();
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

void MainWindow::showFileOperation (QSharedPointer<FileOperation> operation)
{
  ASSERT (operation);
  if (operation->isFinished ())
  {
    return;
  }

  auto widget = new FileOperationWidget (operation);
  widget->hide ();
  fileOperationsLayout_->addWidget (widget);
  QTimer::singleShot (1000, widget, &QWidget::show);
}

void MainWindow::updateGroupsMenu ()
{
  closeGroupAction_->setEnabled (groups_->count () > 1);
}

GroupWidget * MainWindow::addGroup ()
{
  auto group = new GroupWidget (*model_, this);
  connect (group, &GroupWidget::consoleRequested,
           this, &MainWindow::openConsole);
  connect (group, &GroupWidget::editorRequested,
           this, &MainWindow::openInEditor);
  connect (findEdit_, &QLineEdit::textChanged,
           group, &GroupWidget::setNameFilter);
  connect (group, &GroupWidget::fileOperation,
           this, &MainWindow::showFileOperation);

  const auto index = groups_->addWidget (group);
  groups_->setCurrentIndex (index);
  group->setName (tr ("Group %1").arg (index + 1));

  auto action = groupsMenu_->addAction (group->name ());
  action->setCheckable (true);
  groupsActions_->addAction (action);
  action->setChecked (true);
  action->trigger ();

  updateGroupsMenu ();

  return group;
}

void MainWindow::updateCurrentGroup (QAction *groupAction)
{
  ASSERT (groupAction);
  auto index = groupsActions_->actions ().indexOf (groupAction);
  ASSERT (index < groups_->count ());
  groups_->setCurrentIndex (index);
}

GroupWidget * MainWindow::group (int index) const
{
  ASSERT (index > -1);
  ASSERT (index < groups_->count ());

  return static_cast<GroupWidget *>(groups_->widget (index));
}

QAction * MainWindow::groupAction (int index) const
{
  const auto actions = groupsMenu_->actions ();
  const auto menuIndex = actions.size () - (groups_->count () - index);
  ASSERT (menuIndex >= 0);
  ASSERT (menuIndex < actions.size ());
  return actions[menuIndex];
}

void MainWindow::removeGroup ()
{
  ASSERT (groups_->count () > 1);
  const auto name = groupsActions_->checkedAction ()->text ();
  const auto res = QMessageBox::question (this, {}, tr ("Close group \"%1\"?").arg (name),
                                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::No)
  {
    return;
  }

  auto w = groups_->currentWidget ();
  ASSERT (w);

  const auto index = groups_->currentIndex ();
  auto firstAction = groupAction (0);
  firstAction->setChecked (true);
  firstAction->trigger ();

  auto action = groupAction (index);
  groups_->removeWidget (w);
  groupsMenu_->removeAction (action);
  groupsActions_->removeAction (action);


  w->deleteLater ();
  updateGroupsMenu ();
}

void MainWindow::renameGroup ()
{
  ASSERT (groups_->count () > 1);
  const auto name = groupsActions_->checkedAction ()->text ();
  const auto newName = QInputDialog::getText (this, {}, tr ("Group title"),
                                              QLineEdit::Normal, name);
  if (!newName.isEmpty ())
  {
    const auto index = groups_->currentIndex ();
    group (index)->setName (newName);
    groupsActions_->checkedAction ()->setText (newName);
  }
}
