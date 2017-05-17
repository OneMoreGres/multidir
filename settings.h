#pragma once

#include <QDialog>
#include <QKeySequence>

class QKeySequenceEdit;
class QLineEdit;
class QCheckBox;

class Settings : public QDialog
{
Q_OBJECT
public:
  explicit Settings (QWidget *parent = nullptr);

  QKeySequence hotkey () const;
  void setHotkey (const QKeySequence &hotkey);

  QString console () const;
  void setConsole (const QString &console);

  bool checkUpdates () const;
  void setCheckUpdates (bool isOn);

private:
  QKeySequenceEdit *hotkey_;
  QLineEdit *console_;
  QCheckBox *checkUpdates_;
};
