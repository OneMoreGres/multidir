#pragma once

#include <QList>

class QFileInfo;
class QUrl;

class CopyPaste
{
public:
  static void copy (const QList<QFileInfo> &sources);
  static void cut (const QList<QFileInfo> &sources);
  static Qt::DropAction clipboardAction ();
  static QList<QUrl> clipboardUrls ();
  static bool paste (const QList<QUrl> &urls, const QFileInfo &target, Qt::DropAction action);
};
