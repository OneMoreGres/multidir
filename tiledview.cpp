#include "tiledview.h"
#include "backport.h"
#include "debug.h"

#include <QResizeEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QLayout>
#include <QSettings>
#include <QFrame>


using namespace std;
using namespace nonstd;

namespace
{
const auto minTileSize = 100;
const QString qs_rows = "rows";
const QString qs_cols = "cols";
const QString qs_tiles = "tiles";
}

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
  Tile (QWidget *widget, int row, int col);

  QWidget *widget = nullptr;
  QRect geometry = {};
  int row = -1;
  int col = -1;
  int rowSpan = 1;
  int colSpan = 1;
  QMap<Border, QFrame *> borders = {};

  void setWidget (QWidget *widget);
  void setGeometry (const QRect &rect, int spacing);
  void createBorder (Border border, QWidget *parent);
  void removeBorder (Border border);
  void removeBorders ();
  Border borderAt (const QPoint &pos) const;
  bool operator< (const Tile &r) const { return tie (row, col) < tie (r.row,r.col); }
  bool operator== (const Tile &r) const { return tie (row, col) == tie (r.row,r.col); }
  bool occupies (int row, int col) const
  {
    return this->row <= row && this->row + rowSpan - 1 >= row
           && this->col <= col && this->col + colSpan - 1 >= col;
  }
};

Tile::Tile (QWidget *widget, int row, int col) :
  widget (widget),
  row (row),
  col (col)
{

}

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
    borders[Border::Right]->setGeometry (rect.right () + 1, rect.top (), spacing, rect.height ());
  }
  if (borders.contains (Border::Bottom))
  {
    borders[Border::Bottom]->setGeometry (rect.left (), rect.bottom (), rect.width (), spacing);
  }
  if (borders.contains (Border::BottomRight))
  {
    borders[Border::BottomRight]->setGeometry (rect.right () + 1, rect.bottom (), spacing, spacing);
  }
}

