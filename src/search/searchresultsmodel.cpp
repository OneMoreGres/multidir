#include "searchresultsmodel.h"

SearchResultsModel::SearchResultsModel (QObject *parent) :
  QAbstractItemModel (parent),
  items_ ()
{

}

SearchResultsModel::~SearchResultsModel ()
{

}

void SearchResultsModel::addFile (const QString &file)
{
  auto it = std::find (items_.begin (), items_.end (), file);
  if (it == items_.end ())
  {
    const auto last = items_.size ();
    beginInsertRows ({}, last, last);
    items_.append (Item{file});
    endInsertRows ();
  }
}

void SearchResultsModel::addText (const QString &file, int byteOffset, const QString &line)
{
  auto it = std::find (items_.begin (), items_.end (), file);
  if (it == items_.end ())
  {
    const auto last = items_.size ();
    beginInsertRows ({}, last, last);
    items_.append (Item{file});
    endInsertRows ();

    it = items_.end ();
    --it;
  }

  const auto row = int (it - items_.begin ());
  auto parent = index (row, 0, {});
  const auto last = it->children.size ();
  beginInsertRows (parent, last, last);
  it->children.append ({line, byteOffset, &(*it)});
  endInsertRows ();
}

void SearchResultsModel::clear ()
{
  beginResetModel ();
  items_.clear ();
  endResetModel ();
}

SearchResultsModel::Item * SearchResultsModel::toItem (const QModelIndex &index) const
{
  return static_cast<Item *>(index.internalPointer ());
}

QModelIndex SearchResultsModel::toIndex (const SearchResultsModel::Item &item) const
{
  const auto &list = item.parent ? item.parent->children : items_;
  auto *casted = const_cast<Item *>(&item);
  return createIndex (list.indexOf (*casted), 0, casted);
}

QModelIndex SearchResultsModel::index (int row, int column, const QModelIndex &parent) const
{
  const auto &list = parent.isValid () ? toItem (parent)->children : items_;
  if (row >= list.size ())
  {
    return {};
  }
  return createIndex (row, column, const_cast<Item *>(&list[row]));
}

QModelIndex SearchResultsModel::parent (const QModelIndex &child) const
{
  if (auto casted = toItem (child))
  {
    if (casted->parent)
    {
      return toIndex (*casted->parent);
    }
  }
  return {};
}

int SearchResultsModel::rowCount (const QModelIndex &parent) const
{
  const auto &list = parent.isValid () ? toItem (parent)->children : items_;
  return list.size ();
}

int SearchResultsModel::columnCount (const QModelIndex & /*parent*/) const
{
  return ColumnCount;
}

QVariant SearchResultsModel::headerData (int section, Qt::Orientation orientation,
                                         int role) const
{
  if (role != Qt::DisplayRole)
  {
    return {};
  }

  if (orientation == Qt::Vertical)
  {
    return section + 1;
  }

  switch (section)
  {
    case Column::Text: return tr ("Text");
    case Column::ByteOffset: return tr ("Offset");
  }

  return {};
}

QVariant SearchResultsModel::data (const QModelIndex &index, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return {};
  }

  auto casted = toItem (index);
  if (!casted)
  {
    return {};
  }

  switch (index.column ())
  {
    case Column::Text: return casted->text;
    case Column::ByteOffset: return casted->parent ? casted->byteOffset : QVariant ();
  }

  return {};
}



SearchResultsModel::Item::Item (const QString &text) :
  Item (text, 0, nullptr)
{

}

SearchResultsModel::Item::Item (const QString &text, int offset, Item *parent) :
  parent (parent),
  children (),
  text (text),
  byteOffset (offset)
{

}

SearchResultsModel::Item::Item (const Item &r) :
  parent (r.parent),
  children (r.children),
  text (r.text),
  byteOffset (r.byteOffset)
{
  for (auto &i: children)
  {
    i.parent = this;
  }
}

bool SearchResultsModel::Item::operator== (const SearchResultsModel::Item &r) const
{
  return text == r.text;
}
