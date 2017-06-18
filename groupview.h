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

class GroupView : public QWidget
{
Q_OBJECT
public:
  explicit GroupView (FileSystemModel &model, QWidget *parent = nullptr);

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

  void consoleRequested (const QString &path);
  void editorRequested (const QString &path);
  void setNameFilter (const QString &filter);
  void fileOperation (QSharedPointer<FileOperation> operation);

private:
  FileSystemModel &model_;
  QStackedWidget *groups_;
};
