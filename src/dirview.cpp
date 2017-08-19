#include "dirview.h"
#include "filesystemmodel.h"
#include "constants.h"
#include "filedelegate.h"
#include "debug.h"
#include "filepermissiondelegate.h"
#include "styleoptionsproxy.h"

#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QBoxLayout>
#include <QSettings>
#include <QMenu>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>


namespace
{
const QString qs_view = "header";
}


DirView::DirView (QAbstractItemModel &model, QWidget *parent) :
  QWidget (parent),
  isList_ (false),
  isLocked_ (false),
  isExtensive_ (false),
  delegate_ (nullptr),
  model_ (&model),
  table_ (nullptr),
  list_ (nullptr),
  glowColor_ ()
{
  setLayout (new QVBoxLayout);
  layout ()->setMargin (0);
  setIsList (isList_);
  connect (&model, &QAbstractItemModel::rowsInserted,
           this, &DirView::selectFirst);

  connect (&StyleOptionsProxy::instance (), &StyleOptionsProxy::changed,
           this, &DirView::updateStyle);
  updateStyle ();
}

void DirView::save (QSettings &settings) const
{
  if (table_)
  {
    settings.setValue (qs_view, table_->horizontalHeader ()->saveState ());
  }
}

void DirView::restore (QSettings &settings)
{
  if (table_ && settings.contains (qs_view))
  {
    table_->horizontalHeader ()->restoreState (settings.value (qs_view).toByteArray ());
  }
}

void DirView::activate ()
{
  view ()->setFocus ();
}

void DirView::adjustItems ()
{
  if (table_)
  {
    table_->resizeColumnsToContents ();
  }
}

QModelIndex DirView::firstItem () const
{
  return model_->rowCount (rootIndex ()) > 0 ? model_->index (0,0, rootIndex ())
                                             : QModelIndex ();
}

QModelIndex DirView::currentIndex () const
{
  return view ()->currentIndex ();
}

void DirView::setCurrentIndex (const QModelIndex &index)
{
  view ()->clearSelection ();
  view ()->setCurrentIndex (index);
}

QModelIndex DirView::rootIndex () const
{
  return view ()->rootIndex ();
}

void DirView::setRootIndex (const QModelIndex &index)
{
  view ()->setRootIndex (index);
}

QModelIndexList DirView::selectedRows () const
{
  auto selection = view ()->selectionModel ();
  auto items = (table_
                ? selection->selectedRows (FileSystemModel::Column::Name)
                : selection->selectedIndexes ());
  for (const auto &i: items)
  {
    if (i.data () == constants::dotdot)
    {
      items.removeOne (i);
    }
  }
  return items;
}

void DirView::renameCurrent ()
{
  const auto index = currentIndex ();
  const auto nameIndex = index.sibling (index.row (), FileSystemModel::Column::Name);
  setCurrentIndex (nameIndex);
  view ()->edit (nameIndex);
}

void DirView::changeCurrentPermissions ()
{
  const auto index = currentIndex ();
  const auto nameIndex = index.sibling (index.row (), FileSystemModel::Column::Permissions);
  setCurrentIndex (nameIndex);
  view ()->edit (nameIndex);
}

bool DirView::isList () const
{
  return isList_;
}

namespace
{
template<class T>
void cleanup (T * &w)
{
  if (w)
  {
    delete w;
    w = nullptr;
  }
}
}

void DirView::setIsList (bool isList)
{
  if (isList_ == isList && (table_ || list_))
  {
    return;
  }

  const auto root = view () ? rootIndex () : QModelIndex ();

  isList_ = isList;
  if (!isList_)
  {
    cleanup (list_);
    initTable ();
  }
  else
  {
    cleanup (table_);
    initList ();
  }

  auto selection = view ()->selectionModel ();
  connect (selection, &QItemSelectionModel::currentChanged,
           this, &DirView::currentChanged);
  connect (selection, &QItemSelectionModel::selectionChanged,
           this, &DirView::selectionChanged);

  auto *view = this->view ();
  view->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (view, &QWidget::customContextMenuRequested,
           this, &DirView::contextMenuRequested);
  connect (view, &QAbstractItemView::activated,
           this, &DirView::activated);

  view->setDragDropOverwriteMode (false);
  view->setDefaultDropAction (Qt::MoveAction);

  view->installEventFilter (this);
  view->viewport ()->installEventFilter (this);

  layout ()->addWidget (view);

  setRootIndex (root);
  setLocked (isLocked ());
  setExtensive (isExtensive ());

  activate ();
}

void DirView::initTable ()
{
  if (table_)
  {
    return;
  }

  table_ = new QTableView (this);
  table_->setObjectName ("table");
  table_->setModel (model_);

  table_->setSortingEnabled (true);
  table_->setSelectionBehavior (QAbstractItemView::SelectRows);

  table_->horizontalHeader ()->setSectionsMovable (true);
  table_->horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (table_->horizontalHeader (), &QWidget::customContextMenuRequested,
           this, &DirView::showHeaderContextMenu);

  table_->setItemDelegateForColumn (FileSystemModel::Permissions,
                                    new FilePermissionDelegate (table_));
}

