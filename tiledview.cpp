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
const QString qs_rows = "rows";
const QString qs_cols = "cols";
const QString qs_tiles = "tiles";

int square (const QSize &size)
{
  return size.width () * size.height ();
}

}

TiledView::TiledView (QWidget *parent) :
  QSplitter (parent)
{
  setAcceptDrops (true);
  setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
}

TiledView::~TiledView ()
{
}

void TiledView::add (QWidget &widget)
{
  if (count () < 2)
  {
    setOrientation (height () > width () ? Qt::Vertical : Qt::Horizontal);
    addWidget (&widget);
    if (count () == 2)
    {
      const auto size = max (height (), width ()) / 2;
      setSizes ({size, size});
    }
  }
  else
  {
    auto *place = biggest ();
    ASSERT (place);
    auto *parent = cast (place->parentWidget ());
    ASSERT (parent);
    if (parent != this)
    {
      parent->add (widget);
      return;
    }

    const auto placeIndex = indexOf (place);
    ASSERT (placeIndex != -1);
    auto *replacement = new TiledView;
    replacement->resize (place->size ());
    replacement->add (*place);
    replacement->add (widget);
    insertWidget (placeIndex, replacement);
  }
}

void TiledView::remove (QWidget &widget)
{
  auto *parent = cast (widget.parentWidget ());
  if (parent != this)
  {
    parent->remove (widget);
    return;
  }

  ASSERT (!cast (&widget));
  if (auto selfParent = cast (parentWidget ()))
  {
    const auto selfIndex = selfParent->indexOf (this);
    if (count () == 2)
    {
      const auto leftIndex = 1 - indexOf (&widget);
      selfParent->insertWidget (selfIndex, this->widget (leftIndex));
    }
    deleteLater ();
  }
  else
  {
    widget.setParent (nullptr);
  }
}

QWidget * TiledView::biggest () const
{
  auto items = widgets ();
  if (items.isEmpty ())
  {
    return nullptr;
  }
  return *max_element (cbegin (items), cend (items), [](const QWidget *l, const QWidget *r) {
    return square (l->size ()) < square (r->size ());
  });
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

void TiledView::save (QSettings &settings) const
{

}

void TiledView::restore (QSettings &settings)
{

}

TiledView * TiledView::cast (QWidget *widget) const
{
  return qobject_cast<TiledView *>(widget);
}
