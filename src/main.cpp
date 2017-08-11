#include "mainwindow.h"
#include "openwith.h"
#include "debug.h"
#include "settingseditor.h"
#include "settingsmanager.h"
#include "constants.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QCommandLineParser>
#include <QCommandLineOption>


int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName (QLatin1String ("Gres"));
  a.setApplicationName (constants::appName);
  a.setApplicationVersion (constants::version);
#ifndef DEVELOPMENT
  a.setQuitOnLastWindowClosed (false);
#endif

  QLockFile f (QDir::home ().absoluteFilePath (QLatin1String (".multidir.lock")));
  if (!f.tryLock ())
  {
    LWARNING () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  {
    QCommandLineParser parser;
    parser.setApplicationDescription (QLatin1String ("Multiple directory management tool"));
    parser.addVersionOption ();
    parser.addHelpOption ();

    QCommandLineOption portable ("portable",
                                 QLatin1String ("Write settings into working directory"));
    parser.addOption (portable);

    parser.process (a);

    SettingsManager::setPortable (parser.isSet (portable));
  }

  OpenWith::init ();
  SettingsEditor::initOrphanSettings ();

  MainWindow window;
  return a.exec ();
}
