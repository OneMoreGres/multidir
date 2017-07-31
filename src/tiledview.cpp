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

#include <array>


using namespace std;
using namespace nonstd;

namespace
{
const QString qs_state = "state";
const QString qs_tile = "tile";
const QString qs_name = "name";
const QString qs_tiles = "tiles";

int square (const QSize &size)
{
  return size.width () * size.height ();
}

}

//! Class for drag-n-drop support.
class TileMime : public QMimeData
{
Q_OBJECT
public:
  TileMime (QWidget *widget);
  ~TileMime ();

  QWidget *widget;
  QWidget *zoneWidget;
};

TileMime::TileMime (QWidget *widget) :
  widget (widget),
  zoneWidget (new QWidget)
{
  zoneWidget->setWindowFlags (Qt::Tool | Qt::FramelessWindowHint
                              | Qt::WindowTransparentForInput
                              | Qt::BypassWindowManagerHint);
  zoneWidget->setWindowOpacity (0.2);
  zoneWidget->setAutoFillBackground (true);
  zoneWidget->setStyleSheet ("QWidget {background: #0099cc}");
}

TileMime::~TileMime ()
{
  zoneWidget->deleteLater ();
}



TiledView::TiledView (QWidget *parent) :
  QSplitter (parent),
  dragStartPos_ ()
{
  setAcceptDrops (true);
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  setChildrenCollapsible (false);
}

TiledView::~TiledView ()
{
}

void TiledView::add (QWidget &widget)
{
  if (count () < 2)
  {
    const auto zone = (height () > width () ? Zone::Bottom : Zone::Right);
    insert (&widget, zone, nullptr);
  }
  else
  {
    auto *place = biggest ();
    ASSERT (place);
    auto *parent = cast (place->parentWidget ());
    ASSERT (parent);
    const auto zone = (place->height () > place->width () ? Zone::Bottom : Zone::Right);
    parent->insert (&widget, zone, place);
  }
}

void TiledView::remove (QWidget &widget)
{
  if (auto *parent = cast (widget.parentWidget ()))
  {
    parent->take (&widget);
  }
}

void TiledView::take (QWidget *widget)
{
  ASSERT (widget->parentWidget () == this);
  ASSERT (!cast (widget));

  auto parent = cast (parentWidget ());
  QList<int> sizes;
  if (parent)
  {
    sizes = parent->sizes ();
  }

  widget->setParent (nullptr);

  if (parent)
  {
    const auto selfIndex = parent->indexOf (this);
    setParent (nullptr);
    if (count () == 1)
    {
      parent->insertWidget (selfIndex, this->widget (0));
      parent->setSizes (sizes);
    }
    deleteLater ();
  }
}

QWidget * TiledView::biggest () const
{
  auto items = widgets ();
  if (items.isEmpty ())
  {
    return nullptr;
  }
  return *max_element (nonstd::cbegin (items), nonstd::cend (items),
                       [](const QWidget *l, const QWidget *r) {
                         return square (l->size () - l->minimumSize ()) < square (r->size () - r->minimumSize ());
                       });
}

QWidget * TiledView::childAt (const QPoint &pos) const
{
  for (auto i = 0, end = count (); i < end; ++i)
  {
    auto w = widget (i);
    if (w->geometry ().contains (pos))
    {
      if (cast (w))
      {
        return nullptr;
      }
      return w;
    }
  }
  return nullptr;
}

QList<QWidget *> TiledView::widgets () const
{
  QList<QWidget *> result;
  for (auto i = 0, end = count (); i < end; ++i)
  {
    auto w = widget (i);
    if (const auto tiled = cast (w))
    {
      result += tiled->widgets ();
    }
    else
    {
      result << w;
    }
  }
  return result;
}

