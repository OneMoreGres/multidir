#include "tiledview.h"

#include <QResizeEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QLayout>
#include <QDebug>


using namespace std;


//! Tile info.
class Tile
{
public:
  QWidget *widget = nullptr;
  QRect geometry;
  int row = -1;
  int col = -1;

  bool operator< (const Tile &r) const { return tie (row, col) < tie (r.row,r.col); }
};


TiledView::TiledView (QWidget *parent) :
  QWidget (parent),
  rows_ (),
  columns_ (),
  tiles_ (),
  spacing_ (2),
  margin_ (2),
  dragStartPos_ ()
{
  setAcceptDrops (true);
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
}

TiledView::~TiledView ()
{
}

void TiledView::add (QWidget &widget)
{
  auto *ptr = &widget;
  ptr->setParent (this);
  reserveTile ();
  emplace (ptr);
  ptr->show ();
}

void TiledView::reserveTile ()
{
  if (findTile (nullptr))
  {
    return;
  }

  if (!tiles_.isEmpty ())
  {
    const auto widgetAspectRatio = double (width ()) / height ();
    const auto tilesAspectRatio = double (columns_.size ()) / rows_.size ();
    if (tilesAspectRatio < widgetAspectRatio)
    {
      addColumn ();
    }
    else
    {
      addRow ();
    }
  }
  else
  {
    addRow ();
    addColumn ();
  }

  updateTilesGeometry ();
}

Tile * TiledView::findTile (QWidget *widget) const
{
  auto it = find_if (cbegin (tiles_), cend (tiles_), [widget](const Tile &i) {
    return i.widget == widget;
  });
  if (it == cend (tiles_))
  {
    return nullptr;
  }
  return &const_cast<Tile &>(*it);
}

void TiledView::addRow ()
{
  addDimesion (rows_, columns_, height (), true);
}

void TiledView::addColumn ()
{
  addDimesion (columns_, rows_, width (), false);
}

void TiledView::addDimesion (QList<int> &sizes, const QList<int> &opposite, int fullSize, bool isRow)
{
  const auto index = sizes.size ();
  for (auto i = 0, end = opposite.size (); i < end; ++i)
  {
    tiles_.append ({nullptr, {}, (isRow ? index : i), (isRow ? i : index)});
  }

  sort (begin (tiles_), end (tiles_));

  const auto size = max (0, (fullSize - 2 * margin_ - index * spacing_) / (index + 1));
  adjustSizes (sizes, -size - spacing_);
  sizes << size;
}

void TiledView::removeDimesion (QList<int> &sizes, int Tile::*field, int index)
{
  tiles_.erase (remove_if (begin (tiles_), end (tiles_), [index, field](const Tile &i) {
    return i.*field == index;
  }), end (tiles_));
  for (auto &i: tiles_)
  {
    if (i.*field > index)
    {
      --(i.*field);
    }
  }
  const auto size = sizes[index] + (sizes.size () > 1 ? spacing_ : 0);
  sizes.removeAt (index);
  adjustSizes (sizes, size);
}

void TiledView::adjustSizes (QList<int> &sizes, int sizeToFill) const
{
  if (sizes.isEmpty ())
  {
    return;
  }

  const auto change = sizeToFill / sizes.size ();
  for (auto &i: sizes)
  {
    i = max (0, i + change);
  }
}

void TiledView::updateTilesGeometry ()
{
  if (tiles_.isEmpty ())
  {
    return;
  }

  auto left = margin_;
  auto top = margin_;
  auto lastRow = 0;
  for (auto &i: tiles_)
  {
    if (i.row != lastRow)
    {
      left = margin_;
      top += spacing_ + rows_[lastRow];
      lastRow = i.row;
    }
    const auto width = columns_[i.col];
    const auto height = rows_[i.row];
    i.geometry = {left, top, width, height};
    left += width + spacing_;
    if (i.widget)
    {
      i.widget->setGeometry (i.geometry);
    }
  }
}

void TiledView::emplace (QWidget *widget)
{
  auto tile = findTile (nullptr);
  Q_ASSERT (tile);
  tile->widget = widget;
  widget->setGeometry (tile->geometry);
}

void TiledView::remove (QWidget &widget)
{
  auto tile = findTile (&widget);
  Q_ASSERT (tile);
  tile->widget = nullptr;

  auto row = find_if (cbegin (tiles_), cend (tiles_), [tile](const Tile &i) {
    return i.widget && i.row == tile->row;
  });
  const auto rowEmpty = (row == cend (tiles_));
  if (rowEmpty)
  {
    removeRow (tile->row);
  }


  auto col = find_if (cbegin (tiles_), cend (tiles_), [tile](const Tile &i) {
    return i.widget && i.col == tile->col;
  });
  const auto colEmpty = (col == cend (tiles_));
  if (colEmpty)
  {
    removeColumn (tile->col);
  }

  if (rowEmpty || colEmpty)
  {
    updateTilesGeometry ();
  }
}

void TiledView::removeRow (int index)
{
  removeDimesion (rows_, &Tile::row, index);
}

void TiledView::removeColumn (int index)
{
  removeDimesion (columns_, &Tile::col, index);
}

void TiledView::resizeEvent (QResizeEvent *event)
{
  const auto change = event->size () - tilesSize ();
  adjustSizes (columns_, change.width ());
  adjustSizes (rows_, change.height ());
  updateTilesGeometry ();
}

QSize TiledView::tilesSize () const
{
  if (tiles_.isEmpty ())
  {
    return {2 * margin_, 2 * margin_};
  }

  const auto width = accumulate (cbegin (columns_), cend (columns_), 0, [this](int sum, int i) {
    return sum + i + spacing_;
  });
  const auto height = accumulate (cbegin (rows_), cend (rows_), 0, [this](int sum, int i) {
    return sum + i + spacing_;
  });
  return {width + 2 * margin_ - spacing_, height + 2 * margin_ - spacing_};
}
