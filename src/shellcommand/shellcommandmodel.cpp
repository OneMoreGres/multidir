#include "shellcommandmodel.h"
#include "debug.h"
#include "shellcommandwidget.h"
#include "shellcommand.h"
#include "settingsmanager.h"
#include "styleoptionsproxy.h"

ShellCommandModel::ShellCommandModel (QObject *parent) :
  QAbstractListModel (parent),
  widgets_ (),
  commandWrapper_ (),
  runningColor_ (),
  finishedColor_ (),
  erroredColor_ ()
{
  SettingsManager::subscribeForUpdates (this);
  updateSettings ();

  connect (&StyleOptionsProxy::instance (), &StyleOptionsProxy::changed,
           this, &ShellCommandModel::updateStyle);
  updateStyle ();
}

ShellCommandModel::~ShellCommandModel ()
{
  qDeleteAll (widgets_);
}

bool ShellCommandModel::run (const ShellCommand &command)
{
  auto ok = false;
  if (!command.isManaged ())
  {
    ok = command.run ();
  }
  else
  {
    auto widget = new ShellCommandWidget;
    ok = widget->run (command);
    if (ok)
    {
      add (widget);
      widget->show ();
    }
    else
    {
      widget->deleteLater ();
    }
  }

  return ok;
}

bool ShellCommandModel::run (const QString &command, const QMap<QString, Selection> &selectionPerIndex,
                             const QFileInfo &workingDir)
{
  ShellCommand shell (command);
  for (auto i = selectionPerIndex.cbegin (), end = selectionPerIndex.cend (); i != end; ++i)
  {
    const auto &s = i.value ();
    shell.preprocessSelection (i.key (), s.path, s.current, s.selected);
  }
  shell.setWrapper (commandWrapper_);
  shell.preprocessFileArguments (workingDir);
  shell.setWorkDir (workingDir);
  return run (shell);
}

void ShellCommandModel::openConsole (const QFileInfo &path)
{
  ShellCommand command (openConsoleCommand_);
  command.preprocessFileArguments (path);
  command.setWorkDir (path);
  run (command);
}

void ShellCommandModel::openInEditor (const QFileInfo &path, const QFileInfo &workingDir)
{
  ShellCommand command (editorCommand_);
  command.preprocessFileArguments (path, true);
  if (!workingDir.path ().isEmpty ())
  {
    command.setWorkDir (workingDir);
  }
  else
  {
    command.setWorkDir (path.absolutePath ());
  }
  run (command);
}

void ShellCommandModel::updateSettings ()
{
  SettingsManager settings;
  commandWrapper_ = settings.get (SettingsManager::RunInConsoleCommand)
                    .toString ().trimmed ();
  openConsoleCommand_ = settings.get (SettingsManager::OpenConsoleCommand)
                        .toString ().trimmed ();
  editorCommand_ = settings.get (SettingsManager::EditorCommand)
                   .toString ().trimmed ();
}

int ShellCommandModel::rowCount (const QModelIndex & /*parent*/) const
{
  return widgets_.size ();
}

QVariant ShellCommandModel::data (const QModelIndex &index, int role) const
{
  if (index.row () < widgets_.size ())
  {
    const auto w = toWidget (index);
    if (role == Qt::DisplayRole)
    {
      return w->name ();
    }
    if (role == Qt::BackgroundRole)
    {
      if (w->isRunning ())
      {
        return runningColor_;
      }
      return QColor (w->isFailed () ? erroredColor_ : finishedColor_);
    }
  }
  return {};
}

void ShellCommandModel::add (ShellCommandWidget *widget)
{
  const auto last = rowCount ({});
  beginInsertRows ({}, last, last);
  widgets_ << widget;
  endInsertRows ();

  connect (widget, &QObject::destroyed, this, &ShellCommandModel::remove);
  connect (widget, &ShellCommandWidget::stateChanged, this, &ShellCommandModel::update);

  if (last == 0)
  {
    emit filled ();
  }
}

void ShellCommandModel::show (const QModelIndex &index) const
{
  const auto w = toWidget (index);
  w->hide ();
  w->raise ();
  w->show ();
  w->activateWindow ();
}

void ShellCommandModel::closeAll ()
{
  for (auto *i: widgets_)
  {
    i->close ();
  }
}

QModelIndex ShellCommandModel::toIndex (ShellCommandWidget *widget) const
{
  const auto row = widgets_.indexOf (widget);
  ASSERT (row != -1);
  return index (row, 0);
}

ShellCommandWidget * ShellCommandModel::toWidget (const QModelIndex &index) const
{
  ASSERT (index.row () < widgets_.size ());
  return widgets_[index.row ()];
}

void ShellCommandModel::update (ShellCommandWidget *widget)
{
  const auto i = toIndex (widget);
  emit dataChanged (i,i, {Qt::BackgroundRole});
}

void ShellCommandModel::remove (QObject *widget)
{
  const auto index = widgets_.indexOf (static_cast<ShellCommandWidget *>(widget));
  ASSERT (index != -1);

  beginRemoveRows ({}, index, index);
  widgets_.removeAt (index);
  endRemoveRows ();

  if (widgets_.isEmpty ())
  {
    emit emptied ();
  }
}

void ShellCommandModel::updateStyle ()
{
  const auto &options = StyleOptionsProxy::instance ();
  runningColor_ = options.runningCommandColor ();
  finishedColor_ = options.finishedCommandColor ();
  erroredColor_ = options.erroredCommandColor ();
}

#include "moc_shellcommandmodel.cpp"
