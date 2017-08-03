#include "filepermissiondelegate.h"
#include "filepermissions.h"
#include "debug.h"

#include <QRegularExpressionValidator>
#include <QLineEdit>


FilePermissionDelegate::FilePermissionDelegate (QObject *parent) :
  QStyledItemDelegate (parent)
{
}

QWidget * FilePermissionDelegate::createEditor (QWidget *parent,
                                                const QStyleOptionViewItem & /*option*/,
                                                const QModelIndex & /*index*/) const
{
  auto editor = new QLineEdit (parent);
  auto validator = new QRegularExpressionValidator (parent);
  using RE = QRegularExpression;
  const auto regExpText = "^([01234567]{0,3}|([r-]?[w-]?[x-]?){0,3})$";
  validator->setRegularExpression (RE (regExpText, RE::CaseInsensitiveOption));
  editor->setValidator (validator);
  return editor;
}

void FilePermissionDelegate::setEditorData (QWidget *editor, const QModelIndex &index) const
{
  auto casted = qobject_cast<QLineEdit *>(editor);
  ASSERT (casted);
  const auto modelData = index.data (Qt::EditRole).toInt ();
  casted->setText (FilePermissions::toString (QFile::Permissions (modelData)));
}

void FilePermissionDelegate::setModelData (QWidget *editor, QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
  auto casted = qobject_cast<QLineEdit *>(editor);
  ASSERT (casted);
  const auto text = casted->text ();
  if (text.isEmpty ())
  {
    return;
  }
  QFile::Permissions permissions;
  if (text.at (0).isDigit ())
  {
    permissions = FilePermissions::fromNumericString (text);
  }
  else
  {
    permissions = FilePermissions::fromString (text);
  }
  model->setData (index, int (permissions));
}

QString FilePermissionDelegate::displayText (const QVariant &value,
                                             const QLocale & /*locale*/) const
{
  return FilePermissions::toFullString (QFile::Permissions (value.toInt ()));
}
