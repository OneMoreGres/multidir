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
  enum class TabSwitchOrder : int
  {
    ByIndex, ByPosition
  };

  GroupWidget (QSharedPointer<DirWidgetFactory> widgetFactory, QWidget *parent = nullptr);
  ~GroupWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  DirWidget * addWidget ();

  QString name () const;
  void setName (const QString &name);

public slots:
  void updateSettings ();

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void nextTab (DirWidget *widget);
  void previousTab (DirWidget *widget);
  void add (const QFileInfo &path);
  void updateWidgetNames ();
  void updateWidgetShortcuts ();
  void setIds (const QString &ids);
  QList<QWidget *> orderedTabs () const;

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
  TabSwitchOrder tabSwitchOrder_;
};