void Tile::createBorder (Border border, QWidget *parent)
{
  if (borders.contains (border))
  {
    return;
  }

  auto w = new QFrame (parent);
  w->setCursor (QMap<Border,Qt::CursorShape>
                {{Border::Right, Qt::SizeHorCursor},{Border::Bottom, Qt::SizeVerCursor},
                 {Border::BottomRight, Qt::SizeAllCursor}}
                .value (border, Qt::ArrowCursor));
  borders[border] = w;
  w->setFrameShape (QMap<Border,QFrame::Shape>
                    {{Border::Right, QFrame::VLine},{Border::Bottom, QFrame::HLine}}
                    .value (border, QFrame::NoFrame));
  w->setFrameShadow (QFrame::Sunken);
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
  spacing_ (10),
  margin_ (10),
  dragStartPos_ (),
  resizeIndex_ (-1),
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

Tile * TiledView::findTile (int row, int col) const
{
  auto it = find_if (cbegin (tiles_), cend (tiles_), [row, col](const Tile &i) {
    return i.occupies (row, col);
  });
  if (it == cend (tiles_))
  {
    return nullptr;
  }
  return &const_cast<Tile &>(*it);
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
    if (i.geometry.contains (pos))
    {
      return true;
    }
    auto border = i.borderAt (pos);
    return border != Border::None;
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

void TiledView::add (int row, int col)
{
  tiles_.append ({nullptr, row, col});
}

QWidget * TiledView::remove (int row, int col)
{
  auto *occupied = findTile (row, col);
  Q_ASSERT (occupied);
  for (auto i = 0; i < occupied->rowSpan; ++i)
  {
    for (auto j = 0; j < occupied->colSpan; ++j)
    {
      const auto newRow = occupied->row + i;
      const auto newCol = occupied->col + j;
      if ((newRow == row && newCol == col) || (i == 0 && j == 0))
      {
        continue;
      }
      add (newRow, newCol);
    }
  }

  QWidget *widget = nullptr;
  if (occupied->row == row && occupied->col == col)
  {
    widget = occupied->widget;
    occupied->removeBorders ();
    tiles_.removeOne (*occupied);
  }
  else
  {
    occupied->rowSpan = occupied->colSpan = 1;
  }
  return widget;
}

void TiledView::split (Tile &tile)
{
  for (auto i = 0; i < tile.rowSpan; ++i)
  {
    for (auto j = 0; j < tile.colSpan; ++j)
    {
      if (i == 0 && j == 0)
      {
        continue;
      }
      const auto newRow = tile.row + i;
      const auto newCol = tile.col + j;
      add (newRow, newCol);
    }
  }
  tile.rowSpan = tile.colSpan = 1;
}

void TiledView::shift (bool isRow, int change, int start, int end)
{
  int Tile::*field = isRow ? &Tile::row : &Tile::col;
  for (auto &i: tiles_)
  {
    if (i.*field >= start && (end == -1 || i.*field < end))
    {
      i.*field += change;
    }
  }
}

void TiledView::addRow (AddMode mode)
{
  addDimesion (true, mode);
}

void TiledView::addColumn (AddMode mode)
{
  addDimesion (false, mode);
}

void TiledView::addDimesion (bool isRow, AddMode mode)
{
  auto &sizes = this->sizes (isRow);
  const auto &opposite = this->sizes (!isRow);
  const auto fullSize = isRow ? height () : width ();
  const auto isPrepend = mode == AddMode::Prepend;
  const auto index = isPrepend ? 0 : sizes.size ();

  if (isPrepend)
  {
    shift (isRow, 1, 0);
  }
  for (auto i = 0, end = opposite.size (); i < end; ++i)
  {
    add ((isRow ? index : i), (isRow ? i : index));
  }

  sort (begin (tiles_), end (tiles_));

  const auto size = max (0, (fullSize - 2 * margin_ - index * spacing_) / (sizes.size () + 1));
  adjustSizes (sizes, -size - spacing_);
  sizes.insert (index, size);

  updateTilesBorders ();
}

void TiledView::removeRow (int index)
{
  removeDimesion (true, index);
}

void TiledView::removeColumn (int index)
{
  removeDimesion (false, index);
}

void TiledView::removeDimesion (bool isRow, int index)
{
  auto &sizes = isRow ? rows_ : columns_;
  auto &opposite = isRow ? columns_ : rows_;

  for (auto i = 0, end = opposite.size (); i < end; ++i)
  {
    remove ((isRow ? index : i), (isRow ? i : index));
  }

  shift (isRow, -1, index);

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

  for (auto &i: tiles_)
  {
    auto columnIt = cbegin (columns_) + i.col;
    auto left = margin_ + spacing_ * i.col + accumulate (cbegin (columns_), columnIt, 0);
    auto width = spacing_ * (i.colSpan - 1) + accumulate (columnIt, columnIt + i.colSpan, 0);

    auto rowIt = cbegin (rows_) + i.row;
    auto top = margin_ + spacing_ * i.row + accumulate (cbegin (rows_), rowIt, 0);
    auto height = spacing_ * (i.rowSpan - 1) + accumulate (rowIt, rowIt + i.rowSpan, 0);

    i.setGeometry ({left, top, width, height}, spacing_);
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
  ASSERT (tile);
  tile->setWidget (widget);
}

void TiledView::remove (QWidget &widget)
{
  auto tile = findTile (&widget);
  ASSERT (tile);
  split (*tile);
  tile->widget = nullptr;
  cleanupDimensions ();
  updateTilesGeometry ();
  updateGeometry ();
}

QList<QWidget *> TiledView::widgets () const
{
  QList<QWidget *> result;
  for (const auto &i: tiles_)
  {
    if (i.widget)
    {
      result << i.widget;
    }
  }
  return result;
}

void TiledView::save (QSettings &settings) const
{
  const auto toString = [](const QList<int> &list) {
                          QString result = "";
                          for (auto i: list)
                          {
                            result += ',' + QString::number (i);
                          }
                          return result.mid (1);
                        };

  settings.setValue (qs_rows, toString (rows_));
  settings.setValue (qs_cols, toString (columns_));

  QStringList tiles;
  for (const auto &i: as_const (tiles_))
  {
    tiles << QString ("%1:%2:%3").arg (i.widget ? i.widget->objectName () : QString ())
      .arg (i.rowSpan).arg (i.colSpan);
  }
  settings.setValue (qs_tiles, tiles.join (','));
}

void TiledView::restore (QSettings &settings)
{
  const auto toInts = [](const QString &str) {
                        QList<int> result;
                        for (auto i: str.split (','))
                        {
                          auto ok = true;
                          result << i.toInt (&ok);
                          if (!ok)
                          {
                            return QList<int>();
                          }
                        }
                        return result;
                      };

  const auto rows = toInts (settings.value (qs_rows).toString ());
  const auto cols = toInts (settings.value (qs_cols).toString ());
  if (rows.isEmpty () || cols.isEmpty ())
  {
    return;
  }


  QMap<QString, QWidget *> widgets;
  for (const auto &i: tiles_)
  {
    if (i.widget)
    {
      widgets[i.widget->objectName ()] = i.widget;
    }
  }


  for (auto i: rows_)
  {
    Q_UNUSED (i);
    removeRow (0);
  }
  columns_.clear ();


  for (auto i: rows)
  {
    Q_UNUSED (i);
    addRow ();
  }
  rows_ = rows;

  for (auto i: cols)
  {
    Q_UNUSED (i);
    addColumn ();
  }
  columns_ = cols;


  const auto tiles = settings.value (qs_tiles).toString ().split (',');
  auto index = -1;
  for (const auto &i: tiles)
  {
    auto parts = i.split (':');
    if (parts.size () != 3)
    {
      break;
    }
    ++index;
    if (index >= tiles_.size ())
    {
      break;
    }
    const auto &name = parts[0];
    auto &tile = tiles_[index];
    tile.setWidget (!name.isEmpty () ? widgets.take (name) : nullptr);
    const auto &rowSpan = parts[1];
    tile.rowSpan = max (1, rowSpan.toInt ());
    const auto &colSpan = parts[2];
    tile.colSpan = max (1, colSpan.toInt ());

    for (auto r = 0; r < tile.rowSpan; ++r)
    {
      for (auto c = 0; c < tile.colSpan; ++c)
      {
        if (r == 0 && c == 0)
        {
          continue;
        }
        auto smashedIndex = tiles_.indexOf ({nullptr, tile.row + r, tile.col + c});
        if (smashedIndex != -1)
        {
          auto smashed = tiles_.takeAt (smashedIndex);
          smashed.removeBorders ();
        }
      }
    }
  }

  for (auto *i: as_const (widgets))
  {
    add (*i);
  }

  updateTilesGeometry ();
  updateTilesBorders ();
  updateGeometry ();
}

void TiledView::cleanupDimensions ()
{
  QSet<int> rows, cols;
  for (const auto &i: tiles_)
  {
    if (i.widget)
    {
      for (auto r = 0; r < i.rowSpan; ++r)
      {
        for (auto c = 0; c < i.colSpan; ++c)
        {
          rows << i.row + r;
          cols << i.col + c;
        }
      }
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
    const auto hint = i.widget->sizeHint ();
    heights[i.col] += hint.height () + spacing_;
    widths[i.row] += hint.width () + spacing_;
  }
  return QSize (*max_element (cbegin (widths), cend (widths)) - spacing_ + 2 * margin_,
                *max_element (cbegin (heights), cend (heights)) - spacing_ + 2 * margin_);
}

QSize TiledView::minimumSizeHint () const
{
  if (tiles_.isEmpty ())
  {
    return QSize (2 * margin_, 2 * margin_);
  }
  return QSize ((minTileSize  + spacing_) * columns_.size () - spacing_ + 2 * margin_,
                (minTileSize  + spacing_) * rows_.size () - spacing_ + 2 * margin_);
}

void TiledView::startResize (int index, Qt::Orientations dir)
{
  resizeIndex_ = index;
  resizeDir_ = dir;
}

void TiledView::handleResizing (const QPoint &current)
{
  ASSERT (resizeIndex_ >= 0);
  ASSERT (resizeIndex_ < tiles_.size ());
  const auto diff = current - dragStartPos_;
  const auto &tile = tiles_[resizeIndex_];

  if (resizeDir_ & Qt::Horizontal)
  {
    const auto col = tile.col + tile.colSpan - 1;
    resizeDimension (false, col, diff.x ());
  }

  if (resizeDir_ & Qt::Vertical)
  {
    const auto row = tile.row + tile.rowSpan - 1;
    resizeDimension (true, row, diff.y ());
  }

  updateTilesGeometry ();
  dragStartPos_ = current;
}

void TiledView::resizeDimension (bool isRow, int index, int diff)
{
  auto &sizes = this->sizes (isRow);
  if (index < sizes.size () - 1)
  {
    const auto change = clamp (diff, -sizes[index] + minTileSize,
                               sizes[index + 1] - minTileSize);
    sizes[index] += change;
    sizes[index + 1] -= change;
  }
}

void TiledView::handleSpanning (const QPoint &current)
{
  ASSERT (resizeIndex_ >= 0);
  ASSERT (resizeIndex_ < tiles_.size ());
  auto &tile = tiles_[resizeIndex_];
  if (!tile.widget)
  {
    return;
  }
  const auto diff = current - dragStartPos_;
  auto changed = false;

  if (resizeDir_ & Qt::Horizontal)
  {
    changed = spanTile (tile, diff, false);
  }

  if (resizeDir_ & Qt::Vertical)
  {
    changed = spanTile (tile, diff, true);
  }

  if (changed)
  {
    cleanupDimensions ();
    updateTilesBorders ();
    updateTilesGeometry ();
  }
}

bool TiledView::spanTile (Tile &tile, const QPoint &diff, bool isRow)
{
  auto &sizes = (isRow ? rows_ : columns_);
  auto &span = (isRow ? tile.rowSpan : tile.colSpan);
  const auto otherSpan = (isRow ? tile.colSpan : tile.rowSpan);
  const auto index = (isRow ? tile.row : tile.col) + span - 1;
  const auto otherIndex = (isRow ? tile.col : tile.row);
  auto &position = (isRow ? dragStartPos_.ry () : dragStartPos_.rx ());
  const auto change = (isRow ? diff.y () : diff.x ());

  auto changed = false;
  if (index < sizes.size () - 1 && change >= sizes[index + 1] / 2 + spacing_)
  {
    for (auto i = 0; i < otherSpan; ++i)
    {
      const auto r = (isRow ? index + 1 : otherIndex + i);
      const auto c = (isRow ? otherIndex + i : index + 1);
      auto *tile = findTile (r,c);
      ASSERT (tile);
      if (tile->widget)
      {
        return false;
      }
    }

    for (auto i = 0; i < otherSpan; ++i)
    {
      const auto r = (isRow ? index + 1 : otherIndex + i);
      const auto c = (isRow ? otherIndex + i : index + 1);
      auto *widget = remove (r, c);
      ASSERT (!widget);
    }
    position += sizes[index + 1] + spacing_;
    changed = true;
    ++span;
  }
  else if (span > 1 && change < -sizes[index] / 2 - spacing_)
  {
    --span;
    for (auto i = 0; i < otherSpan; ++i)
    {
      add ((isRow ? index : otherIndex + i), (isRow ? otherIndex + i : index));
    }
    sort (begin (tiles_), end (tiles_));
    position -= sizes[index] + spacing_;
    changed = true;
  }

  return changed;
}

QList<int> &TiledView::sizes (bool isRow)
{
  return isRow ? rows_ : columns_;
}

void TiledView::mousePressEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = event->pos ();

    if (auto borders = findTileBorders (event->pos ()))
    {
      auto type = borders->borderAt (event->pos ());
      auto dir = QMap<Border,Qt::Orientations>
      {{Border::Right, Qt::Horizontal},{Border::Bottom, Qt::Vertical},
       {Border::BottomRight, Qt::Horizontal | Qt::Vertical}}
      .value (type, 0);
      startResize (tiles_.indexOf (*borders), dir);
    }
    event->accept ();
  }
}

void TiledView::mouseReleaseEvent (QMouseEvent *event)
{
  if (event->button () == Qt::LeftButton)
  {
    dragStartPos_ = {};
    startResize (-1, 0);
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
    if (event->modifiers () &= Qt::ControlModifier)
    {
      handleSpanning (event->pos ());
    }
    else
    {
      handleResizing (event->pos ());
    }
    event->accept ();
    return;
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
  ASSERT (source);
  ASSERT (source->widget);

  auto pos = event->pos ();
  auto sourceWidget = source->widget;
  auto changed = false;
  if (pos.x () < margin_ || width () - pos.x () < margin_)
  {
    const auto prepend = pos.x () < margin_;
    addColumn (prepend ? AddMode::Prepend : AddMode::Append);
    changed = true;
    pos.rx () += (prepend ? 1 : -1) * margin_;
  }
  if (pos.y () < margin_ || height () - pos.y () < margin_)
  {
    const auto prepend = pos.y () < margin_;
    addRow (prepend ? AddMode::Prepend : AddMode::Append);
    changed = true;
    pos.ry () += (prepend ? 1 : -1) * margin_;
  }
  if (changed)
  {
    source = findTile (sourceWidget); // addRow can alter it
    updateTilesGeometry ();
  }
  ASSERT (source);
  ASSERT (source->widget);

  auto target = findTile (pos);
  if (!target)
  {
    return;
  }

  if (event->dropAction () == Qt::MoveAction)
  {
    if (!target->widget)
    {
      split (*source);
      updateTilesBorders ();
      updateTilesGeometry ();
    }
    source->setWidget (target->widget);
    target->setWidget (sourceWidget);
    cleanupDimensions ();
    event->acceptProposedAction ();
    updateGeometry ();
    emit tileSwapped ();
  }
}

#include "tiledview.moc"
