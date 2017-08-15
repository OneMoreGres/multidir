#pragma once

#include <QWidget>

class GroupWidget;
class FileSystemModel;
class FileOperation;
class FileOperationWidget;

class QSettings;
class QAction;
class QMenu;
class QLineEdit;
class QStackedWidget;
class QActionGroup;

class GroupsView : public QWidget
{
Q_OBJECT
public:
  explicit GroupsView (FileSystemModel &model, QWidget *parent = nullptr);

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  int count () const;
  GroupWidget &current () const;
  GroupWidget &at (int index) const;
  int currentIndex () const;
  void setCurrentIndex (int index);

  void addWidgetToCurrent ();
  GroupWidget &add ();
  void renameCurrent (const QString &newName);
  void removeCurrent ();

signals:
  void currentChanged (const QString &name);
  void restored ();

  void consoleRequested (const QString &path);
  void editorRequested (const QString &path);
  void setNameFilter (const QString &filter);

private:
  FileSystemModel &model_;
  QStackedWidget *groups_;
};
