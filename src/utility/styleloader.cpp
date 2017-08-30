#include "styleloader.h"
#include "debug.h"
#include "settingsmanager.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

void StyleLoader::load ()
{
  const auto saved = current ();
  setCurrent (saved);
}

QStringList StyleLoader::available ()
{
  QStringList result {builtin ()};
  for (const auto &dir: searchPaths ())
  {
    for (const auto &file: QDir (dir).entryInfoList ({QLatin1String ("*.css")}, QDir::Files))
    {
      const auto name = file.baseName ();
      if (!result.contains (name))
      {
        result << name;
      }
    }
  }
  return result;
}

QString StyleLoader::current ()
{
  SettingsManager settings;
  auto saved = settings.get (SettingsManager::Style).toString ();
  if (saved.isEmpty () || fileName (saved).isEmpty ())
  {
    saved = builtin ();
  }
  return saved;
}

QString StyleLoader::fileName (const QString &name)
{
  for (const auto &dir: searchPaths ())
  {
    QFileInfo info (dir + QLatin1Char ('/') + name + QLatin1String (".css"));
    if (info.exists ())
    {
      return info.absoluteFilePath ();
    }
  }
  return {};
}

void StyleLoader::setCurrent (const QString &style)
{
  SettingsManager settings;
  if (style == builtin ())
  {
    settings.set (SettingsManager::Style, QLatin1String (""));
    qApp->setStyleSheet ({});
    return;
  }

  settings.set (SettingsManager::Style, style);

  const auto file = fileName (style);
  QFile f (file);
  if (!file.isEmpty () && f.open (QFile::ReadOnly))
  {
    qApp->setStyleSheet (QString::fromUtf8 (f.readAll ()));
  }
}

QStringList StyleLoader::searchPaths ()
{
  return QStringList {
           QLatin1String ("styles")

#ifdef Q_OS_OSX
           , QLibraryInfo::location (QLibraryInfo::DataPath)
           + QLatin1String ("/Resources/styles"),
#endif
#ifdef Q_OS_LINUX
           , QLatin1String ("/usr/share/multidir/styles"),
           qgetenv ("APPDIR") + QLatin1String ("/usr/share/multidir/styles"), // appimage
           qgetenv ("APPDIR") + QLatin1String ("/styles"), // appimage
#endif
  };
}

QString StyleLoader::builtin ()
{
  return QObject::tr ("Default");
}
