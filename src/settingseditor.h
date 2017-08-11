#pragma once

#include <QDialog>
#include <QKeySequence>

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

private:
  void init ();

  void saveState (QSettings &settings) const;
  void restoreState (QSettings &settings);

  void loadShortcuts ();
  void saveShortcuts ();
  void loadLanguage ();
  void saveLanguage ();

  void load ();
  void save ();

  QTabWidget *tabs_;

  QLineEdit *console_;
  QLineEdit *editor_;
  QCheckBox *checkUpdates_;
  QCheckBox *startInBackground_;
  QSpinBox *imageCache_;
  QComboBox *languages_;

  QTableWidget *shortcuts_;
  QLineEdit *groupShortcuts_;
  QLineEdit *tabShortcuts_;

  QCheckBox *showFreeSpace_;
  QCheckBox *showFilesInfo_;
  QCheckBox *showSelectionInfo_;

  QHash<QWidget *, int> editorToSettings_;
};
