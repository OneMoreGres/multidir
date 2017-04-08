#pragma once

#include <QFileSystemModel>

class FileSystemModel : public QFileSystemModel
{
public:
  explicit FileSystemModel (QObject *parent = nullptr);
  bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row,
                     int column, const QModelIndex &parent) override;
};
