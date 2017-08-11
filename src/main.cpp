#include "mainwindow.h"
#include "openwith.h"
#include "debug.h"
#include "settingseditor.h"
#include "constants.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>


int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName (QLatin1String ("Gres"));
  a.setApplicationName (constants::appName);
#ifndef DEVELOPMENT
  a.setQuitOnLastWindowClosed (false);
#endif

  QLockFile f (QDir::home ().absoluteFilePath (QLatin1String (".multidir.lock")));
  if (!f.tryLock ())
  {
    LWARNING () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  OpenWith::init ();
  SettingsEditor::initOrphanSettings ();

  MainWindow window;
  return a.exec ();
}
