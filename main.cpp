#include "controller.h"

#include <QApplication>

int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName ("Gres");
  a.setApplicationName ("MultiDir");
  a.setQuitOnLastWindowClosed (false);

  Controller control;
  return a.exec ();
}
