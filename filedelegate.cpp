#include "filedelegate.h"
#include "constants.h"

#include <QDebug>

FileDelegate::FileDelegate (QObject *parent) :
  QStyledItemDelegate (parent)
{
}

QSize FileDelegate::sizeHint (const QStyleOptionViewItem &option, const QModelIndex &) const
{
  return {constants::listItemWidth,
          option.decorationSize.height () + option.fontMetrics.height () +
          constants::itemVerticalMargins};
}
