#include "shellcommandmodel.h"
#include "debug.h"
#include "shellcommandwidget.h"
#include "shellcommand.h"
#include "settingsmanager.h"

ShellCommandModel::ShellCommandModel (QObject *parent) :
  QAbstractListModel (parent),
  widgets_ (),
  commandWrapper_ (),
  runningColor_ (Qt::yellow),
  finishedColor_ (Qt::green),
  erroredColor_ (Qt::red)
{
  SettingsManager::subscribeForUpdates (this);
  updateSettings ();
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

bool ShellCommandModel::run (const ShellCommand &command, const QFileInfo &workingDir)
{
  auto local = command;
  local.setWrapper (commandWrapper_);
  local.preprocessFileArguments (workingDir);
  local.setWorkDir (workingDir);
  return run (local);
}

void ShellCommandModel::openConsole (const QFileInfo &path)
{
  ShellCommand command (openConsoleCommand_);
  command.preprocessFileArguments (path);
  run (command, path);
}

void ShellCommandModel::openInEditor (const QFileInfo &path, const QFileInfo &workingDir)
{
  ShellCommand command (editorCommand_);
  command.preprocessFileArguments (path, true);
  run (command, workingDir);
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

#include "moc_shellcommandmodel.cpp"
