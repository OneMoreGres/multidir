#pragma once

#include <QFile>

class FilePermissions
{
public:
  static QString toNumericString (QFile::Permissions permissions);
  static QString toString (QFile::Permissions permissions);
  static QString toFullString (QFile::Permissions permissions);

  static QFile::Permissions fromNumericString (const QString &text);
  static QFile::Permissions fromString (const QString &text);
};
