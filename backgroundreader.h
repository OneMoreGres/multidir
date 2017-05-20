#pragma once

#include <QObject>

class BackgroundReader : public QObject
{
Q_OBJECT
public:
  explicit BackgroundReader (QObject *parent = 0);

signals:
  void iconRead (const QString &fileName, const QPixmap &pixmap);

public slots:
  void readIcon (const QString &fileName);
};
