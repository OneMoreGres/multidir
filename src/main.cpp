#include "mainwindow.h"
#include "openwith.h"
#include "shortcutmanager.h"
#include "debug.h"
#include "translationloader.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>


int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName (QLatin1String ("Gres"));
  a.setApplicationName (QLatin1String ("MultiDir"));
#ifndef DEVELOPMENT
  a.setQuitOnLastWindowClosed (false);
#endif

  TranslationLoader::load ();

  ShortcutManager::setDefaults ();

  QLockFile f (QDir::home ().absoluteFilePath (QLatin1String (".multidir.lock")));
  if (!f.tryLock ())
  {
    LWARNING () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  OpenWith::init ();

  MainWindow window;
  return a.exec ();
}
