#include "dirview.h"
#include "filesystemmodel.h"

#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QBoxLayout>
#include <QSettings>
#include <QMenu>

#include <QDebug>


namespace
{
const QString qs_view = "header";
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

DirView::DirView (QAbstractItemModel &model, QWidget *parent) :
  QWidget (parent),
  isList_ (false),
  isLocked_ (false),
  isExtensive_ (false),
  model_ (&model),
  table_ (nullptr),
  list_ (nullptr)
{
  setLayout (new QVBoxLayout);
  setIsList (isList_);
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
  return view ()->selectionModel ()->selectedRows (FileSystemModel::Column::Name);
}

void DirView::edit (const QModelIndex &index)
{
  view ()->edit (index);
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
  connect (table_, &QTableView::doubleClicked,
           this, &DirView::doubleClicked);
  connect (table_, &QWidget::customContextMenuRequested,
           this, &DirView::contextMenuRequested);

  table_->horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (table_->horizontalHeader (), &QWidget::customContextMenuRequested,
           this, &DirView::showHeaderContextMenu);

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

  list_->setDragDropOverwriteMode (false);
  list_->setDefaultDropAction (Qt::MoveAction);
  list_->setContextMenuPolicy (Qt::CustomContextMenu);
  connect (list_, &QListView::doubleClicked,
           this, &DirView::doubleClicked);
  connect (list_, &QWidget::customContextMenuRequested,
           this, &DirView::contextMenuRequested);

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

  const auto fontHeight = view ()->fontMetrics ().height ();
  if (table_)
  {
    const auto margins = (isExtensive ? 14 : 4);
    table_->verticalHeader ()->setDefaultSectionSize (fontHeight + margins);
  }
  else
  {
    const auto iconSize = (isExtensive ? 64 : 16);
    const auto margins = 4;
    const auto width = 120;
    list_->setIconSize ({iconSize, iconSize});
    list_->setGridSize ({width + margins, iconSize + fontHeight + margins});
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
