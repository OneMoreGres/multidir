#include "shellcommand.h"
#include "notifier.h"
#include "debug.h"

#include <QProcess>
#include <QRegularExpression>
#include <QDir>


ShellCommand::ShellCommand (const QString &raw) :
  command_ (raw.trimmed ()),
  workDir_ ()
{

}

bool ShellCommand::run ()
{
  const auto parts = parse (command_);
  if (!parts.isEmpty () && QProcess::startDetached (parts[0], parts.mid (1), workDir_))
  {
    return true;
  }

  if (parts.isEmpty ())
  {
    Notifier::error (QObject::tr ("Empty command"));
  }
  else
  {
    Notifier::error (QObject::tr ("Failed to run command '%1' in '%2'")
                     .arg (command_, workDir_));
  }
  return false;
}

void ShellCommand::setWorkDir (const QFileInfo &info)
{
  workDir_ = QDir::toNativeSeparators (info.isDir () ? info.absoluteFilePath ()
                                                     : info.absolutePath ());
}

void ShellCommand::preprocessSelection (const QString &index, const QFileInfo &path,
                                        const QFileInfo &current, const QList<QFileInfo> &selection)
{
  const QString pathPlaceholder ("%%1%");
  command_.replace (pathPlaceholder.arg (index),
                    QDir::toNativeSeparators (path.absoluteFilePath ()));

  const QString itemPlaceholder ("%-%1%");
  command_.replace (itemPlaceholder.arg (index),
                    QDir::toNativeSeparators (current.absoluteFilePath ()));

  const QString selectedPlaceholder (R"(%(.)?\*%1%)");
  const QRegularExpression selectedRegex (selectedPlaceholder.arg (index));
  if (command_.contains (selectedRegex))
  {
    QStringList items;
    for (const auto &i: selection)
    {
      items << QDir::toNativeSeparators (i.absoluteFilePath ());
    }

    auto match = selectedRegex.match (command_);
    while (match.hasMatch ())
    {
      const auto separator = (!match.captured (1).isEmpty ()
                              ? match.captured (1) : " ");
      command_.replace (match.capturedStart (0), match.capturedLength (0),
                        items.join (separator));
      match = selectedRegex.match (command_);
    }
  }
}

void ShellCommand::preprocessFileArguments (const QFileInfo &info, bool forceFilePath)
{
  command_.replace ("%d", QDir::toNativeSeparators (info.isDir () ? info.absoluteFilePath ()
                                                                  : info.absolutePath ()));
  if (forceFilePath && !command_.contains ("%p"))
  {
    command_ += " %p";
  }
  command_.replace ("%p", QDir::toNativeSeparators (info.absoluteFilePath ()));
}

void ShellCommand::setConsoleWrapper (const QString &wrapper)
{
  if (!command_.startsWith ('+'))
  {
    return;
  }

  auto command = wrapper;
  if (!command.contains ("%command%"))
  {
    command += " %command%";
  }

  command_ = command.replace ("%command%", command_.mid (1));
}

QStringList ShellCommand::parse (const QString &command)
{
  QStringList parts;
  QChar separator;
  auto startIndex = -1;
  for (auto i = 0, end = command.size (); i < end; ++i)
  {
    if (separator.isNull ())
    {
      separator = (command[i] == '"' ? '"' : ' ');
      separator = (command[i] == '\'' ? '\'' : separator);
      startIndex = i + int (separator != ' ');
    }
    else
    {
      if (command[i] == separator)
      {
        parts << command.mid (startIndex, i - startIndex);
        startIndex = i + int (separator != ' ');
        separator = QChar ();
      }
    }
  }
  if (startIndex != command.size ())
  {
    parts << command.mid (startIndex);
  }
  return parts;
}
