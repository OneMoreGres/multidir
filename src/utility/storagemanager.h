#pragma once

#include <QStorageInfo>

class StorageManager
{
public:
  static const QStorageInfo * storage (const QFileInfo &path);
};
