#include "utils.h"

#include <QDir>
#include <QObject>

namespace
{
const auto kb = 1024.0;
const auto mb = 1024 * kb;
const auto gb = 1024 * mb;
const auto tb = 1024 * gb;
}

namespace utils
{

QString sizeString (const QFileInfo &info)
{
  const auto bytes = info.size ();
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

}
