#pragma once

#include <QWidget>

class DirWidget;
class FileSystemModel;
class TiledView;

class QSettings;
class QMenu;
class QLineEdit;
class QFileInfo;

class MultiDirWidget : public QWidget
{
Q_OBJECT
public:
  explicit MultiDirWidget (QWidget *parent = nullptr);
  ~MultiDirWidget ();

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

signals:
  void settingsRequested ();

protected:
  void keyPressEvent (QKeyEvent *event) override;

private:
  void close (DirWidget *widget);
  void clone (DirWidget *widget);
  void add (const QFileInfo &path);
  DirWidget * addWidget ();
  void showContextMenu ();
  void activateFindMode ();
  void showAbout ();

  FileSystemModel *model_;
  QList<DirWidget *> widgets_;
  TiledView *view_;
  QMenu *contextMenu_;
  QLineEdit *findEdit_;
};
