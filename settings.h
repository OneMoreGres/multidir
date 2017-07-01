#pragma once

#include <QDialog>
#include <QKeySequence>

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QTabWidget;
class QTableWidget;

class Settings : public QDialog
{
Q_OBJECT
public:
  explicit Settings (QWidget *parent = nullptr);

  QString console () const;
  void setConsole (const QString &console);

  bool checkUpdates () const;
  void setCheckUpdates (bool isOn);

  bool startInBackground () const;
  void setStartInBackground (bool isOn);

  int imageCacheSizeKb () const;
  void setImageCacheSize (int sizeKb);

  QString editor () const;
  void setEditor (const QString &editor);

  QString groupShortcuts () const;
  void setGroupShortcuts (const QString &value);

  QString tabShortcuts () const;
  void setTabShortcuts (const QString &value);

private:
  void loadShortcuts ();
  void saveShortcuts ();

  QTabWidget *tabs_;
  QLineEdit *console_;
  QLineEdit *editor_;
  QCheckBox *checkUpdates_;
  QCheckBox *startInBackground_;
  QSpinBox *imageCache_;
  QTableWidget *shortcuts_;
  QLineEdit *groupShortcuts_;
  QLineEdit *tabShortcuts_;
};
