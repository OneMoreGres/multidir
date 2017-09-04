#pragma once

#include <QAbstractItemModel>

#include <memory>

class SearchResultsModel : public QAbstractItemModel
{
public:
  enum Column
  {
    Text, Offset,
    ColumnCount
  };

  explicit SearchResultsModel (QObject *parent = nullptr);
  ~SearchResultsModel ();

  void addFile (const QString &file);
  void addText (const QString &file, const QMap<int, QString> &occurrences);
  void clear ();

  QString fileName (const QModelIndex &index) const;
  int occurrenceOffset (const QModelIndex &index) const;

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
    Item (const QString &text, int charOffset, Item *parent);
    Item (const Item &r);
    Item &operator= (const Item &r);
    bool operator== (const Item &r) const;

    Item *parent{nullptr};
    QList<Item> children;
    QString text;
    int charOffset{0};
  };

  Item * toItem (const QModelIndex &index) const;
  QModelIndex toIndex (const Item &item) const;

  QList<Item> items_;
};
