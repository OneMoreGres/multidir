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
  shortcuts[SM::AddTab] = {{QS ("Ctrl+N")}, QObject::tr ("Add tab to group")};
  shortcuts[SM::Find] = {{QS ("Ctrl+F")}, QObject::tr ("Find")};
  shortcuts[SM::Copy] = {{QS ("Ctrl+C")}, QObject::tr ("Copy")};
  shortcuts[SM::Paste] = {{QS ("Ctrl+V")}, QObject::tr ("Paste")};
  shortcuts[SM::Cut] = {{QS ("Ctrl+X")}, QObject::tr ("Cut")};
  shortcuts[SM::Rename] = {{QS ("F2")}, QObject::tr ("Rename item")};
  shortcuts[SM::OpenInExplorer] = {{QS ("Alt+E")}, QObject::tr ("Open tab in explorer")};
  shortcuts[SM::OpenConsole] = {{QS ("Alt+C")}, QObject::tr ("Open tab in console")};
  shortcuts[SM::ChangePath] = {{QS ("Alt+P,E")}, QObject::tr ("Edit tab's path")};
  shortcuts[SM::CopyPath] = {{QS ("Alt+P,C")}, QObject::tr ("Copy tab's path to clipboard")};
  shortcuts[SM::LockTab] = {{QS ("Ctrl+L")}, QObject::tr ("Lock tab")};
  shortcuts[SM::CloneTab] = {{QS ("Ctrl+D")}, QObject::tr ("Duplicate tab")};
  shortcuts[SM::CloseTab] = {{QS ("Ctrl+W")}, QObject::tr ("Close tab")};
  shortcuts[SM::OpenInTab] = {{QS ("Alt+Return")}, QObject::tr ("Open item in new tab")};
  shortcuts[SM::OpenInEditor] = {{QS ("Ctrl+Return")}, QObject::tr ("Open item in editor")};
  shortcuts[SM::Trash] = {{QS ("Del")}, QObject::tr ("Move item to trash")};
  shortcuts[SM::Remove] = {{QS ("Shift+Del")}, QObject::tr ("Remove item")};
  shortcuts[SM::ShowDirectories] = {{QS ("Alt+V,D")}, QObject::tr ("Toggle show directories")};
  shortcuts[SM::ShowHidden] = {{QS ("Alt+V,H")}, QObject::tr ("Toggle show hidden")};
  shortcuts[SM::ExtensiveMode] = {{QS ("Alt+V,E")}, QObject::tr ("Toggle extensive view")};
  shortcuts[SM::ListMode] = {{QS ("Alt+V,L")}, QObject::tr ("Toggle list mode")};
  shortcuts[SM::ShowThumbnails] = {{QS ("Alt+V,I")}, QObject::tr ("Toggle show thumbnails")};
  shortcuts[SM::AddGroup] = {{QS ("Ctrl+G,A")}, QObject::tr ("Add new tabs group")};
  shortcuts[SM::RenameGroup] = {{QS ("Ctrl+G,R")}, QObject::tr ("Rename tabs group")};
  shortcuts[SM::RemoveGroup] = {{QS ("Ctrl+G,D")}, QObject::tr ("Delete tabs group")};
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
