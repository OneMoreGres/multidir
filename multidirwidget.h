#pragma once

#include <QWidget>

class DirWidget;

class QFileSystemModel;
class QSettings;
class QGridLayout;

class MultiDirWidget : public QWidget
{
public:
  explicit MultiDirWidget (QWidget *parent = nullptr);
  ~MultiDirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

protected:
  void keyPressEvent (QKeyEvent *event) override;

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  DirWidget * addWidget ();

  QFileSystemModel *model_;
  QList<DirWidget *> widgets_;
  QGridLayout *layout_;
};
