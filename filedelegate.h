#pragma once

#include <QStyledItemDelegate>

class FileDelegate : public QStyledItemDelegate
{
public:
  explicit FileDelegate (QObject *parent = nullptr);

  QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
