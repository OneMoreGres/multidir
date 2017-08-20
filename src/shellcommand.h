#pragma once

#include <QStringList>

class QFileInfo;
class QProcess;

class ShellCommand
{
public:
  explicit ShellCommand (const QString &raw);

  bool run ();
  bool run (QProcess &process) const;

  bool isManaged () const;
  QString command () const;
  QString workDir () const;

  void setWorkDir (const QFileInfo &info);

  void preprocessSelection (const QString &index, const QFileInfo &path,
                            const QFileInfo &current, const QList<QFileInfo> &selection);
  void preprocessFileArguments (const QFileInfo &info, bool forceFilePath = false);

  void setConsoleWrapper (const QString &wrapper);

protected:
  static QStringList parse (const QString &command);

  QString command_;
  QString workDir_;
  bool isManaged_;
};
