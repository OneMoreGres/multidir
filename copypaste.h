#pragma once

#include <QList>

class QFileInfo;
class QMimeData;

class CopyPaste
{
public:
  CopyPaste ();

  void copy (const QList<QFileInfo> &sources) const;
  void cut (const QList<QFileInfo> &sources) const;
  void paste (const QFileInfo &target) const;
  bool paste (const QList<QUrl> &urls, const QFileInfo &target, Qt::DropAction action) const;

private:
  bool isCut (const QMimeData &mime) const;
  QString uniqueFilePath (const QString &targetPath, const QString &name) const;
  QStringList names (const QList<QUrl> &urls) const;
  QList<QUrl> urls (const QList<QFileInfo> &infos) const;
  QList<QFileInfo> infos (const QList<QUrl> &urls) const;
};
