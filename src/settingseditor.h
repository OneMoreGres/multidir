#pragma once

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QTabWidget;
class QTableWidget;
class QComboBox;
class QSettings;

class SettingsEditor : public QDialog
{
Q_OBJECT
public:
  explicit SettingsEditor (QWidget *parent = nullptr);
  ~SettingsEditor ();

  static void initOrphanSettings ();

private:
  void init ();

  void saveState (QSettings &settings) const;
  void restoreState (QSettings &settings);

  void updateOrphanSettings ();

  void loadShortcuts ();
  void saveShortcuts ();
  void loadLanguage ();
  void saveLanguage ();
  void loadStyle ();
  void saveStyle ();

  void load ();
  void save ();

  QTabWidget *tabs_;

  QLineEdit *openConsole_;
  QLineEdit *runInConsole_;
  QLineEdit *editor_;
  QCheckBox *checkUpdates_;
  QCheckBox *startInBackground_;
  QSpinBox *imageCache_;
  QComboBox *languages_;

  QTableWidget *shortcuts_;
  QLineEdit *groupShortcuts_;
  QLineEdit *tabShortcuts_;

  QComboBox *styles_;
  QCheckBox *showFreeSpace_;
  QCheckBox *showFilesInfo_;
  QCheckBox *showSelectionInfo_;

  QHash<QWidget *, int> editorToSettings_;
};