TiledView * TiledView::cast (QWidget *widget) const
{
  return qobject_cast<TiledView *>(widget);
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
  if (dragStartPos_.isNull () ||
      (event->pos () - dragStartPos_).manhattanLength () < QApplication::startDragDistance ())
  {
    return;
  }

  auto *item = childAt (dragStartPos_);
  if (!item)
  {
    dragStartPos_ = {};
    return;
  }

  auto *drag = new QDrag (this);
  drag->setPixmap (item->grab ());

  auto *mime = new TileMime (item);
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

void TiledView::dragMoveEvent (QDragMoveEvent *event)
{
  auto *mime = qobject_cast<const TileMime *>(event->mimeData ());
  if (!mime)
  {
    return;
  }
  ASSERT (mime->widget);
  auto target = childAt (event->pos ());
  if (!target || target == mime->widget)
  {
    mime->zoneWidget->hide ();
    return;
  }

  const auto zone = dropZone (target->mapFromParent (event->pos ()), target->size ());
  if (zone == Zone::None)
  {
    mime->zoneWidget->hide ();
    return;
  }

  else if (zone == Zone::Center)
  {
    mime->zoneWidget->setGeometry ({mapToGlobal (target->pos ()), target->size ()});
  }
  else
  {
    const auto orientation = (zone == Zone::Left || zone == Zone::Right
                              ? Qt::Horizontal : Qt::Vertical);
    const QSize s ((orientation == Qt::Vertical) ? target->width () : target->width () / 3,
                   (orientation == Qt::Vertical) ? target->height () / 3 : target->height ());
    mime->zoneWidget->resize (s);
    const auto x = (zone == Zone::Right ? s.width () * 2 : 0);
    const auto y = (zone == Zone::Bottom ? s.height () * 2 : 0);
    mime->zoneWidget->move (target->mapToGlobal ({x, y}));
  }
  mime->zoneWidget->show ();
  event->acceptProposedAction ();
}

void TiledView::dropEvent (QDropEvent *event)
{
  auto *mime = qobject_cast<const TileMime *>(event->mimeData ());
  if (!mime)
  {
    return;
  }
  mime->zoneWidget->hide ();
  ASSERT (mime->widget);
  auto target = childAt (event->pos ());
  if (!target || target == mime->widget)
  {
    return;
  }

  const auto zone = dropZone (target->mapFromParent (event->pos ()), target->size ());
  if (zone == Zone::None)
  {
    return;
  }

  if (zone == Zone::Center)
  {
    swap (mime->widget, target);
  }
  else
  {
    insert (mime->widget, zone, target);
  }
  event->acceptProposedAction ();
  emit tileSwapped ();
}

TiledView::Zone TiledView::dropZone (const QPoint &pos, const QSize &size) const
{
  const auto xThreshold = size.width () / 3 + 1; // +1 for boundary case
  const auto xPart = pos.x () / xThreshold;
  const auto yThreshold = size.height () / 3 + 1;
  const auto yPart = pos.y () / yThreshold;
  const array<Zone, 9> zones {{Zone::None, Zone::Top, Zone::None,
                               Zone::Left, Zone::Center, Zone::Right,
                               Zone::None, Zone::Bottom, Zone::None}};
  const auto index = xPart + 3 * yPart;
  ASSERT (index < 9);
  return zones[index];
}

void TiledView::swap (QWidget *first, QWidget *second)
{
  auto firstParent = cast (first->parentWidget ());
  ASSERT (firstParent);
  const auto firstSizes = firstParent->sizes ();
  const auto firstIndex = firstParent->indexOf (first);

  auto secondParent = cast (second->parentWidget ());
  ASSERT (secondParent);
  const auto secondSizes = secondParent->sizes ();
  const auto secondIndex = secondParent->indexOf (second);

  second->resize (first->size ());
  firstParent->insertWidget (firstIndex, second);
  secondParent->insertWidget (secondIndex, first);
  firstParent->setSizes (firstSizes);
  secondParent->setSizes (secondSizes);
}

void TiledView::insert (QWidget *widget, TiledView::Zone zone, QWidget *target)
{
  ASSERT (widget);
  ASSERT (zone != Zone::None);
  ASSERT (zone != Zone::Center);

  const auto newIndex = (zone == Zone::Left || zone == Zone::Top ? 0 : 1);
  const auto orientation = (zone == Zone::Left || zone == Zone::Right
                            ? Qt::Horizontal : Qt::Vertical);

  if (widget->parentWidget () == this)
  {
    widget->setParent (nullptr);
  }
  else
  {
    remove (*widget);
  }

  if (count () < 2)
  {
    setOrientation (orientation);
    insertWidget (newIndex, widget);
  }
  else
  {
    ASSERT (target);
    const auto targetIndex = indexOf (target);
    ASSERT (targetIndex != -1);
    auto *targetReplacement = new TiledView;
    connect (targetReplacement, &TiledView::tileSwapped, this, &TiledView::tileSwapped);
    targetReplacement->setOrientation (orientation);

    const auto state = sizes ();
    targetReplacement->insertWidget (newIndex, widget);
    targetReplacement->insertWidget (1 - newIndex, target);
    insertWidget (targetIndex, targetReplacement);
    setSizes (state);
  }
}

void TiledView::save (QSettings &settings) const
{
  settings.setValue (qs_state, saveState ());
  settings.beginWriteArray (qs_tiles, count ());
  for (auto i = 0, end = count (); i < end; ++i)
  {
    settings.setArrayIndex (i);
    auto w = widget (i);
    if (auto tile = cast (w))
    {
      settings.setValue (qs_name, "");
      tile->save (settings);
    }
    else
    {
      settings.setValue (qs_name, w->objectName ());
    }
  }
  settings.endArray ();
}

void TiledView::restore (QSettings &settings)
{
  QHash<QString, QWidget *> widgetByName;
  for (auto *i: widgets ())
  {
    ASSERT (!i->objectName ().isEmpty ());
    ASSERT (!widgetByName.contains (i->objectName ()));
    widgetByName.insert (i->objectName (), i);
    i->setParent (nullptr);
  }

  for (auto *i: findChildren<TiledView *>())
  {
    i->setParent (nullptr);
    i->deleteLater ();
  }

  restoreImpl (settings, widgetByName);

  for (auto *i: widgetByName)
  {
    add (*i);
  }
}

void TiledView::restoreImpl (QSettings &settings, QHash<QString, QWidget *> &widgetByName)
{
  auto count = settings.beginReadArray (qs_tiles);
  for (auto i = 0; i < count && i < 2; ++i)
  {
    settings.setArrayIndex (i);
    const auto name = settings.value (qs_name).toString ();
    if (name.isEmpty ())
    {
      auto tile = new TiledView (this);
      connect (tile, &TiledView::tileSwapped, this, &TiledView::tileSwapped);
      tile->restoreImpl (settings, widgetByName);
    }
    else if (auto *w = widgetByName.take (name))
    {
      add (*w);
    }
  }
  settings.endArray ();

  if (settings.contains (qs_state))
  {
    restoreState (settings.value (qs_state).toByteArray ());
    setChildrenCollapsible (false);
  }
}

#include "tiledview.moc"
