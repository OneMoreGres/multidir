#include "controller.h"
#include "multidirwidget.h"
#include "globalaction.h"
#include "settings.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>
#include <QSettings>

namespace
{
const QString qs_hotkey = "hotkey";
}

Controller::Controller (QObject *parent) :
  QObject (parent),
  tray_ (new QSystemTrayIcon (this)),
  widget_ (new MultiDirWidget),
  toggleAction_ (nullptr)
{
  connect (widget_.data (), &MultiDirWidget::settingsRequested,
           this, &Controller::editSettings);

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

  widget_->save (settings);
}

void Controller::restore (QSettings &settings)
{
  QKeySequence hotkey (settings.value (qs_hotkey, QLatin1String ("Ctrl+Alt+D")).toString ());
  toggleAction_->setShortcut (hotkey);
  GlobalAction::makeGlobal (toggleAction_);

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

  if (settings.exec () == QDialog::Accepted)
  {
    GlobalAction::removeGlobal (toggleAction_);
    toggleAction_->setShortcut (settings.hotkey ());
    GlobalAction::makeGlobal (toggleAction_);
  }
}
