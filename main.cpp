#include "controller.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QTranslator>
#include <QDebug>

int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName (QLatin1String ("Gres"));
  a.setApplicationName (QLatin1String ("MultiDir"));
  a.setQuitOnLastWindowClosed (false);

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

  QLockFile f (QDir::home ().absoluteFilePath (QLatin1String (".multidir.lock")));
  if (!f.tryLock ())
  {
    qDebug () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  Controller control;
  return a.exec ();
}
