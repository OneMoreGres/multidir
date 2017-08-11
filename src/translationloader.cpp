#include "translationloader.h"
#include "debug.h"
#include "settingsmanager.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

namespace
{
const QString appTranslation = QLatin1String ("multidir");
const auto builtin = QLocale::English;
}

void TranslationLoader::load ()
{
  QLocale locale (translation ());

  QStringList names {QLatin1String ("qt"), QLatin1String ("qtbase"), appTranslation};
  auto last = new QTranslator (QApplication::instance ());
  for (const auto &name: names)
  {
    for (const auto &dir: searchPaths ())
    {
      if (last->load (locale, name, QLatin1String ("_"), dir))
      {
        QApplication::installTranslator (last);
        last = new QTranslator (QApplication::instance ());
      }
    }
  }
  last->deleteLater ();
}

QStringList TranslationLoader::availableLanguages ()
{
  QStringList result {QLocale (builtin).nativeLanguageName ()};
  auto checker = new QTranslator (QApplication::instance ());

  for (const auto &dir: searchPaths ())
  {
    for (const auto &file: QDir (dir).entryInfoList (
           {appTranslation + '*'}, QDir::Files))
    {
      if (checker->load (file.absoluteFilePath ()))
      {
        const auto name = file.baseName ();
        const auto suffixIndex = name.indexOf (QLatin1Char ('_'));
        if (suffixIndex < 0)
        {
          continue;
        }
        const auto suffix = name.mid (suffixIndex + 1);
        const auto locale = QLocale (suffix);
        result.append (locale.nativeLanguageName ());
      }
    }
  }
  return result;
}

QString TranslationLoader::language ()
{
  return toLanguage (translation ());
}

void TranslationLoader::setLanguage (const QString &language)
{
  setTranslation (toTranslation (language));
}

QString TranslationLoader::translation ()
{
  SettingsManager settings;
  auto name = settings.get (SettingsManager::Translation).toString ();
  if (name.isEmpty ())
  {
    const QLocale locale;
    if (locale.language () == QLocale::Language::C)
    {
      name = QLocale (builtin).name ();
    }
    else
    {
      name = locale.name ();
    }
  }
  return name;
}

void TranslationLoader::setTranslation (const QString &translation)
{
  SettingsManager settings;
  settings.set (SettingsManager::Translation, translation);
}

QStringList TranslationLoader::searchPaths ()
{
  return QStringList {
           QLatin1String ("translations"),
           QLibraryInfo::location (QLibraryInfo::TranslationsPath)
#ifdef Q_OS_LINUX
           , QLatin1String ("/usr/share/multidir/translations"),
           qgetenv ("APPDIR") + QLatin1String ("/usr/share/multidir/translations"), // appimage
           qgetenv ("APPDIR") + QLatin1String ("/translations"), // appimage
#endif
  };
}

QString TranslationLoader::toLanguage (const QString &translation)
{
  return QLocale (translation).nativeLanguageName ();
}

QString TranslationLoader::toTranslation (const QString &language)
{
  for (auto i = 0; i < QLocale::Language::LastLanguage; ++i)
  {
    const auto locale = QLocale (QLocale::Language (i));
    if (locale.nativeLanguageName () == language)
    {
      return locale.name ();
    }
  }
  return QLocale ().name ();
}