void DirView::initList ()
{
  if (list_)
  {
    return;
  }

  list_ = new QListView (this);
  list_->setObjectName ("list");
  list_->setModel (model_);

  list_->setWrapping (true);
  list_->setResizeMode (QListView::Adjust);
  list_->setViewMode (QListView::IconMode);
  list_->setMovement (QListView::Snap);
  list_->setUniformItemSizes (true);
  list_->setSelectionMode (QListView::ExtendedSelection);

  if (!delegate_)
  {
    delegate_ = new FileDelegate (this);
  }

  list_->setItemDelegate (delegate_);
}

void DirView::setGlowColor (const QColor &color)
{
  glowColor_ = color;
  if (auto effect = qobject_cast<QGraphicsDropShadowEffect *>(graphicsEffect ()))
  {
    effect->setColor (glowColor_);
  }
}

bool DirView::isLocked () const
{
  return isLocked_;
}

void DirView::setLocked (bool isLocked)
{
  isLocked_ = isLocked;
  using View = QAbstractItemView;
  view ()->setEditTriggers (isLocked ? View::NoEditTriggers : View::SelectedClicked);
  view ()->setDragDropMode (isLocked ? View::NoDragDrop : View::DragDrop);
}

bool DirView::isExtensive () const
{
  return isExtensive_;
}

void DirView::setExtensive (bool isExtensive)
{
  isExtensive_ = isExtensive;

  if (table_)
  {
    const auto margins = (isExtensive ? 2 : 1) * constants::itemVerticalMargins;
    const auto fontHeight = view ()->fontMetrics ().height ();
    table_->verticalHeader ()->setDefaultSectionSize (fontHeight + margins);
  }
  else
  {
    const auto iconSize = (isExtensive ? constants::iconSize : constants::iconMinSize);
    list_->setIconSize ({iconSize, iconSize});
  }
}

void DirView::showHeaderContextMenu ()
{
  QMenu menu;
  auto header = table_->horizontalHeader ();
  for (auto i = 0, end = model_->columnCount (); i < end; ++i)
  {
    auto action = menu.addAction (model_->headerData (i, Qt::Horizontal).toString ());
    action->setCheckable (true);
    action->setChecked (!header->isSectionHidden (i));
    action->setData (i);
  }

  auto choice = menu.exec (QCursor::pos ());
  if (choice)
  {
    header->setSectionHidden (choice->data ().toInt (), !choice->isChecked ());
  }

}

QAbstractItemView * DirView::view () const
{
  return (!isList_
          ? static_cast<QAbstractItemView *>(table_)
          : static_cast<QAbstractItemView *>(list_) );
}

void DirView::selectFirst ()
{
  if (!currentIndex ().isValid ())
  {
    setCurrentIndex (firstItem ());
  }
}

bool DirView::eventFilter (QObject *watched, QEvent *event)
{
  if (event->type () == QEvent::MouseButtonRelease)
  {
    auto casted = static_cast<QMouseEvent *>(event);
    if (casted->button () == Qt::MiddleButton && casted->buttons () == Qt::NoButton)
    {
      emit backgroundActivated (currentIndex ());
      return true;
    }
  }
  else if (event->type () == QEvent::KeyPress)
  {
    auto casted = static_cast<QKeyEvent *>(event);
    const auto key = casted->key ();
    const auto modifiers = casted->modifiers ();

    if (modifiers == Qt::NoModifier)
    {
      if (key == Qt::Key_Backspace || (watched == table_ && key == Qt::Key_Left))
      {
        emit movedBackward ();
        return true;
      }

      if (key == Qt::Key_Space || (watched == table_ && key == Qt::Key_Right))
      {
        const auto index = currentIndex ();
        if (index.isValid ())
        {
          emit activated (index);
        }
        return true;
      }

      if (key == Qt::Key_Home)
      {
        const auto rows = model_->rowCount (rootIndex ());
        if (rows > 0)
        {
          setCurrentIndex (model_->index (0, 0, rootIndex ()));
        }
        return true;
      }

      if (key == Qt::Key_End)
      {
        const auto rows = model_->rowCount (rootIndex ());
        if (rows > 0)
        {
          setCurrentIndex (model_->index (rows - 1, 0, rootIndex ()));
        }
        return true;
      }
    }
  }
  else if (event->type () == QEvent::FocusIn)
  {
    auto effect = new QGraphicsDropShadowEffect (this);
    effect->setOffset (0.);
    effect->setBlurRadius (20.0);
    effect->setColor (QColor (0,153,204));
    effect->setColor (glowColor_);
    setGraphicsEffect (effect);
  }
  else if (event->type () == QEvent::FocusOut)
  {
    setGraphicsEffect (nullptr);
  }
  return false;
}

void DirView::updateStyle ()
{
  const auto &options = StyleOptionsProxy::instance ();
  setGlowColor (options.activeGlowColor ());
}

#include "moc_dirview.cpp"
