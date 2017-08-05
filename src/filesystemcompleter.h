#pragma once

#include <QCompleter>

class FileSystemModel;
class ProxyModel;

class FileSystemCompleter : public QCompleter
{
Q_OBJECT
public:
  FileSystemCompleter (FileSystemModel *model, QObject *parent = nullptr);

public:
  QString pathFromIndex (const QModelIndex &index) const override;
  QStringList splitPath (const QString &path) const override;

private:
  ProxyModel *proxy_;
  mutable QString lastPath_;
};
