#include "copypaste.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QUrl>

#include <QDebug>

namespace
{
const QString kdeCut = QLatin1String ("application/x-kde-cutselection");
const QByteArray kdeCutValue = "1";
const QString gnomeCut = QLatin1String ("x-special/gnome-copied-files");
const QByteArray gnomeCutValue = "cut\n";
const QByteArray gnomeCopyValue = "copy\n";
const QString winCut = QLatin1String ("Preferred DropEffect");
const QByteArray winCutValue = QByteArray::fromHex ("02000000");


bool isCut (const QMimeData &mime)
{
#ifdef Q_OS_LINUX
  return (mime.data (kdeCut) == kdeCutValue ||
          mime.data (gnomeCut).startsWith (gnomeCutValue));
#endif
#ifdef Q_OS_WIN
  return (mime.data (winCut) == winCutValue);
#endif
}

QString uniqueFilePath (const QString &targetPath, const QString &name)
{
  auto targetName = name;
  while (QFile::exists (targetPath + targetName))
  {
    targetName = QObject::tr ("copy_") + targetName;
  }
  return targetPath + targetName;
}

QStringList names (const QList<QUrl> &urls)
{
  QStringList result;
  result.reserve (urls.size ());
  for (const auto &i: urls)
  {
    result << i.toString ();
  }
  return result;
}

QList<QUrl> toUrls (const QList<QFileInfo> &infos)
{
  QList<QUrl> urls;
  urls.reserve (infos.size ());
  for (const auto &i: infos)
  {
    urls << QUrl::fromLocalFile (i.absoluteFilePath ());
  }
  return urls;
}

QList<QFileInfo> toInfos (const QList<QUrl> &urls)
{
  QList<QFileInfo> result;
  result.reserve (urls.size ());
  for (const auto &i: urls)
  {
    if (i.isLocalFile ())
    {
      result << i.toLocalFile ();
    }
  }
  return result;
}

}

void CopyPaste::copy (const QList<QFileInfo> &sources)
{
  auto mime = new QMimeData;
  const auto urls = toUrls (sources);
  mime->setUrls (urls);
#ifdef Q_OS_LINUX
  mime->setData (gnomeCut, gnomeCopyValue + names (urls).join ("\n").toUtf8 ());
#endif
  QApplication::clipboard ()->setMimeData (mime);
}

void CopyPaste::cut (const QList<QFileInfo> &sources)
{
  auto mime = new QMimeData;
  const auto urls = toUrls (sources);
  mime->setUrls (urls);
#ifdef Q_OS_LINUX
  mime->setData (kdeCut, kdeCutValue);
  mime->setData (gnomeCut, gnomeCutValue + names (urls).join ("\n").toUtf8 ());
#endif
#ifdef Q_OS_WIN
  mime->setData (winCut, winCutValue);
#endif
  QApplication::clipboard ()->setMimeData (mime);
}

Qt::DropAction CopyPaste::clipboardAction ()
{
  if (auto mime = QApplication::clipboard ()->mimeData (QClipboard::Clipboard))
  {
    return isCut (*mime) ? Qt::MoveAction : Qt::CopyAction;
  }
  return Qt::CopyAction;
}

QList<QUrl> CopyPaste::clipboardUrls ()
{
  if (auto mime = QApplication::clipboard ()->mimeData (QClipboard::Clipboard))
  {
    return mime->urls ();
  }
  return {};
}

bool CopyPaste::paste (const QList<QUrl> &urls, const QFileInfo &target, Qt::DropAction action)
{
  auto infos = toInfos (urls);
  if (infos.isEmpty ())
  {
    return true;
  }

  auto targetPath = (target.isDir () ? target.absoluteFilePath () : target.absolutePath ())
                    + QDir::separator ();
  bool ok = true;
  switch (action)
  {
    case Qt::CopyAction:
      for (const auto &i: infos)
      {
        ok &= QFile::copy (i.absoluteFilePath (), uniqueFilePath (targetPath, i.fileName ()));
      }
      break;

    case Qt::LinkAction:
      for (const auto &i: infos)
      {
        ok &= QFile::link (i.absoluteFilePath (), targetPath + i.fileName ());
      }
      break;

    case Qt::MoveAction:
      for (const auto &i: infos)
      {
        ok &= QFile::rename (i.absoluteFilePath (), targetPath + i.fileName ());
      }
      break;

    default:
      return false;
  }
  return ok;
}
