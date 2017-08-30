#include "trash.h"
#include "debug.h"

#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QUrl>

#ifdef Q_OS_WIN32

#  include "windows.h"

bool Trash::trash (const QFileInfo &file)
{
  if (!file.exists ())
  {
    return false;
  }
  WCHAR from [MAX_PATH];
  memset (from, 0, sizeof (from));
  int len = file.absoluteFilePath ().toWCharArray (from);
  ASSERT (0 <= len && len < MAX_PATH);
  from [len] = '\0';
  SHFILEOPSTRUCT fileop;
  memset (&fileop, 0, sizeof (fileop));
  fileop.wFunc = FO_DELETE;
  fileop.pFrom = from;
  fileop.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
  return bool(SHFileOperation (&fileop));
}

#endif

#ifdef Q_OS_LINUX

bool Trash::trash (const QFileInfo &file)
{
  if (!file.exists ())
  {
    return false;
  }

  const QDir files (QDir::homePath () + QLatin1String ("/.local/share/Trash/files"));
  auto trashedName = file.fileName ();
  auto index = 0;
  while (files.exists (trashedName))
  {
    const auto diff = (++index ? "." + QString::number (index) : QString ());
    trashedName = file.baseName () + diff + "." + file.completeSuffix ();
  }

  const QDir infos (QDir::homePath () + QLatin1String ("/.local/share/Trash/info"));
  const auto infoName = trashedName + QLatin1String (".trashinfo");
  QFile info (infos.absoluteFilePath (infoName));
  if (!info.open (QFile::WriteOnly))
  {
    return false;
  }
  QTextStream stream (&info);
  stream << "[Trash Info]\n";
  const auto encodedName = QUrl::toPercentEncoding (file.absoluteFilePath (), "~_-./");
  stream << "Path=" << encodedName << '\n';
  const auto date = QDateTime::currentDateTime ().toString (Qt::ISODate);
  stream << "DeletionDate=" << date << '\n';
  info.close ();


  if (!QFile::rename (file.absoluteFilePath (), files.absoluteFilePath (trashedName)))
  {
    QFile::remove (infos.absoluteFilePath (infoName));
    return false;
  }
  return true;
}

#endif


#ifdef Q_OS_MAC

#  include <Carbon/Carbon.h>

bool Trash::trash (const QFileInfo &file)
{
  if (!file.exists ())
  {
    return false;
  }

  const auto source = file.absoluteFilePath ().toUtf8 ();
  char *newPath = 0;
  auto result = FSPathMoveObjectToTrashSync (source.constData (), &newPath,
                                             kFSFileOperationDoNotMoveAcrossVolumes);

  return result == noErr;
}

#endif
