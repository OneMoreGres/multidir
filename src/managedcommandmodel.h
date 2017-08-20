#pragma once

#include <QAbstractListModel>
#include <QColor>

class ShellCommandWidget;

class ManagedCommandModel : public QAbstractListModel
{
Q_OBJECT
public:
  explicit ManagedCommandModel (QObject *parent = nullptr);
  ~ManagedCommandModel ();

  int rowCount (const QModelIndex &parent) const override;
  QVariant data (const QModelIndex &index, int role) const override;

  void add (ShellCommandWidget *widget);
  void show (const QModelIndex &index) const;

signals:
  void filled ();
  void emptied ();

private:
  QModelIndex toIndex (ShellCommandWidget *widget) const;
  ShellCommandWidget * toWidget (const QModelIndex &index) const;

  void update (ShellCommandWidget *widget);
  void remove (QObject *widget);

  QVector<ShellCommandWidget *> widgets_;
  QColor runningColor_;
  QColor finishedColor_;
  QColor erroredColor_;
};
