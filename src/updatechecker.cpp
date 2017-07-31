#include "updatechecker.h"
#include "constants.h"
#include "debug.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

namespace
{

class Semver
{
public:
  explicit Semver (const QByteArray &version)
    : major_ (0), minor_ (0), patch_ (0)
  {
    auto parts = version.split ('.');
    if (parts.size () < 3)
    {
      return;
    }
    major_ = parts[0].toInt ();
    minor_ = parts[1].toInt ();
    patch_ = parts[2].toInt ();
  }

  bool operator< (const Semver &r) const
  {
    return std::tie (major_, minor_, patch_) < std::tie (r.major_, r.minor_, r.patch_);
  }

private:
  int major_, minor_, patch_;
};

}

UpdateChecker::UpdateChecker (QObject *parent) :
  QObject (parent),
  manager_ (new QNetworkAccessManager (this))
{
  connect (manager_, &QNetworkAccessManager::finished,
           this, &UpdateChecker::parse);
}

void UpdateChecker::check ()
{
  manager_->get (QNetworkRequest (QUrl (constants::updateUrl)));
}

void UpdateChecker::parse (QNetworkReply *reply)
{
  if (reply->error () != QNetworkReply::NoError)
  {
    LERROR () << "Available version check failed" << LARG (reply->errorString ());
    return;
  }
  const auto data = reply->readAll ().trimmed ();
  const auto available = Semver (data);
  const auto current = Semver (constants::version);
  if (current < available)
  {
    emit updateAvailable (data);
  }
  else
  {
    emit noUpdates ();
  }
}
