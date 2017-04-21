#pragma once

#include <QStyledItemDelegate>

class Delegate : public QStyledItemDelegate
{
public:
  explicit Delegate (QObject *parent = nullptr);

  QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
