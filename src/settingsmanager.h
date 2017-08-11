#pragma once

#include <QSettings>

#include <functional>

class SettingsManager
{
public:
  enum Type
  {
    ConsoleCommand, EditorCommand, CheckUpdates, StartInBackground, ImageCacheSize,
    GroupIds, TabIds, Translation,
    ShowFreeSpace, ShowFilesInfo, ShowSelectionInfo,
    TypeCount
  };

  static void triggerUpdate ();
  static void subscribeForUpdates (QObject *object, const QString &method =
                                     QLatin1String("updateSettings"));

  SettingsManager ();

  QVariant get (Type type) const;
  void set (Type type, const QVariant &value);

  QSettings &qsettings ();

private:
  QSettings settings_;
};
