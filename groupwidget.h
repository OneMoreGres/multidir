#pragma once

#include <QWidget>

class DirWidget;
class TiledView;
class FileSystemModel;
class FileOperation;

class QSettings;
class QFileInfo;

class GroupWidget : public QWidget
{
Q_OBJECT
public:
  GroupWidget (FileSystemModel &model, QWidget *parent = nullptr);
  ~GroupWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setNameFilter (const QString &filter);
  DirWidget * addWidget ();

  QString name () const;
  void setName (const QString &name);

signals:
  void consoleRequested (const QString &path);
  void editorRequested (const QString &path);
  void fileOperation (QSharedPointer<FileOperation> operation);

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void add (const QFileInfo &path);
  void updateWidgetNames ();

  QString name_;
  FileSystemModel *model_;
  QList<DirWidget *> widgets_;
  TiledView *view_;
};
