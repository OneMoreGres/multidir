#pragma once

#include <QWidget>

class DirWidget;
class TiledView;
class DirWidgetFactory;

class QSettings;
class QFileInfo;

class GroupWidget : public QWidget
{
Q_OBJECT
public:
  GroupWidget (QSharedPointer<DirWidgetFactory> widgetFactory, QWidget *parent = nullptr);
  ~GroupWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  void setNameFilter (const QString &filter);
  DirWidget * addWidget ();

  QString name () const;
  void setName (const QString &name);

public slots:
  void updateSettings ();

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void nextTab (DirWidget *widget);
  void add (const QFileInfo &path);
  void updateWidgetNames ();
  void updateWidgetShortcuts ();
  void setIds (const QString &ids);

  struct Widget
  {
    DirWidget *widget;
    QAction *action;

    bool operator== (const Widget &r) const;
  };

  QString name_;
  QSharedPointer<DirWidgetFactory> widgetFactory_;
  QList<Widget> widgets_;
  TiledView *view_;
  QString ids_;
};
