#include "controller.h"
#include "multidirwidget.h"
#include "globalaction.h"
#include "settings.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>
#include <QSettings>
#include <QProcess>

#include <QDebug>

namespace
{
const QString qs_hotkey = "hotkey";
const QString qs_console = "console";
}

Controller::Controller (QObject *parent) :
  QObject (parent),
  tray_ (new QSystemTrayIcon (this)),
  widget_ (new MultiDirWidget),
  toggleAction_ (nullptr),
  consoleCommand_ ()
{
  connect (widget_.data (), &MultiDirWidget::settingsRequested,
           this, &Controller::editSettings);
  connect (widget_.data (), &MultiDirWidget::consoleRequested,
           this, &Controller::openConsole);

  auto menu = new QMenu;
  connect (menu, &QMenu::aboutToShow,
           this, &Controller::updateMenu);

  toggleAction_ = menu->addAction (QIcon (":/popup.png"), tr ("Toggle"));
  toggleAction_->setCheckable (true);
  toggleAction_->setChecked (true);
  GlobalAction::init ();

  auto quit = menu->addAction (QIcon (":/quit.png"), tr ("Quit"));
  connect (quit, &QAction::triggered,
           qApp, &QApplication::quit);


  tray_->setContextMenu (menu);
  tray_->setToolTip (tr ("MultiDir"));
  tray_->setIcon (QIcon (":/app.png"));
  tray_->show ();
  connect (tray_, &QSystemTrayIcon::activated,
           this, &Controller::trayClicked);

  updateMenu ();

  QSettings settings;
  restore (settings);
}

Controller::~Controller ()
{
  QSettings settings;
  save (settings);
}

void Controller::save (QSettings &settings) const
{
  settings.setValue (qs_hotkey, toggleAction_->shortcut ().toString ());
  settings.setValue (qs_console, consoleCommand_);

  widget_->save (settings);
}

void Controller::restore (QSettings &settings)
{
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

  widget_->restore (settings);
  widget_->show ();
  widget_->activateWindow ();
}

void Controller::updateMenu ()
{
  disconnect (toggleAction_, &QAction::toggled,
              this, &Controller::toggleWidget);
  toggleAction_->setChecked (widget_->isVisible ());
  connect (toggleAction_, &QAction::toggled,
           this, &Controller::toggleWidget);
}

void Controller::trayClicked (QSystemTrayIcon::ActivationReason reason)
{
  if (reason == QSystemTrayIcon::ActivationReason::Trigger)
  {
    toggleWidget ();
  }
}

void Controller::toggleWidget ()
{
  if (widget_->isVisible () && widget_->isActiveWindow ())
  {
    widget_->hide ();
  }
  else
  {
    widget_->show ();
    widget_->activateWindow ();
  }
}

void Controller::editSettings ()
{
  Settings settings;
  settings.setHotkey (toggleAction_->shortcut ());
  settings.setConsole (consoleCommand_);

  if (settings.exec () == QDialog::Accepted)
  {
    GlobalAction::removeGlobal (toggleAction_);
    toggleAction_->setShortcut (settings.hotkey ());
    consoleCommand_ = settings.console ();
    GlobalAction::makeGlobal (toggleAction_);
  }
}

void Controller::openConsole (const QString &path)
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
