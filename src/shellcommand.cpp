#include "shellcommand.h"
#include "notifier.h"
#include "debug.h"

#include <QProcess>
#include <QRegularExpression>
#include <QDir>

namespace
{
const QVector<QChar> quotes {QLatin1Char ('"'), QLatin1Char ('\'')};
}


ShellCommand::ShellCommand (const QString &raw) :
  command_ (raw.trimmed ()),
  workDir_ (),
  isManaged_ (raw.startsWith ('+'))
{
  if (isManaged_)
  {
    command_ = command_.mid (1);
  }
}

bool ShellCommand::run () const
{
  const auto parts = parse (command_);
  LDEBUG () << "Shell" << parts;
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

bool ShellCommand::run (QProcess &process) const
{
  const auto parts = parse (command_);
  LDEBUG () << "Shell" << parts;
  if (parts.isEmpty ())
  {
    Notifier::error (QObject::tr ("Empty command"));
    return false;
  }

  process.setWorkingDirectory (workDir_);
  process.start (parts[0], parts.mid (1));
  return true;
}

QString ShellCommand::command () const
{
  return command_;
}

QString ShellCommand::workDir () const
{
  return workDir_;
}

bool ShellCommand::isManaged () const
{
  return isManaged_;
}

void ShellCommand::setWorkDir (const QFileInfo &info)
{
  workDir_ = toDir (info);
}

void ShellCommand::preprocessSelection (const QString &index, const QFileInfo &path,
                                        const QFileInfo &current, const QList<QFileInfo> &selection)
{
  const auto pathPlaceholder = QString ("%%1%").arg (index);
  if (command_.contains (pathPlaceholder))
  {
    command_.replace (pathPlaceholder, toPath (path));
  }

  const auto itemPlaceholder = QString ("%-%1%").arg (index);
  if (command_.contains (itemPlaceholder))
  {
    command_.replace (itemPlaceholder, toPath (current));
  }

  const auto selectedPlaceholder = QString (R"(%(.)?\*%1%)").arg (index);
  const QRegularExpression selectedRegex (selectedPlaceholder);
  if (command_.contains (selectedRegex))
  {
    QStringList items;
    for (const auto &i: selection)
    {
      items << toPath (i);
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
  command_.replace ("%d", toDir (info));
  if (forceFilePath && !command_.contains ("%p"))
  {
    command_ += " %p";
  }
  command_.replace ("%p", toPath (info));
}

void ShellCommand::setWrapper (const QString &wrapper)
{
  auto command = wrapper;
  if (!command.contains ("%command%"))
  {
    command += " %command%";
  }

  command_ = command.replace ("%command%", command_);
}

QStringList ShellCommand::parse (const QString &command)
{
  QStringList parts;
  auto startIndex = -1;
  auto isQuoted = false;
  for (auto i = 0, end = command.size (); i < end; ++i)
  {
    if (command[i] == QLatin1Char (' '))
    {
      if (isQuoted || (i > 1 && command[i - 1] == '\\'))
      {
        continue;
      }

      parts << toArg (command.mid (startIndex, i - startIndex));
      startIndex = i + 1;
    }
    else if (quotes.contains (command[i]))
    {
      isQuoted = !isQuoted;
    }
  }

  if (startIndex != command.size ())
  {
    parts << toArg (command.mid (startIndex));
  }

  parts.removeAll ({});
  return parts;
}

QString ShellCommand::toPath (const QFileInfo &info)
{
  auto path = QDir::toNativeSeparators (info.absoluteFilePath ());
  return path.replace (' ', "\\ ");
}

QString ShellCommand::toDir (const QFileInfo &info)
{
  return toPath (info.isDir () ? info : info.absolutePath ());
}

QString ShellCommand::toArg (const QString &arg)
{
  if (arg.size () < 2)
  {
    return arg;
  }

  auto result = arg;
  const auto first = *result.begin ();
  const auto last = *(result.end () - 1);
  if (first == last && quotes.contains (first))
  {
    result = result.mid (1, result.size () - 2);
  }

  result.replace ("\\ ", " ");
  return result;
}
