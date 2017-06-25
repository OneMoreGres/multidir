#include "shortcutmanager.h"
#include "debug.h"

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QAction>

namespace
{
struct Shortcut
{
  QKeySequence key;
  QString name;
};

using SM = ShortcutManager;
QVector<QList<QAction *> > actions (SM::ShortcutCount);
QVector<Shortcut> shortcuts (SM::ShortcutCount);
}

void ShortcutManager::setDefaults ()
{
  using QS = QLatin1String;
  shortcuts[SM::ToggleGui] = {{QS ("Ctrl+Alt+D")}, QObject::tr ("Toggle GUI")};
  shortcuts[SM::ToggleGui] = {{QS ("Ctrl+Alt+D")}, QObject::tr ("Toggle GUI")};
  shortcuts[SM::AddDir] = {{QS ("Ctrl+N")}, QObject::tr ("Add widget to group")};
  shortcuts[SM::Find] = {{QS ("Ctrl+F")}, QObject::tr ("Find")};
  shortcuts[SM::Copy] = {{QS ("Ctrl+C")}, QObject::tr ("Copy")};
  shortcuts[SM::Paste] = {{QS ("Ctrl+V")}, QObject::tr ("Paste")};
  shortcuts[SM::Cut] = {{QS ("Ctrl+X")}, QObject::tr ("Cut")};
  shortcuts[SM::Rename] = {{QS ("F2")}, QObject::tr ("Rename item")};
}

void ShortcutManager::save (QSettings &settings)
{
  for (auto i = 0; i < ShortcutCount; ++i)
  {
    settings.setValue (QString::number (i), get (Shortcut (i)));
  }
}

void ShortcutManager::restore (QSettings &settings)
{
  for (auto i = 0; i < ShortcutCount; ++i)
  {
    if (settings.contains (QString::number (i)))
    {
      set (Shortcut (i), settings.value (QString::number (i)).toString ());
    }
  }
}

void ShortcutManager::add (Shortcut type, QAction *action)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  actions[type] << action;
  action->setShortcut (get (type));
  QObject::connect (action, &QObject::destroyed,
                    [type, action] {ShortcutManager::remove (type, action);});
}

void ShortcutManager::remove (Shortcut type, QAction *action)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  actions[type].removeOne (action);
}

QKeySequence ShortcutManager::get (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].key;
}

void ShortcutManager::set (ShortcutManager::Shortcut type, QKeySequence key)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  shortcuts[type].key = key;
  for (auto *i: actions[type])
  {
    i->setShortcut (key);
  }
}

QString ShortcutManager::name (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].name;
}
