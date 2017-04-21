#include "delegate.h"
#include "constants.h"

#include <QDebug>

Delegate::Delegate (QObject *parent) :
  QStyledItemDelegate (parent)
{
}

QSize Delegate::sizeHint (const QStyleOptionViewItem &option, const QModelIndex &) const
{
  return {constants::listItemWidth,
          option.decorationSize.height () + option.fontMetrics.height () +
          constants::itemVerticalMargins};
}
