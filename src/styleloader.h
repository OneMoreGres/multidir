#pragma once

#include <QString>

class StyleLoader
{
public:
  static void load ();

  static QStringList available ();
  static QString current ();
  static void setCurrent (const QString &style);

private:
  static QStringList searchPaths ();
  static QString fileName (const QString &name);
  static QString builtin ();
};
