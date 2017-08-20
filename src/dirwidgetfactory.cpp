#include "dirwidgetfactory.h"
#include "debug.h"
#include "dirwidget.h"
#include "mainwindow.h"

DirWidgetFactory::DirWidgetFactory (FileSystemModel *model, ShellCommandModel *commands,
                                    MainWindow *main) :
  model_ (model),
  commands_ (commands),
  main_ (main)
{

}

DirWidget * DirWidgetFactory::create (QWidget *parent)
{
  auto *widget = new DirWidget (model_, commands_, parent);
  QObject::connect (main_, &MainWindow::nameFilterChanged, widget, &DirWidget::setNameFilter);
  return widget;
}
