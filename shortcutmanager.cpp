#include "shortcutmanager.h"
#include "debug.h"

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QAction>

namespace
{
using SM = ShortcutManager;
struct Shortcut
{
  QKeySequence key;
  QString name;
  SM::Context context;
};

QVector<QList<QAction *> > actions (SM::ShortcutCount);
QVector<Shortcut> shortcuts (SM::ShortcutCount);
QVector<QString> contexts (SM::ContextCount);
}

void ShortcutManager::setDefaults ()
{
  using QS = QLatin1String;
  contexts[SM::General] = {QObject::tr ("General")};
  contexts[SM::Group] = {QObject::tr ("Group")};
  contexts[SM::Tab] = {QObject::tr ("Tab")};
  contexts[SM::Item] = {QObject::tr ("Item")};


  auto c = SM::General;
  shortcuts[SM::ToggleGui] = {{QS ("Ctrl+Alt+D")}, QObject::tr ("Toggle GUI"), c};
  shortcuts[SM::Find] = {{QS ("Ctrl+F")}, QObject::tr ("Find"), c};

  c = SM::Group;
  shortcuts[SM::AddGroup] = {{QS ("Ctrl+G,A")}, QObject::tr ("Add"), c};
  shortcuts[SM::RenameGroup] = {{QS ("Ctrl+G,R")}, QObject::tr ("Rename.."), c};
  shortcuts[SM::RemoveGroup] = {{QS ("Ctrl+G,D")}, QObject::tr ("Delete..."), c};
  shortcuts[SM::SwitchGroup] = {{QS ("Ctrl+Q")}, QObject::tr ("Switch (plus ID)"), c};

  c = SM::Tab;
  shortcuts[SM::AddTab] = {{QS ("Ctrl+N")}, QObject::tr ("Add tab"), c};
  shortcuts[SM::OpenInExplorer] = {{QS ("Alt+E")}, QObject::tr ("Open in explorer"), c};
  shortcuts[SM::OpenConsole] = {{QS ("Alt+C")}, QObject::tr ("Open in console"), c};
  shortcuts[SM::ChangePath] = {{QS ("Alt+D")}, QObject::tr ("Edit path"), c};
  shortcuts[SM::CopyPath] = {{QS ("Alt+P,C")}, QObject::tr ("Copy path to clipboard"), c};
  shortcuts[SM::LockTab] = {{QS ("Ctrl+L")}, QObject::tr ("Lock"), c};
  shortcuts[SM::CloneTab] = {{QS ("Ctrl+D")}, QObject::tr ("Duplicate"), c};
  shortcuts[SM::CloseTab] = {{QS ("Ctrl+W")}, QObject::tr ("Close"), c};
  shortcuts[SM::ShowDirectories] = {{QS ("Alt+V,D")}, QObject::tr ("Show directories"), c};
  shortcuts[SM::ShowHidden] = {{QS ("Alt+V,H")}, QObject::tr ("Show hidden"), c};
  shortcuts[SM::ExtensiveMode] = {{QS ("Alt+V,E")}, QObject::tr ("Extensive view"), c};
  shortcuts[SM::ListMode] = {{QS ("Alt+V,L")}, QObject::tr ("List mode"), c};
  shortcuts[SM::ShowThumbnails] = {{QS ("Alt+V,I")}, QObject::tr ("Show thumbnails"), c};
  shortcuts[SM::CreateFolder] = {{QS ("F7")}, QObject::tr ("Create folder"), c};
  shortcuts[SM::NextTab] = {{QS ("Tab")}, QObject::tr ("Switch to next"), c};
  shortcuts[SM::SwitchTab] = {{QS ("Ctrl+E")}, QObject::tr ("Switch tab (plus ID)"), c};

  c = SM::Item;
  shortcuts[SM::Copy] = {{QS ("Ctrl+C")}, QObject::tr ("Copy"), c};
  shortcuts[SM::Paste] = {{QS ("Ctrl+V")}, QObject::tr ("Paste"), c};
  shortcuts[SM::Cut] = {{QS ("Ctrl+X")}, QObject::tr ("Cut"), c};
  shortcuts[SM::Rename] = {{QS ("F2")}, QObject::tr ("Rename"), c};
  shortcuts[SM::OpenInTab] = {{QS ("Alt+Return")}, QObject::tr ("Open in tab"), c};
  shortcuts[SM::OpenInEditor] = {{QS ("F4")}, QObject::tr ("Open in editor"), c};
  shortcuts[SM::Trash] = {{QS ("Del")}, QObject::tr ("Move to trash..."), c};
  shortcuts[SM::Remove] = {{QS ("Shift+Del")}, QObject::tr ("Remove..."), c};
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

QString ShortcutManager::contextName (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  const auto context = shortcuts[type].context;
  ASSERT (context >= 0 && context < SM::ContextCount);
  return contexts[context];
}
