#pragma once

#include <QStringList>

class QFileInfo;
class QProcess;

class ShellCommand
{
public:
  explicit ShellCommand (const QString &raw);

  bool run () const;
  bool run (QProcess &process) const;

  bool isManaged () const;
  QString command () const;
  QString workDir () const;

  void setWorkDir (const QFileInfo &info);

  void preprocessSelection (const QString &index, const QFileInfo &path,
                            const QFileInfo &current, const QList<QFileInfo> &selection);
  void preprocessFileArguments (const QFileInfo &info, bool forceFilePath = false);

  void setWrapper (const QString &wrapper);

protected:
  static QStringList parse (const QString &command);
  static QString toPath (const QFileInfo &info);
  static QString toDir (const QFileInfo &info);
  static QString toArg (const QString &arg);

  QString command_;
  QString workDir_;
  bool isManaged_;
};
