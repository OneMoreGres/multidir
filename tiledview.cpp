#include "tiledview.h"
#include "backport.h"

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


enum class Border
{
  None, Right, Bottom, BottomRight
};

//! Tile info.
class Tile
{
public:
  QWidget *widget = nullptr;
  QRect geometry = {};
  int row = -1;
  int col = -1;
  QMap<Border, QWidget *> borders = {};

  void setWidget (QWidget *widget);
  void setGeometry (const QRect &rect, int spacing);
  void createBorder (Border border, QWidget *parent);
  void removeBorder (Border border);
  void removeBorders ();
  Border borderAt (const QPoint &pos) const;
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

void Tile::setGeometry (const QRect &rect, int spacing)
{
  geometry = rect;
  if (widget)
  {
    widget->setGeometry (rect);
  }
  if (borders.contains (Border::Right))
  {
    borders[Border::Right]->setGeometry (rect.right (), rect.top (), spacing, rect.height ());
  }
  if (borders.contains (Border::Bottom))
  {
    borders[Border::Bottom]->setGeometry (rect.left (), rect.bottom (), rect.width (), spacing);
  }
  if (borders.contains (Border::BottomRight))
  {
    borders[Border::BottomRight]->setGeometry (rect.right (), rect.bottom (), spacing, spacing);
  }
}

void Tile::createBorder (Border border, QWidget *parent)
{
  if (borders.contains (border))
  {
    return;
  }

  auto w = new QWidget (parent);
  w->setCursor (QMap<Border,Qt::CursorShape>
                {{Border::Right, Qt::SizeHorCursor},{Border::Bottom, Qt::SizeVerCursor},
                 {Border::BottomRight, Qt::SizeAllCursor}}
                .value (border, Qt::ArrowCursor));
  borders[border] = w;
  w->show ();
}

void Tile::removeBorder (Border border)
{
  if (auto i = borders.take (border))
  {
    i->deleteLater ();
  }
}

void Tile::removeBorders ()
{
  for (auto i: borders)
  {
    i->deleteLater ();
  }
}

Border Tile::borderAt (const QPoint &pos) const
{
  for (auto i = cbegin (borders), end = cend (borders); i != end; ++i)
  {
    if (i.value ()->geometry ().contains (pos))
    {
      return i.key ();
    }
  }
  return Border::None;
}



TiledView::TiledView (QWidget *parent) :
  QWidget (parent),
  rows_ (),
  columns_ (),
  tiles_ (),
  spacing_ (2),
  margin_ (2),
  dragStartPos_ (),
  resizeRow_ (-1),
  resizeCol_ (-1),
  resizeDir_ (0)
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

Tile * TiledView::findTileBorders (const QPoint &pos) const
{
  auto it = find_if (cbegin (tiles_), cend (tiles_), [pos](const Tile &i) {
    return i.borderAt (pos) != Border::None;
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

  updateTilesBorders ();
}

void TiledView::removeDimesion (QList<int> &sizes, int Tile::*field, int index)
{
  for (auto &i :tiles_)
  {
    if (i.*field == index)
    {
      i.removeBorders ();
    }
  }

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

  updateTilesBorders ();
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
    i.setGeometry ({left, top, width, height}, spacing_);
    left += width + spacing_;
  }
}

void TiledView::updateTilesBorders ()
{
  if (tiles_.isEmpty ())
  {
    return;
  }

  const auto lastRow = rows_.size () - 1;
  const auto lastCol = columns_.size () - 1;
  for (auto &i: tiles_)
  {
    i.col < lastCol ? i.createBorder (Border::Right, this) : i.removeBorder (Border::Right);

    i.row < lastRow ? i.createBorder (Border::Bottom, this) : i.removeBorder (Border::Bottom);

    i.col < lastCol && i.row < lastRow
    ? i.createBorder (Border::BottomRight, this) : i.removeBorder (Border::BottomRight);
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

void TiledView::setResize (int col, int row, Qt::Orientations dir)
{
  resizeCol_ = col;
  resizeRow_ = row;
  resizeDir_ = dir;
}

void TiledView::handleResize (const QPoint &old, const QPoint &current)
{
  const auto diff = current - old;
  if (resizeDir_ & Qt::Horizontal && resizeCol_ < columns_.size () - 1)
  {
    const auto change = nonstd::clamp (diff.x (), -columns_[resizeCol_], columns_[resizeCol_ + 1]);
    columns_[resizeCol_] += change;
    columns_[resizeCol_ + 1] -= change;
  }

  if (resizeDir_ & Qt::Vertical && resizeRow_ < rows_.size () - 1)
  {
    const auto change = nonstd::clamp (diff.y (), -rows_[resizeRow_], rows_[resizeRow_ + 1]);
    rows_[resizeRow_] += change;
    rows_[resizeRow_ + 1] -= change;
  }

  updateTilesGeometry ();
}

void TiledView::mousePressEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = event->pos ();

    auto borders = findTileBorders (event->pos ());
    if (borders)
    {
      auto type = borders->borderAt (event->pos ());
      auto dir = QMap<Border,Qt::Orientations>
      {{Border::Right, Qt::Horizontal},{Border::Bottom, Qt::Vertical},
       {Border::BottomRight, Qt::Horizontal | Qt::Vertical}}
      .value (type, 0);
      setResize (borders->col, borders->row, dir);
    }
    event->accept ();
  }
}

void TiledView::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = {};
    setResize (-1, -1, 0);
  }
}

void TiledView::mouseMoveEvent (QMouseEvent *event)
{
  if (dragStartPos_.isNull ())
  {
    return;
  }


  if (resizeDir_)
  {
    handleResize (dragStartPos_, event->pos ());
    dragStartPos_ = event->pos ();
    event->accept ();
  }


  if ((event->pos () - dragStartPos_).manhattanLength () < QApplication::startDragDistance ())
  {
    return;
  }
  auto tile = findTile (dragStartPos_);
  if (!tile || !tile->widget)
  {
    return;
  }

  auto *drag = new QDrag (this);
  drag->setPixmap (tile->widget->grab ());

  auto *mime = new TileMime;
  mime->tile = tile;
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
  Q_ASSERT (source->widget);

  auto pos = event->pos ();
  auto sourceWidget = source->widget;
  if (height () - pos.y () < margin_)
  {
    addRow ();
    source = findTile (sourceWidget); // addRow can alter it
    updateTilesGeometry ();
    pos.ry () -= margin_;
  }
  else if (width () - pos.x () < margin_)
  {
    addColumn ();
    source = findTile (sourceWidget); // addColumn can alter it
    updateTilesGeometry ();
    pos.rx () -= margin_;
  }
  Q_ASSERT (source);
  Q_ASSERT (source->widget);

  auto target = findTile (pos);
  if (!target)
  {
    return;
  }

  if (event->dropAction () == Qt::MoveAction)
  {
    source->setWidget (target->widget);
    target->setWidget (sourceWidget);
    cleanupDimensions ();
    event->acceptProposedAction ();
  }
}

#include "tiledview.moc"
