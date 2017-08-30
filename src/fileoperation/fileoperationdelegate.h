#pragma once

#include <QStyledItemDelegate>

class FileOperation;

class FileOperationDelegate : public QStyledItemDelegate
{
public:
  explicit FileOperationDelegate (QObject *parent = nullptr);

  void paint (QPainter *painter, const QStyleOptionViewItem &option,
              const QModelIndex &index) const override;

  QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  QString text (const QModelIndex &index) const;
  QString actionText (int action) const;
};
