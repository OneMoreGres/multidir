#pragma once

#include <QDialog>
#include <QKeySequence>

class QKeySequenceEdit;
class QLineEdit;

class Settings : public QDialog
{
Q_OBJECT
public:
  explicit Settings (QWidget *parent = nullptr);

  QKeySequence hotkey () const;
  void setHotkey (const QKeySequence &hotkey);

  QString console () const;
  void setConsole (const QString &console);

private:
  QKeySequenceEdit *hotkey_;
  QLineEdit *console_;
};
