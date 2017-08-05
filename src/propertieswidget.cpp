#include "propertieswidget.h"
#include "utils.h"
#include "filepermissions.h"

#include <QFormLayout>
#include <QFileInfo>
#include <QDateTime>
#include <QLabel>
#include <QDialogButtonBox>


namespace
{

qint64 getSize (const QFileInfo &info)
{
  qint64 result = 0;
  if (info.isDir ())
  {
    for (const auto &i: utils::dirEntries (info))
    {
      result += getSize (i);
    }
  }
  else
  {
    result += info.size ();
  }
  return result;
}
}

PropertiesWidget::PropertiesWidget (const QFileInfo &info, QWidget *parent) :
  QWidget (parent)
{
  setWindowTitle (tr ("Properties"));

  auto layout = new QFormLayout (this);

  layout->addRow (tr ("Name: "), new QLabel (info.fileName ()));
  layout->addRow (tr ("Path: "), new QLabel (info.absolutePath ()));
  layout->addRow (tr ("Size: "), new QLabel (utils::sizeString (getSize (info))));
  layout->addRow (tr ("Created: "), new QLabel (info.created ().toString ()));
  layout->addRow (tr ("Last modified: "), new QLabel (info.lastModified ().toString ()));
  layout->addRow (tr ("Owner: "), new QLabel (info.owner ()));
  layout->addRow (tr ("Group: "), new QLabel (info.group ()));
  layout->addRow (tr ("Access rights: "),
                  new QLabel (FilePermissions::toFullString (info.permissions ())));

  auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok, this);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &QWidget::close);
  layout->addRow (buttons);
}

#include "moc_propertieswidget.cpp"
