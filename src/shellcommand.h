#pragma once

#include <QStringList>

class DirWidget;

class QFileInfo;

class ShellCommand
{
public:
  ShellCommand (const QString &raw);

  bool run ();

  void setWorkDir (const QFileInfo &info);

  void preprocessSelections (const DirWidget &widget);
  void preprocessFileArguments (const QFileInfo &info, bool forceFilePath = false);

private:
  void preprocessWidgetSelection (const DirWidget &w, const QString &index);
  QStringList parse (const QString &command) const;

  QString command_;
  QString workDir_;
};
