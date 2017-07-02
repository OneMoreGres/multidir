#pragma once

#include <QFileInfo>

namespace utils
{
using Infos = QList<QFileInfo>;

QString sizeString (const QFileInfo &info);

qint64 totalSize (const QFileInfo &info);

Infos dirEntries (const QFileInfo &info);

QString uniqueChars (const QString &source);

QList<QUrl> toUrls (const Infos &infos);

QStringList parseShellCommand (const QString &command);

}
