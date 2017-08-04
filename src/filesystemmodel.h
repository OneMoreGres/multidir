#pragma once

#include <QFileSystemModel>

class FileOperation;

class FileSystemModel : public QFileSystemModel
{
Q_OBJECT
public:
  enum Column : int
  {
    Name, Size, Type, Date, Permissions, Owner, Group, LinkTarget,
    ColumnCount
  };

  explicit FileSystemModel (QObject *parent = nullptr);
  bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row,
                     int column, const QModelIndex &parent) override;
  int columnCount (const QModelIndex &parent) const override;
  QVariant headerData (int section, Qt::Orientation orientation, int role) const override;
  QVariant data (const QModelIndex &index, int role) const override;
  bool setData (const QModelIndex &index, const QVariant &value, int role) override;
  Qt::ItemFlags flags (const QModelIndex &index) const override;

signals:
  void fileOperation (QSharedPointer<FileOperation> operation);
};
