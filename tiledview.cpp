#include "tiledview.h"

#include <QResizeEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QLayout>
#include <QDebug>


using namespace std;

//! Class for drag-n-drop support.
class TileMime : public QMimeData
{
Q_OBJECT
public:
  Tile *tile;
};



//! Tile info.
class Tile
{
public:
  QWidget *widget = nullptr;
  QRect geometry;
  int row = -1;
  int col = -1;

  void setWidget (QWidget *widget);
  bool operator< (const Tile &r) const { return tie (row, col) < tie (r.row,r.col); }
};

void Tile::setWidget (QWidget *widget)
{
  this->widget = widget;
  if (widget)
  {
    widget->setGeometry (geometry);
  }
}



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
  updateGeometry ();
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

Tile * TiledView::findTile (const QPoint &pos) const
{
  auto it = find_if (cbegin (tiles_), cend (tiles_), [pos](const Tile &i) {
    return i.geometry.contains (pos);
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
  tile->setWidget (widget);
}

void TiledView::remove (QWidget &widget)
{
  auto tile = findTile (&widget);
  Q_ASSERT (tile);
  tile->widget = nullptr;
  cleanupDimensions ();
  updateGeometry ();
}

void TiledView::removeRow (int index)
{
  removeDimesion (rows_, &Tile::row, index);
}

void TiledView::removeColumn (int index)
{
  removeDimesion (columns_, &Tile::col, index);
}

void TiledView::cleanupDimensions ()
{
  QSet<int> rows, cols;
  for (const auto &i: tiles_)
  {
    if (i.widget)
    {
      rows << i.row;
      cols << i.col;
    }
  }

  auto edited = false;
  for (auto i = rows_.size () - 1; i >= 0; --i)
  {
    if (!rows.contains (i))
    {
      edited = true;
      removeRow (i);
    }
  }

  for (auto i = columns_.size () - 1; i >= 0; --i)
  {
    if (!cols.contains (i))
    {
      edited = true;
      removeColumn (i);
    }
  }

  if (edited)
  {
    updateTilesGeometry ();
  }
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

QSize TiledView::sizeHint () const
{
  return getSizeHint (&QWidget::sizeHint);
}

QSize TiledView::minimumSizeHint () const
{
  return getSizeHint (&QWidget::minimumSizeHint);
}

QSize TiledView::getSizeHint (QSize (QWidget::*type)() const) const
{
  if (tiles_.isEmpty ())
  {
    return {};
  }

  QMap<int, int> widths, heights;
  for (const auto &i: tiles_)
  {
    if (!i.widget)
    {
      continue;
    }
    const auto hint = (i.widget->*type)();
    heights[i.col] += hint.height () + spacing_;
    widths[i.row] += hint.width () + spacing_;
  }
  return QSize (*max_element (cbegin (widths), cend (widths)) - spacing_ + 2 * margin_,
                *max_element (cbegin (heights), cend (heights)) - spacing_ + 2 * margin_);
}

void TiledView::mousePressEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = event->pos ();
  }
}

void TiledView::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = {};
  }
}

void TiledView::mouseMoveEvent (QMouseEvent *event)
{
  if (dragStartPos_.isNull ())
  {
    return;
  }
  if ((event->pos () - dragStartPos_).manhattanLength () < QApplication::startDragDistance ())
  {
    return;
  }
  const auto tile = findTile (dragStartPos_);
  if (!tile || !tile->widget)
  {
    return;
  }

  auto *drag = new QDrag (this);
  drag->setPixmap (tile->widget->grab ());

  auto *mime = new TileMime;
  mime->tile = const_cast<Tile *>(tile);
  drag->setMimeData (mime);

  drag->exec (Qt::MoveAction);
}

void TiledView::dragEnterEvent (QDragEnterEvent *event)
{
  if (qobject_cast<const TileMime *>(event->mimeData ()))
  {
    event->acceptProposedAction ();
  }
}

void TiledView::dropEvent (QDropEvent *event)
{
  auto *mime = qobject_cast<const TileMime *>(event->mimeData ());
  if (!mime)
  {
    return;
  }
  auto source = mime->tile;
  Q_ASSERT (source);

  auto pos = event->pos ();
  if (height () - pos.y () < margin_)
  {
    addRow ();
    updateTilesGeometry ();
    pos.ry () -= margin_;
  }
  else if (width () - pos.x () < margin_)
  {
    addColumn ();
    updateTilesGeometry ();
    pos.rx () -= margin_;
  }

  auto target = findTile (pos);
  if (!target)
  {
    return;
  }

  if (event->dropAction () == Qt::MoveAction)
  {
    auto targetWidget = target->widget;
    target->setWidget (source->widget);
    source->setWidget (targetWidget);
    cleanupDimensions ();
    event->acceptProposedAction ();
  }
}

#include "tiledview.moc"
