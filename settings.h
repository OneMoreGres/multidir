#pragma once

#include <QDialog>
#include <QKeySequence>

class QKeySequenceEdit;

class Settings : public QDialog
{
Q_OBJECT
public:
  explicit Settings (QWidget *parent = nullptr);

  QKeySequence hotkey () const;
  void setHotkey (const QKeySequence &hotkey);

private:
  QKeySequenceEdit *hotkey_;
};
