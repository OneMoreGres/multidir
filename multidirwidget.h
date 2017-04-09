#pragma once

#include <QWidget>

class DirWidget;
class FileSystemModel;

class QSettings;
class QGridLayout;
class QMenu;
class QLineEdit;

class MultiDirWidget : public QWidget
{
Q_OBJECT
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
  void addToLayout (DirWidget *widget);
  void showContextMenu ();
  void activateFindMode ();
  void setIsOverlay (bool isOn);
  bool isOverlay () const;

  FileSystemModel *model_;
  QList<DirWidget *> widgets_;
  QGridLayout *layout_;
  QMenu *menu_;
  QAction *overlayAction_;
  QLineEdit *findEdit_;
};
