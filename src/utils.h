#pragma once

#include <QFileInfo>

namespace utils
{
using Infos = QList<QFileInfo>;

QString sizeString (qint64 bytes, int precision = 0);

qint64 totalSize (const QFileInfo &info);

Infos dirEntries (const QFileInfo &info);

QString uniqueChars (const QString &source);

QList<QUrl> toUrls (const Infos &infos);

QString fileNames (const Infos &infos);

}
