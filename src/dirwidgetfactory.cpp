#include "dirwidgetfactory.h"
#include "debug.h"
#include "dirwidget.h"

DirWidgetFactory::DirWidgetFactory (FileSystemModel *model, ShellCommandModel *commands) :
  model_ (model),
  commands_ (commands)
{

}

DirWidget * DirWidgetFactory::create (QWidget *parent)
{
  auto *widget = new DirWidget (model_, commands_, parent);
  return widget;
}
