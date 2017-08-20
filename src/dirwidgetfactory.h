#pragma once

class DirWidget;
class FileSystemModel;

class QWidget;

class DirWidgetFactory
{
public:
  DirWidgetFactory (FileSystemModel *model);

  DirWidget * create (QWidget *parent);

private:
  FileSystemModel *model_;
};
