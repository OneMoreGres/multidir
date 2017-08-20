#pragma once

#include <QWidget>
#include <QProcess>

class ShellCommand;

class QTextEdit;

class ShellCommandWidget : public QWidget
{
Q_OBJECT
public:
  explicit ShellCommandWidget (QWidget *parent = nullptr);
  ~ShellCommandWidget ();

  bool run (const ShellCommand &command);

  QString name () const;
  bool isRunning () const;
  bool isFailed () const;

signals:
  void stateChanged (ShellCommandWidget *widget);

protected:
  void closeEvent (QCloseEvent *event) override;

private:
  void handleStart ();
  void handleFinish (int exitCode, QProcess::ExitStatus exitStatus);
  void readError ();
  void readOut ();

  void processUserInput ();
  void processUserInputWithEot ();
  void terminate ();

  QProcess *process_;
  QTextEdit *log_;
  QTextEdit *userInput_;

  QColor errorColor_;
};
