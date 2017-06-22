#include "utils.h"

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

}
