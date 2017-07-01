#include "mainwindow.h"
#include "openwith.h"
#include "shortcutmanager.h"
#include "debug.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QTranslator>

int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName (QLatin1String ("Gres"));
  a.setApplicationName (QLatin1String ("MultiDir"));
#ifndef DEVELOPMENT
  a.setQuitOnLastWindowClosed (false);
#endif

  {
    QStringList dirs {
      QLatin1String ("translations")
#ifdef Q_OS_LINUX
      , QLatin1String ("/usr/share/multidir/translations"),
      qgetenv ("APPDIR") + QLatin1String ("/usr/share/multidir/translations") // appimage
#endif
    };
    QStringList names {QLatin1String ("qt"), QLatin1String ("qtbase"),
                       QLatin1String ("multidir")};
    auto last = new QTranslator (&a);
    for (const auto &name: names)
    {
      for (const auto &dir: dirs)
      {
        if (last->load (QLocale (), name, QLatin1String ("_"), dir))
        {
          a.installTranslator (last);
          last = new QTranslator (&a);
        }
      }
    }
    last->deleteLater ();
  }

  ShortcutManager::setDefaults ();

  QLockFile f (QDir::home ().absoluteFilePath (QLatin1String (".multidir.lock")));
  if (!f.tryLock ())
  {
    WARNING () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  OpenWith::init ();

  MainWindow window;
  return a.exec ();
}
