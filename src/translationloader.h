#pragma once

#include <QString>

class TranslationLoader
{
public:
  static void load ();

  static QStringList availableLanguages ();
  static QString language ();
  static void setLanguage (const QString &language);

private:
  static QStringList availableTranslations ();
  static QString translation ();
  static void setTranslation (const QString &translation);
  static QStringList searchPaths ();

  static QString toTranslation (const QString &language);
  static QString toLanguage (const QString &translation);
};
