#include "utils.h"
#include "debug.h"

#include <QDir>
#include <QObject>
#include <QUrl>

namespace
{
const auto kb = 1024.0;
const auto mb = 1024 * kb;
const auto gb = 1024 * mb;
const auto tb = 1024 * gb;
}

namespace utils
{

QString sizeString (qint64 bytes, int precision)
{
  if (bytes >= tb)
  {
    return QString::number (bytes / tb, 'f', precision) + ' ' + QObject::tr ("Tb");
  }
  if (bytes >= gb)
  {
    return QString::number (bytes / gb, 'f', precision) + ' ' + QObject::tr ("Gb");
  }
  if (bytes >= mb)
  {
    return QString::number (bytes / mb, 'f', precision) + ' ' + QObject::tr ("Mb");
  }
  if (bytes >= kb)
  {
    return QString::number (bytes / kb, 'f', precision) + ' ' + QObject::tr ("Kb");
  }
  return QString::number (bytes) + ' ' + QObject::tr ("bytes");
}

qint64 totalSize (const QFileInfo &info)
{
  qint64 result = 0;
  if (info.isFile ())
  {
    result = info.size ();
  }
  else if (info.isDir ())
  {
    for (const auto &i: dirEntries (info))
    {
      result += totalSize (i);
    }
  }
  return result;
}

Infos dirEntries (const QFileInfo &info)
{
  QDir dir (info.absoluteFilePath ());
  return dir.entryInfoList (QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
}

QString uniqueChars (const QString &source)
{
  QString result;
  for (auto i = 0, end = source.size (); i < end; ++i)
  {
    const auto c = source.at (i);
    if (!result.contains (c))
    {
      result += c;
    }
  }
  return result;
}

QList<QUrl> toUrls (const Infos &infos)
{
  QList<QUrl> urls;
  urls.reserve (infos.size ());
  for (const auto &i: infos)
  {
    urls << QUrl::fromLocalFile (i.absoluteFilePath ());
  }
  return urls;
}

}
