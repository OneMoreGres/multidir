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

  const QString &ids () const;
  void setIds (const QString &ids);

public slots:
  void updateSettings ();

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void nextTab (DirWidget *widget);
  void add (const QFileInfo &path);
  void updateWidgetNames ();
  void updateWidgetShortcuts ();

  struct Widget
  {
    DirWidget *widget;
    QAction *action;

    bool operator== (const Widget &r) const;
  };

  QString name_;
  FileSystemModel *model_;
  QList<Widget> widgets_;
  TiledView *view_;
  QString ids_;
};
