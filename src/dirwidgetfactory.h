#pragma once

class DirWidget;
class FileSystemModel;
class ShellCommandModel;

class QWidget;

class DirWidgetFactory
{
public:
  DirWidgetFactory (FileSystemModel *model, ShellCommandModel *commands);

  DirWidget * create (QWidget *parent);

private:
  FileSystemModel *model_;
  ShellCommandModel *commands_;
};
