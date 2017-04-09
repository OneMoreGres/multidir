#include "controller.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QTranslator>
#include <QDebug>

int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName ("Gres");
  a.setApplicationName ("MultiDir");
  a.setQuitOnLastWindowClosed (false);

  QTranslator baseTranslator;
  if (baseTranslator.load (QLocale (), QLatin1String ("qtbase"), QLatin1String ("_")))
  {
    a.installTranslator (&baseTranslator);
  }
  QTranslator appTranslator;
  if (appTranslator.load (QLocale (), QLatin1String ("multidir"), QLatin1String ("_")))
  {
    a.installTranslator (&appTranslator);
  }

  QLockFile f (QDir::home ().absoluteFilePath (".multidir.lock"));
  if (!f.tryLock ())
  {
    qDebug () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  Controller control;
  return a.exec ();
}
