#include "storagemanager.h"
#include "debug.h"

#include <QDateTime>

namespace
{
static QList<QStorageInfo> storages{};
static QDateTime lastCheck{};

void update ()
{
  const auto now = QDateTime::currentDateTime ();
  const auto minDelaySec = 10;
  if (!lastCheck.isValid () || lastCheck.secsTo (now) > minDelaySec)
  {
    storages.clear ();
    for (const auto &i : QStorageInfo::mountedVolumes ())
    {
      if (i.isValid () && i.isReady ())
      {
        storages << i;
      }
    }
    lastCheck = now;
  }
}
}

const QStorageInfo * StorageManager::storage (const QFileInfo &path)
{
  update ();

  const QStorageInfo *result = nullptr;
  auto bestCount = 0;
  const auto absolute = path.absoluteFilePath ();
  for (const auto &i: storages)
  {
    if (!i.isValid () || !absolute.startsWith (i.rootPath ()))
    {
      continue;
    }
    const auto current = i.rootPath ().size ();
    if (current > bestCount)
    {
      bestCount = current;
      result = &i;
    }
  }
  return result;
}
