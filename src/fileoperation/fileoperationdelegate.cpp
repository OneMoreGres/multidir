#include "fileoperationdelegate.h"
#include "fileoperationmodel.h"
#include "fileoperation.h"
#include "debug.h"

#include <QApplication>

FileOperationDelegate::FileOperationDelegate (QObject *parent) :
  QStyledItemDelegate (parent)
{
}

void FileOperationDelegate::paint (QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
  const auto bundle = index.data ().value<const FileOperationModel::Bundle *>();
  ASSERT (bundle);

  QStyleOptionProgressBar bar;
  bar.minimum = 0;
  bar.maximum = 100;
  bar.progress = bundle->progress;

  bar.textVisible = true;
  bar.text = text (index);

  bar.rect = option.rect;
  bar.state = QStyle::State_Enabled;
  bar.palette = option.palette;
  bar.fontMetrics = option.fontMetrics;

  QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &bar, painter);
}

QSize FileOperationDelegate::sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  const auto metrics = option.fontMetrics;
  return metrics.boundingRect (text (index)).size ();
}

QString FileOperationDelegate::text (const QModelIndex &index) const
{
  const auto bundle = index.data ().value<const FileOperationModel::Bundle *>();
  ASSERT (bundle);

  const auto target = (!bundle->target.isEmpty ())
                      ? tr (" to \"%1\"").arg (bundle->target)
                      : QString ();

  const auto maxFileNameLength = 15;
  const auto current = (bundle->current.length () < maxFileNameLength)
                       ? bundle->current
                       : QLatin1String ("...") + bundle->current.right (maxFileNameLength - 3);

  auto result = QString ("%1 \"%2\"%3").arg (actionText (bundle->action), current, target);
  return result;
}

QString FileOperationDelegate::actionText (int action) const
{
  const auto actionText = QMap<FileOperation::Action, QString>{
    {FileOperation::Action::Copy, tr ("Copying")},
    {FileOperation::Action::Link, tr ("Linking")},
    {FileOperation::Action::Move, tr ("Moving")},
    {FileOperation::Action::Remove, tr ("Removing")},
    {FileOperation::Action::Trash, tr ("Moving to trash")}
  }.value (FileOperation::Action (action));
  return actionText;
}
