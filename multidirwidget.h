#pragma once

#include <QWidget>

class DirWidget;
class TiledView;
class FileSystemModel;
class FileOperation;

class QSettings;
class QFileInfo;

class MultiDirWidget : public QWidget
{
Q_OBJECT
public:
  MultiDirWidget (FileSystemModel &model, QWidget *parent = nullptr);
  ~MultiDirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setNameFilter (const QString &filter);
  DirWidget * addWidget ();

signals:
  void consoleRequested (const QString &path);
  void fileOperation (QSharedPointer<FileOperation> operation);

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void add (const QFileInfo &path);
  void updateWidgetNames ();

  FileSystemModel *model_;
  QList<DirWidget *> widgets_;
  TiledView *view_;
};
