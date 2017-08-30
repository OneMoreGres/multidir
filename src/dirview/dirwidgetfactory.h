#pragma once

class DirWidget;
class FileSystemModel;
class ShellCommandModel;
class FileOperationModel;
class MainWindow;

class QWidget;

class DirWidgetFactory
{
public:
  DirWidgetFactory (FileSystemModel *model, ShellCommandModel *commands,
                    FileOperationModel *fileOperations, MainWindow *main);

  DirWidget * create (QWidget *parent);

private:
  FileSystemModel *model_;
  ShellCommandModel *commands_;
  FileOperationModel *fileOperations_;
  MainWindow *main_;
};
