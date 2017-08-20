#pragma once

class DirWidget;
class FileSystemModel;
class ShellCommandModel;
class MainWindow;

class QWidget;

class DirWidgetFactory
{
public:
  DirWidgetFactory (FileSystemModel *model, ShellCommandModel *commands, MainWindow *main);

  DirWidget * create (QWidget *parent);

private:
  FileSystemModel *model_;
  ShellCommandModel *commands_;
  MainWindow *main_;
};
