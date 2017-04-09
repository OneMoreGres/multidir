#include "controller.h"

#include <QApplication>
#include <QDir>
#include <QLockFile>
#include <QDebug>

int main (int argc, char *argv[])
{
  QApplication a (argc, argv);
  a.setOrganizationName ("Gres");
  a.setApplicationName ("MultiDir");
  a.setQuitOnLastWindowClosed (false);

  QLockFile f (QDir::home ().absoluteFilePath (".multidir.lock"));
  if (!f.tryLock ())
  {
    qDebug () << "Another instance is running. Lock file is busy.";
    return 0;
  }

  Controller control;
  return a.exec ();
}
