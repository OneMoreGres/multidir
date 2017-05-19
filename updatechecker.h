#pragma once

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateChecker : public QObject
{
Q_OBJECT
public:
  explicit UpdateChecker (QObject *parent = nullptr);

  void check ();

signals:
  void updateAvailable (const QString &version);
  void noUpdates ();

private:
  void parse (QNetworkReply *reply);

  QNetworkAccessManager *manager_;
};
