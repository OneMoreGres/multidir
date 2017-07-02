#include "propertieswidget.h"
#include "utils.h"

#include <QFormLayout>
#include <QFileInfo>
#include <QDateTime>
#include <QLabel>
#include <QDialogButtonBox>


namespace
{

enum Mode : size_t
{
  User = 0, Group, Other
};

QString permissionString (QFile::Permissions permissions, Mode mode)
{
  using P = QFile::Permission;
  std::array<QFile::Permission, 9> map {{P::ReadUser, P::WriteUser, P::ExeUser,
                                         P::ReadGroup, P::WriteGroup, P::ExeGroup,
                                         P::ReadOther, P::WriteOther, P::ExeOther}};
  std::array<QChar, 3> modes {{'r', 'w', 'x'}};
  QString result;
  for (size_t i = 0; i < 3; ++i)
  {
    if (permissions & map [3 * mode + i])
    {
      result += modes[i];
    }
  }
  return result;
}


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
  const auto perm = info.permissions ();
  layout->addRow (tr ("Access user: "), new QLabel (permissionString (perm, Mode::User)));
  layout->addRow (tr ("Access group: "), new QLabel (permissionString (perm, Mode::Group)));
  layout->addRow (tr ("Access other: "), new QLabel (permissionString (perm, Mode::Other)));

  auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok, this);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &QWidget::close);
  layout->addRow (buttons);
}
