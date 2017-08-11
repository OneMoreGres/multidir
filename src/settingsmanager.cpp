#include "settingsmanager.h"
#include "debug.h"

#include <QVector>
#include <QWidget>
#include <QPixmapCache>
#include <QPointer>
#include <QApplication>

namespace
{
#ifdef Q_OS_LINUX
const auto defaultConsole = QString ("xterm");
const auto defaultEditor = QString ("gedit");
#endif
#ifdef Q_OS_WIN
const auto defaultConsole = QString ("cmd");
const auto defaultEditor = QString ("notepad.exe");
#endif
#ifdef Q_OS_MAC
const auto defaultConsole = QString ("open -a Terminal");
const auto defaultEditor = QString ("open -a TextEdit");
#endif

struct Entry
{
  QString path;
  QVariant defaultValue;
};
QVector<Entry> entries = [] {
  QVector<Entry> result (SettingsManager::TypeCount);
  using QS = QLatin1String;

#define SET(XXX) result[SettingsManager::XXX]

  SET (ConsoleCommand) = {QS ("console"), defaultConsole};
  SET (EditorCommand) = {QS ("editor"), defaultEditor};
  SET (CheckUpdates) = {QS ("checkUpdates"), false};
  SET (StartInBackground) = {QS ("startBackground"), false};
  SET (ImageCacheSize) = {QS ("imageCacheSize"), 10240};
  SET (GroupIds) = {QS ("groupIds"),
                    QS ("1234567890QWERTYUIOPASDFGHJKLZXCVBNM")};
  SET (TabIds) = {QS ("tabIds"),
                  QS ("1234567890QWERTYUIOPASDFGHJKLZXCVBNM")};

  SET (Translation) = {QS ("language"), {}};

  SET (ShowFreeSpace) = {QS ("statusShowFree"), true};
  SET (ShowFilesInfo) = {QS ("statusShowFiles"), true};
  SET (ShowSelectionInfo) = {QS ("statusShowSelection"), true};
#undef SET

  return result;
} ();


struct Subscriber
{
  QPointer<QObject> object;
  QString method;
};
QVector<Subscriber> subscribers;

}


SettingsManager::SettingsManager () :
  settings_ ()
{
}

QVariant SettingsManager::get (Type type) const
{
  ASSERT (0 <= type && type < TypeCount);
  const auto &entry = entries[type];
  ASSERT (settings_.group ().isEmpty ());
  return settings_.value (entry.path, entry.defaultValue);
}


void SettingsManager::set (Type type, const QVariant &value)
{
  ASSERT (0 <= type && type < TypeCount);
  const auto &entry = entries[type];
  ASSERT (settings_.group ().isEmpty ());
  settings_.setValue (entry.path, value);
}

QSettings &SettingsManager::qsettings ()
{
  return settings_;
}

void SettingsManager::triggerUpdate ()
{
  subscribers.erase (std::remove_if (subscribers.begin (), subscribers.end (),
                                     [](const Subscriber &i) {return !i.object;}), subscribers.end ());

  for (const auto &i:subscribers)
  {
    QMetaObject::invokeMethod (i.object.data (), qPrintable (i.method));
  }
}

void SettingsManager::subscribeForUpdates (QObject *object, const QString &method)
{
  ASSERT (!method.isEmpty ());
  subscribers.append ({object, method});
}

void SettingsManager::setPortable (bool isPortable)
{
  QSettings::setDefaultFormat (isPortable ? QSettings::IniFormat : QSettings::NativeFormat);
  QSettings::setPath (QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath ());
}
