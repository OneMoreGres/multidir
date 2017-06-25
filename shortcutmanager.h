#pragma once

#include <QKeySequence>

class QSettings;
class QAction;

class ShortcutManager
{
public:
  enum Shortcut
  {
    ToggleGui, AddDir, Find, Copy, Paste, Cut, Rename,
    ShortcutCount
  };

  static void setDefaults ();

  static void save (QSettings &settings);
  static void restore (QSettings &settings);

  static void add (Shortcut type, QAction *action);
  static void remove (Shortcut type, QAction *action);

  static QKeySequence get (Shortcut type);
  static void set (Shortcut type, QKeySequence key);
  static QString name (Shortcut type);
};
