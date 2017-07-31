#include "translationloader.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

void TranslationLoader::load ()
{
  QStringList dirs {
    QLatin1String ("translations"),
    QLibraryInfo::location (QLibraryInfo::TranslationsPath)
#ifdef Q_OS_LINUX
    , QLatin1String ("/usr/share/multidir/translations"),
    qgetenv ("APPDIR") + QLatin1String ("/usr/share/multidir/translations"),   // appimage
    qgetenv ("APPDIR") + QLatin1String ("/translations"),   // appimage
#endif
  };
  QStringList names {QLatin1String ("qt"), QLatin1String ("qtbase"),
                     QLatin1String ("multidir")};
  auto last = new QTranslator (QApplication::instance ());
  for (const auto &name: names)
  {
    for (const auto &dir: dirs)
    {
      if (last->load (QLocale (), name, QLatin1String ("_"), dir))
      {
        QApplication::installTranslator (last);
        last = new QTranslator (QApplication::instance ());
      }
    }
  }
  last->deleteLater ();
}
