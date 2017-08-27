#include "copypaste.h"
#include "utils.h"
#include "debug.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFileInfo>
#include <QUrl>

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
#ifdef Q_OS_OSX
  return false;
#endif
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

}

void CopyPaste::copy (const QList<QFileInfo> &sources)
{
  auto mime = new QMimeData;
  const auto urls = utils::toUrls (sources);
  mime->setUrls (urls);
#ifdef Q_OS_LINUX
  mime->setData (gnomeCut, gnomeCopyValue + names (urls).join ("\n").toUtf8 ());
#endif
  QApplication::clipboard ()->setMimeData (mime);
}

void CopyPaste::cut (const QList<QFileInfo> &sources)
{
  auto mime = new QMimeData;
  const auto urls = utils::toUrls (sources);
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
