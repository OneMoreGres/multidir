#pragma once

#include <QAbstractItemModel>

#include <memory>

class SearchResultsModel : public QAbstractItemModel
{
public:
  enum Column
  {
    Text, ByteOffset,
    ColumnCount
  };

  explicit SearchResultsModel (QObject *parent = nullptr);
  ~SearchResultsModel ();

  void addFile (const QString &file);
  void addText (const QString &file, int byteOffset, const QString &occurence);
  void clear ();

  QModelIndex index (int row, int column, const QModelIndex &parent) const override;
  QModelIndex parent (const QModelIndex &child) const override;
  int rowCount (const QModelIndex &parent) const override;
  int columnCount (const QModelIndex &parent) const override;
  QVariant headerData (int section, Qt::Orientation orientation, int role) const override;
  QVariant data (const QModelIndex &index, int role) const override;

private:
  struct Item
  {
    Item (const QString &text = {});
    Item (const QString &text, int byteOffset, Item *parent);
    Item (const Item &r);
    bool operator== (const Item &r) const;

    Item *parent{nullptr};
    QVector<Item> children;
    QString text;
    int byteOffset{0};
  };

  Item * toItem (const QModelIndex &index) const;
  QModelIndex toIndex (const Item &item) const;

  QVector<Item> items_;
};
