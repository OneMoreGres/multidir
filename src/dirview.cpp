#include "dirview.h"
#include "filesystemmodel.h"
#include "constants.h"
#include "filedelegate.h"
#include "debug.h"
#include "filepermissiondelegate.h"

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
  list_ (nullptr)
{
  setLayout (new QVBoxLayout);
  layout ()->setMargin (0);
  setIsList (isList_);
  connect (&model, &QAbstractItemModel::rowsInserted,
           this, &DirView::selectFirst);
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
    w->deleteLater ();
    w = nullptr;
  }
}
}

void DirView::setIsList (bool isList)
{
  auto root = view () ? rootIndex () : QModelIndex ();

  isList_ = isList;
  if (!isList_)
  {
    cleanup (list_);

    if (!table_)
    {
      initTable ();
    }
  }
  else
  {
    cleanup (table_);

    if (!list_)
    {
      initList ();
    }
    if (!delegate_)
    {
      delegate_ = new FileDelegate (this);
    }
    list_->setItemDelegate (delegate_);
  }

  setRootIndex (root);
  setLocked (isLocked ());
  setExtensive (isExtensive ());
}

void DirView::initTable ()
{
  table_ = new QTableView (this);
  table_->setModel (model_);

  table_->setSortingEnabled (true);
  table_->setSelectionBehavior (QAbstractItemView::SelectRows);
  table_->setDragDropOverwriteMode (false);
  table_->setDefaultDropAction (Qt::MoveAction);
  table_->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (table_, &QTableView::activated,
           this, &DirView::activated);
  connect (table_, &QWidget::customContextMenuRequested,
           this, &DirView::contextMenuRequested);

  connect (table_->selectionModel (), &QItemSelectionModel::currentChanged,
           this, &DirView::currentChanged);
  connect (table_->selectionModel (), &QItemSelectionModel::selectionChanged,
           this, &DirView::selectionChanged);

  table_->horizontalHeader ()->setSectionsMovable (true);
  table_->horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (table_->horizontalHeader (), &QWidget::customContextMenuRequested,
           this, &DirView::showHeaderContextMenu);

  table_->setItemDelegateForColumn (FileSystemModel::Permissions,
                                    new FilePermissionDelegate (table_));

  table_->installEventFilter (this);
  table_->viewport ()->installEventFilter (this);

  layout ()->addWidget (table_);
}

void DirView::initList ()
{
  list_ = new QListView (this);
  list_->setModel (model_);

  list_->setWrapping (true);
  list_->setResizeMode (QListView::Adjust);
  list_->setViewMode (QListView::IconMode);
  list_->setMovement (QListView::Snap);
  list_->setUniformItemSizes (true);
  list_->setSelectionMode (QListView::ExtendedSelection);

  list_->setDragDropOverwriteMode (false);
  list_->setDefaultDropAction (Qt::MoveAction);
  list_->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (list_, &QListView::activated,
           this, &DirView::activated);
  connect (list_, &QWidget::customContextMenuRequested,
           this, &DirView::contextMenuRequested);

  connect (list_->selectionModel (), &QItemSelectionModel::currentChanged,
           this, &DirView::currentChanged);
  connect (list_->selectionModel (), &QItemSelectionModel::selectionChanged,
           this, &DirView::selectionChanged);

  list_->installEventFilter (this);
  list_->viewport ()->installEventFilter (this);

  layout ()->addWidget (list_);
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
  return (table_
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
    if (casted->key () == Qt::Key_Backspace ||
        (watched == table_ && casted->key () == Qt::Key_Left))
    {
      emit movedBackward ();
    }
    else if (casted->key () == Qt::Key_Space ||
             (watched == table_ && casted->key () == Qt::Key_Right))
    {
      const auto index = currentIndex ();
      if (index.isValid ())
      {
        emit activated (index);
      }
    }
    else if (casted->key () == Qt::Key_Home)
    {
      const auto rows = model_->rowCount (rootIndex ());
      if (rows > 0)
      {
        setCurrentIndex (model_->index (0, 0, rootIndex ()));
      }
      return true;
    }
    else if (casted->key () == Qt::Key_End)
    {
      const auto rows = model_->rowCount (rootIndex ());
      if (rows > 0)
      {
        setCurrentIndex (model_->index (rows - 1, 0, rootIndex ()));
      }
      return true;
    }
  }
  else if (event->type () == QEvent::FocusIn)
  {
    auto effect = new QGraphicsDropShadowEffect ();
    effect->setOffset (0.);
    effect->setBlurRadius (10.0);
    effect->setColor (QColor (0,153,204));
    setGraphicsEffect (effect);
  }
  else if (event->type () == QEvent::FocusOut)
  {
    setGraphicsEffect (nullptr);
  }
  return false;
}

#include "moc_dirview.cpp"
