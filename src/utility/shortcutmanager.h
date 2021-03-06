#pragma once

#include <QIcon>
#include <QKeySequence>

class QSettings;
class QAction;
class QWidget;
class QMenu;

class ShortcutManager
{
public:
  enum Shortcut
  {
    ToggleGui, AddTab, Find, Copy, Paste, Cut, Rename, OpenInExplorer, OpenConsole,
    ChangePath, LockTab, CloneTab, CloseTab, OpenInTab, OpenInEditor, CopyPath,
    Trash, Remove, ShowDirectories, ShowHidden, ExtensiveMode, ListMode, ShowThumbnails,
    AddGroup, RenameGroup, RemoveGroup, CreateFolder, SwitchGroup, SwitchTab,
    NextTab, OpenItem, MoveUp, Settings, Quit, Debug, About, CopyTo, MoveTo, LinkTo,
    FixMinSize, RunCommand, ShowProperties, ChangePermissions, View, HistoryForward, HistoryBackward,
    AdjustColumSizes, PreviousTab, EqulalizeTabs, CopyToPath, MoveToPath, LinkToPath,
    Search,
    ShortcutCount
  };

  enum Context
  {
    General, Group, Tab, Item,
    ContextCount
  };

  static void setDefaults ();

  static QAction * create (QWidget *context, Shortcut type,
                           QMenu *menu = nullptr, bool isCheckable = false);
  static QAction * create (QWidget *context, QKeySequence keys,
                           QMenu *menu = nullptr, bool isCheckable = false);

  static void save (QSettings &settings);
  static void restore (QSettings &settings);

  static void add (Shortcut type, QAction *action);
  static void remove (Shortcut type, QAction *action);

  static QKeySequence get (Shortcut type);
  static void set (Shortcut type, QKeySequence key);
  static QString name (Shortcut type);
  static QIcon icon (Shortcut type);
  static bool isGlobal (Shortcut type);

  static QString contextName (Shortcut type);
};
