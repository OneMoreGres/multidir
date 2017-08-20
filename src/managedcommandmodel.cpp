#include "managedcommandmodel.h"
#include "debug.h"
#include "shellcommandwidget.h"

ManagedCommandModel::ManagedCommandModel (QObject *parent) :
  QAbstractListModel (parent),
  widgets_ (),
  runningColor_ (Qt::yellow),
  finishedColor_ (Qt::green),
  erroredColor_ (Qt::red)
{
}

ManagedCommandModel::~ManagedCommandModel ()
{
  qDeleteAll (widgets_);
}

int ManagedCommandModel::rowCount (const QModelIndex & /*parent*/) const
{
  return widgets_.size ();
}

QVariant ManagedCommandModel::data (const QModelIndex &index, int role) const
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

void ManagedCommandModel::add (ShellCommandWidget *widget)
{
  const auto last = rowCount ({});
  beginInsertRows ({}, last, last);
  widgets_ << widget;
  endInsertRows ();

  connect (widget, &QObject::destroyed, this, &ManagedCommandModel::remove);
  connect (widget, &ShellCommandWidget::stateChanged, this, &ManagedCommandModel::update);

  if (last == 0)
  {
    emit filled ();
  }
}

void ManagedCommandModel::show (const QModelIndex &index) const
{
  const auto w = toWidget (index);
  w->hide ();
  w->raise ();
  w->show ();
  w->activateWindow ();
}

QModelIndex ManagedCommandModel::toIndex (ShellCommandWidget *widget) const
{
  const auto row = widgets_.indexOf (widget);
  ASSERT (row != -1);
  return index (row, 0);
}

ShellCommandWidget * ManagedCommandModel::toWidget (const QModelIndex &index) const
{
  ASSERT (index.row () < widgets_.size ());
  return widgets_[index.row ()];
}

void ManagedCommandModel::update (ShellCommandWidget *widget)
{
  const auto i = toIndex (widget);
  emit dataChanged (i,i, {Qt::BackgroundRole});
}

void ManagedCommandModel::remove (QObject *widget)
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

#include "moc_managedcommandmodel.cpp"
