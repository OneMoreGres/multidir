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

QString sizeString (qint64 bytes)
{
  if (bytes >= tb)
  {
    return QString::number (bytes / tb, 'f', 3) + QObject::tr ("Tb");
  }
  if (bytes >= gb)
  {
    return QString::number (bytes / gb, 'f', 3) + QObject::tr ("Gb");
  }
  if (bytes >= mb)
  {
    return QString::number (bytes / mb, 'f', 3) + QObject::tr ("Mb");
  }
  if (bytes >= kb)
  {
    return QString::number (bytes / kb, 'f', 3) + QObject::tr ("Kb");
  }
  return QString::number (bytes) + QObject::tr ("bytes");
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

QStringList parseShellCommand (const QString &command)
{
  QStringList parts;
  QChar separator;
  auto startIndex = -1;
  for (auto i = 0, end = command.size (); i < end; ++i)
  {
    if (separator.isNull ())
    {
      separator = (command[i] == '"' ? '"' : ' ');
      startIndex = i + int (separator == '"');
    }
    else
    {
      if (command[i] == separator)
      {
        parts << command.mid (startIndex, i - startIndex);
        separator = QChar ();
      }
    }
  }
  if (startIndex != command.size ())
  {
    parts << command.mid (startIndex);
  }
  return parts;
}

}
