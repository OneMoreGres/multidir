#include "dirwidgetfactory.h"
#include "debug.h"
#include "dirwidget.h"

DirWidgetFactory::DirWidgetFactory (FileSystemModel *model) :
  model_ (model)
{

}

DirWidget * DirWidgetFactory::create (QWidget *parent)
{
  auto *widget = new DirWidget (model_, parent);
  return widget;
}
