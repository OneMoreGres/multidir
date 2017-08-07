#include "shortcutmanager.h"
#include "globalaction.h"
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
  Shortcut (const QKeySequence &key = {}, const QString &name = {}, const QIcon &icon = {},
            SM::Context context = {}, bool isGlobal = false);

  QKeySequence key;
  QString name;
  QIcon icon;
  SM::Context context;
  bool isGlobal;
};

Shortcut::Shortcut (const QKeySequence &key, const QString &name, const QIcon &icon,
                    ShortcutManager::Context context, bool isGlobal) :
  key (key),
  name (name),
  icon (icon),
  context (context),
  isGlobal (isGlobal)
{
}


QVector<QList<QAction *> > actions (SM::ShortcutCount);
QVector<Shortcut> shortcuts (SM::ShortcutCount);
QVector<QString> contexts (SM::ContextCount);
}

void ShortcutManager::setDefaults ()
{
  GlobalAction::init ();

  using QS = QLatin1String;
  contexts[SM::General] = {QObject::tr ("General")};
  contexts[SM::Group] = {QObject::tr ("Group")};
  contexts[SM::Tab] = {QObject::tr ("Tab")};
  contexts[SM::Item] = {QObject::tr ("Item")};


  auto c = SM::General;
  shortcuts[SM::ToggleGui] = {{QS ("Ctrl+Alt+D")}, QObject::tr ("Toggle"),
                              QIcon (":/popup.png"), c, true};
  shortcuts[SM::Find] = {{QS ("Ctrl+F")}, QObject::tr ("Filter"),
                         QIcon (":/find.png"), c};
  shortcuts[SM::Settings] = {{}, QObject::tr ("Settings"),
                             QIcon (":/settings.png"), c};
  shortcuts[SM::Quit] = {{}, QObject::tr ("Quit"),
                         QIcon (":/quit.png"), c};
  shortcuts[SM::Debug] = {{}, QObject::tr ("Debug mode"),
                          QIcon (":/debug.png"), c};
  shortcuts[SM::About] = {{}, QObject::tr ("About"),
                          QIcon (":/about.png"), c};

  c = SM::Group;
  shortcuts[SM::AddGroup] = {{QS ("Ctrl+G,A")}, QObject::tr ("Add"),
                             QIcon (":/add.png"), c};
  shortcuts[SM::RenameGroup] = {{QS ("Ctrl+G,R")}, QObject::tr ("Rename..."),
                                QIcon (":/rename.png"), c};
  shortcuts[SM::RemoveGroup] = {{QS ("Ctrl+G,D")}, QObject::tr ("Delete..."),
                                QIcon (":/minus.png"), c};
  shortcuts[SM::SwitchGroup] = {{QS ("Ctrl+Q")}, QObject::tr ("Switch (plus ID)"),
                                {}, c};

  c = SM::Tab;
  shortcuts[SM::AddTab] = {{QS ("Ctrl+N")}, QObject::tr ("Add tab"),
                           QIcon (":/add.png"), c};
  shortcuts[SM::OpenInExplorer] = {{QS ("Alt+E")}, QObject::tr ("Open in explorer"),
                                   QIcon (":/openExternal.png"), c};
  shortcuts[SM::OpenConsole] = {{QS ("Alt+C")}, QObject::tr ("Open in console"),
                                QIcon (":/console.png"), c};
  shortcuts[SM::ChangePath] = {{QS ("Alt+D")}, QObject::tr ("Edit path"),
                               QIcon (":/rename.png"), c};
  shortcuts[SM::LockTab] = {{QS ("Ctrl+L")}, QObject::tr ("Lock"),
                            QIcon (":/lockTab.png"), c};
  shortcuts[SM::CloneTab] = {{QS ("Ctrl+D")}, QObject::tr ("Duplicate..."),
                             QIcon (":/cloneTab.png"), c};
  shortcuts[SM::CloseTab] = {{QS ("Ctrl+W")}, QObject::tr ("Close..."),
                             QIcon (":/closeTab.png"), c};
  shortcuts[SM::ShowDirectories] = {{QS ("Alt+V,D")}, QObject::tr ("Show directories"),
                                    QIcon (":/folder.png"), c};
  shortcuts[SM::ShowHidden] = {{QS ("Alt+V,H")}, QObject::tr ("Show hidden"),
                               QIcon (":/hidden.png"), c};
  shortcuts[SM::ExtensiveMode] = {{QS ("Alt+V,E")}, QObject::tr ("Extensive view"),
                                  QIcon (":/extensive.png"), c};
  shortcuts[SM::ListMode] = {{QS ("Alt+V,L")}, QObject::tr ("List mode"),
                             QIcon (":/listMode.png"), c};
  shortcuts[SM::ShowThumbnails] = {{QS ("Alt+V,I")}, QObject::tr ("Show thumbnails"),
                                   QIcon (":/showThumbs.png"), c};
  shortcuts[SM::FixMinSize] = {{QS ("Alt+V,F")}, QObject::tr ("Fix min size"),
                               QIcon (":/fixSize.png"), c};
  shortcuts[SM::CreateFolder] = {{QS ("F7")}, QObject::tr ("Create folder"),
                                 QIcon (":/newFolder.png"), c};
  shortcuts[SM::NextTab] = {{QS ("Tab")}, QObject::tr ("Switch to next"),
                            {}, c};
  shortcuts[SM::SwitchTab] = {{QS ("Ctrl+E")}, QObject::tr ("Switch tab (plus ID)"),
                              {}, c};
  shortcuts[SM::MoveUp] = {{}, QObject::tr ("Move up"),
                           QIcon (":/up.png"), c};
  shortcuts[SM::RunCommand] = {{QS ("Ctrl+R")}, QObject::tr ("Run command here..."),
                               QIcon (":/command.png"), c};

  c = SM::Item;
  shortcuts[SM::OpenItem] = {{}, QObject::tr ("Open"),
                             QIcon (":/open.png"), c};
  shortcuts[SM::Copy] = {{QS ("Ctrl+C")}, QObject::tr ("Copy"),
                         QIcon::fromTheme ("copy"), c};
  shortcuts[SM::Paste] = {{QS ("Ctrl+V")}, QObject::tr ("Paste"),
                          QIcon::fromTheme ("paste"), c};
  shortcuts[SM::Cut] = {{QS ("Ctrl+X")}, QObject::tr ("Cut"),
                        QIcon::fromTheme ("cut"), c};
  shortcuts[SM::Rename] = {{QS ("F2")}, QObject::tr ("Rename"),
                           QIcon (":/rename.png"), c};
  shortcuts[SM::OpenInTab] = {{QS ("Ctrl+Return")}, QObject::tr ("Open in tab"),
                              {}, c};
  shortcuts[SM::OpenInEditor] = {{QS ("F4")}, QObject::tr ("Open in editor"),
                                 QIcon (":/editor.png"), c};
  shortcuts[SM::Trash] = {{QS ("Del")}, QObject::tr ("Move to trash..."),
                          QIcon (":/trash.png"), c};
  shortcuts[SM::Remove] = {{QS ("F8")}, QObject::tr ("Remove..."),
                           QIcon (":/remove.png"), c};
  shortcuts[SM::CopyPath] = {{QS ("Alt+P,C")}, QObject::tr ("Copy path"),
                             QIcon (":/path.png"), c};
  shortcuts[SM::CopyTo] = {{QS ("F5")}, QObject::tr ("Copy to (plus ID)"),
                           QIcon (":/copyTo.png"), c};
  shortcuts[SM::MoveTo] = {{QS ("F6")}, QObject::tr ("Move to (plus ID)"),
                           QIcon (":/moveTo.png"), c};
  shortcuts[SM::LinkTo] = {{}, QObject::tr ("Link to (plus ID)"),
                           QIcon (":/linkTo.png"), c};
  shortcuts[SM::ShowProperties] = {{QS ("Alt+Return")}, QObject::tr ("Properties"),
                                   QIcon (":/properties.png"), c};
  shortcuts[SM::ChangePermissions] = {{QS ("Ctrl+M")}, QObject::tr ("Change permissions"),
                                      QIcon (":/permissions.png"), c};
  shortcuts[SM::View] = {{QS ("F3")}, QObject::tr ("View"),
                         QIcon (":/read.png"), c};
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
  auto &shortcut = shortcuts[type];
  action->setText (shortcut.name);
  action->setIcon (shortcut.icon);
  if (shortcut.isGlobal)
  {
    GlobalAction::makeGlobal (action);
  }
  QObject::connect (action, &QObject::destroyed,
                    [type, action] {ShortcutManager::remove (type, action);});
}

void ShortcutManager::remove (Shortcut type, QAction *action)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  actions[type].removeOne (action);
  if (shortcuts[type].isGlobal)
  {
    GlobalAction::removeGlobal (action);
  }
}

QKeySequence ShortcutManager::get (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].key;
}

void ShortcutManager::set (ShortcutManager::Shortcut type, QKeySequence key)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  auto &shortcut = shortcuts[type];
  shortcut.key = key;
  for (auto *i: actions[type])
  {
    if (shortcut.isGlobal)
    {
      GlobalAction::removeGlobal (i);
    }
    i->setShortcut (key);
    if (shortcut.isGlobal)
    {
      GlobalAction::makeGlobal (i);
    }
  }
}

QString ShortcutManager::name (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].name;
}

QIcon ShortcutManager::icon (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].icon;
}

bool ShortcutManager::isGlobal (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  return shortcuts[type].isGlobal;
}

QString ShortcutManager::contextName (ShortcutManager::Shortcut type)
{
  ASSERT (type >= 0 && type < SM::ShortcutCount);
  const auto context = shortcuts[type].context;
  ASSERT (context >= 0 && context < SM::ContextCount);
  return contexts[context];
}
