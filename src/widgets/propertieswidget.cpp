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

struct AggregateInfo
{
  qint32 files{0};
  qint32 hidden{0};
  qint32 links{0};
  qint32 dirs{0};
  qint64 size{0};

  AggregateInfo &operator+= (const AggregateInfo &r)
  {
    files += r.files;
    hidden += r.hidden;
    links += r.links;
    dirs += r.dirs;
    size += r.size;
    return *this;
  }
};

AggregateInfo getAggregate (const QFileInfo &info, bool isTopLevel)
{
  AggregateInfo result;
  if (info.isDir ())
  {
    if (!isTopLevel)
    {
      ++result.dirs;
    }

    for (const auto &i: utils::dirEntries (info))
    {
      result += getAggregate (i, false);
    }
  }
  else
  {
    ++result.files;
    if (info.isHidden ())
    {
      ++result.hidden;
    }
    if (info.isSymLink ())
    {
      ++result.links;
    }
    result.size += info.size ();
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
  const auto aggregate = getAggregate (info, true);
  layout->addRow (tr ("Files: "), new QLabel (QString::number (aggregate.files)));
  layout->addRow (tr ("Hidden: "), new QLabel (QString::number (aggregate.hidden)));
  layout->addRow (tr ("Links: "), new QLabel (QString::number (aggregate.links)));
  layout->addRow (tr ("Directories: "), new QLabel (QString::number (aggregate.dirs)));
  layout->addRow (tr ("Size: "), new QLabel (utils::sizeString (aggregate.size, 3)));
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
