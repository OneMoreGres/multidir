#include "controller.h"
#include "multidirwidget.h"
#include "globalaction.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QApplication>
#include <QSettings>

Controller::Controller (QObject *parent) :
  QObject (parent),
  tray_ (new QSystemTrayIcon (this)),
  widget_ (new MultiDirWidget),
  toggleAction_ (nullptr)
{
  auto menu = new QMenu;
  connect (menu, &QMenu::aboutToShow,
           this, &Controller::updateMenu);

  toggleAction_ = menu->addAction (QIcon (":/popup.png"), tr ("Toggle"));
  toggleAction_->setCheckable (true);
  toggleAction_->setChecked (true);
  toggleAction_->setShortcut (QKeySequence (QLatin1String ("Ctrl+Alt+D")));
  GlobalAction::init ();
  GlobalAction::makeGlobal (toggleAction_);

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
  widget_->save (settings);
}

void Controller::restore (QSettings &settings)
{
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
