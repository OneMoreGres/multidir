#include "dirwidget.h"
#include "proxymodel.h"
#include "filesystemmodel.h"

#include <QTableView>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QToolButton>
#include <QHeaderView>
#include <QSettings>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QMessageBox>
#include <QLineEdit>
#include <QListView>

#include <QDebug>

namespace
{
const QString qs_mode = "mode";
const QString qs_dir = "dir";
const QString qs_isLocked = "locked";
const QString qs_view = "view";
const QString qs_showDirs = "showDirs";
}

DirWidget::DirWidget (FileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  menu_ (new QMenu (this)),
  model_ (model),
  proxy_ (new ProxyModel (model, this)),
  tableView_ (nullptr),
  listView_ (nullptr),
  viewMenu_ (new QMenu (this)),
  openAction_ (nullptr),
  renameAction_ (nullptr),
  removeAction_ (nullptr),
  pathLabel_ (new QLabel (this)),
  dirLabel_ (new QLabel (this)),
  pathEdit_ (new QLineEdit (this)),
  isLocked_ (nullptr),
  up_ (new QToolButton (this)),
  showDirs_ (nullptr),
  listMode_ (nullptr),
  controlsLayout_ (new QHBoxLayout)
{
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, [this] {menu_->exec (QCursor::pos ());});

  auto openExternal = menu_->addAction (QIcon (":/openExternal.png"), tr ("Open in explorer"));
  connect (openExternal, &QAction::triggered,
           this, [this] {QDesktopServices::openUrl (QUrl::fromLocalFile (path ()));});

  auto editPath = menu_->addAction (QIcon (":/rename.png"), tr ("Change path"));
  connect (editPath, &QAction::triggered,
           this, [this] {togglePathEdition (true);});

  menu_->addSeparator ();

  isLocked_ = menu_->addAction (QIcon (":/lockTab.png"), tr ("Lock"));
  isLocked_->setCheckable (true);
  connect (isLocked_, &QAction::toggled,
           this, &DirWidget::setIsLocked);

  showDirs_ = menu_->addAction (QIcon (":/folder.png"), tr ("Show directories"));
  showDirs_->setCheckable (true);
  showDirs_->setChecked (proxy_->showDirs ());
  connect (showDirs_, &QAction::toggled,
           this, &DirWidget::toggleShowDirs);


  listMode_ = menu_->addAction (QIcon (":/listMode.png"), tr ("List mode"));
  listMode_->setCheckable (true);
  listMode_->setChecked (false);
  connect (listMode_, &QAction::toggled,
           this, [this](bool on) {setViewMode (on ? ViewMode::List : ViewMode::Table);});

  menu_->addSeparator ();

  auto clone = menu_->addAction (QIcon (":/cloneTab.png"), tr ("Clone"));
  connect (clone, &QAction::triggered,
           this, [this]() {emit cloneRequested (this);});

  menu_->addSeparator ();

  auto close = menu_->addAction (QIcon (":/closeTab.png"), tr ("Close..."));
  connect (close, &QAction::triggered,
           this, &DirWidget::promptClose);


  proxy_->setDynamicSortFilter (true);

  openAction_ = viewMenu_->addAction (tr ("Open"));
  connect (openAction_, &QAction::triggered,
           this, [this]() {openPath (view ()->currentIndex ());});

  renameAction_ = viewMenu_->addAction (QIcon (":/rename.png"), tr ("Rename"));
  connect (renameAction_, &QAction::triggered,
           this, &DirWidget::startRenaming);

  removeAction_ = viewMenu_->addAction (QIcon (":/remove.png"), tr ("Remove..."));
  connect (removeAction_, &QAction::triggered,
           this, &DirWidget::promptRemove);


  up_->setIcon (QIcon (":/up.png"));
  up_->setToolTip (tr ("Move up"));
  connect (up_, &QToolButton::pressed,
           this, &DirWidget::moveUp);


  pathLabel_->setAlignment (pathLabel_->alignment () | Qt::AlignRight);
  pathLabel_->installEventFilter (this);

  auto dirFont = dirLabel_->font ();
  dirFont.setBold (true);
  dirLabel_->setFont (dirFont);
  dirLabel_->installEventFilter (this);

  pathEdit_->installEventFilter (this);
  connect (pathEdit_, &QLineEdit::editingFinished,
           this, [this] {togglePathEdition (false);});

  togglePathEdition (false);

  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (pathLabel_);
  controlsLayout_->addWidget (dirLabel_);
  controlsLayout_->addWidget (pathEdit_);
  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (up_);

  controlsLayout_->setStretch (controlsLayout_->indexOf (pathEdit_), 40);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (controlsLayout_);

  setViewMode (ViewMode::Table);
  setIsLocked (false);
}

DirWidget::~DirWidget ()
{

}

void DirWidget::save (QSettings &settings) const
{
  settings.setValue (qs_mode, int(viewMode ()));
  settings.setValue (qs_dir, path ());
  settings.setValue (qs_isLocked, isLocked ());
  if (tableView_)
  {
    settings.setValue (qs_view, tableView_->horizontalHeader ()->saveState ());
  }
  settings.setValue (qs_showDirs, proxy_->showDirs ());
}

void DirWidget::restore (QSettings &settings)
{
  setViewMode (ViewMode (settings.value (qs_mode, int (ViewMode::Table)).toInt ()));
  setPath (settings.value (qs_dir).toString ());
  isLocked_->setChecked (settings.value (qs_isLocked, isLocked ()).toBool ());
  if (tableView_ && settings.contains (qs_view))
  {
    tableView_->horizontalHeader ()->restoreState (settings.value (qs_view).toByteArray ());
  }
  showDirs_->setChecked (settings.value (qs_showDirs, true).toBool ());
}

void DirWidget::setPath (const QString &path)
{
  auto absolutePath = QDir (path).absolutePath ();
  auto index = proxy_->mapFromSource (model_->index (absolutePath));
  if (!index.isValid ())
  {
    return;
  }
  view ()->setRootIndex (index);
  proxy_->setCurrent (index);

  pathLabel_->setText (fittedPath ());
  auto nameIndex = absolutePath.lastIndexOf (QLatin1Char ('/')) + 1;
  dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);
}

QString DirWidget::path () const
{
  return model_->filePath (proxy_->mapToSource (view ()->rootIndex ()));
}

void DirWidget::setNameFilter (const QString &filter)
{
  proxy_->setNameFilter (QLatin1String ("*") + filter + QLatin1String ("*"));
}

void DirWidget::setIsExtensiveView (bool isExtensive)
{
  if (tableView_)
  {
    const auto margins = (isExtensive ? 14 : 4);
    tableView_->verticalHeader ()->setDefaultSectionSize (tableView_->fontMetrics ().height () + margins);
  }
}

void DirWidget::setIsLocked (bool isLocked)
{
  up_->setEnabled (!isLocked);
  using View = QAbstractItemView;
  view ()->setEditTriggers (isLocked ? View::NoEditTriggers : View::SelectedClicked);
  view ()->setDragDropMode (isLocked ? View::NoDragDrop : View::DragDrop);
}

void DirWidget::openPath (const QModelIndex &index)
{
  if (!index.isValid ())
  {
    return;
  }

  const auto mapped = proxy_->mapToSource (index);
  auto path = model_->filePath (mapped);
  if (!model_->isDir (mapped))
  {
    QDesktopServices::openUrl (QUrl::fromLocalFile (path));
  }
  else
  {
    if (!isLocked () && model_->permissions (mapped) & QFile::ExeUser)
    {
      setPath (path);
    }
  }
}

bool DirWidget::isLocked () const
{
  return isLocked_->isChecked ();
}

void DirWidget::moveUp ()
{
  QDir dir (path ());
  if (dir.cdUp ())
  {
    setPath (dir.absolutePath ());
  }
}

void DirWidget::toggleShowDirs (bool show)
{
  proxy_->setShowDirs (show);
}

void DirWidget::showViewContextMenu ()
{
  const auto index = view ()->currentIndex ();
  const auto isDotDot = index.isValid () && index.data () == QLatin1String ("..");
  openAction_->setEnabled (index.isValid ());
  renameAction_->setEnabled (index.isValid () && !isLocked () && !isDotDot);
  removeAction_->setEnabled (index.isValid () && !isLocked () && !isDotDot);

  viewMenu_->exec (QCursor::pos ());
}

void DirWidget::showHeaderContextMenu ()
{
  QMenu menu;
  auto header = tableView_->horizontalHeader ();
  for (auto i = 0, end = proxy_->columnCount (); i < end; ++i)
  {
    auto action = menu.addAction (proxy_->headerData (i, Qt::Horizontal).toString ());
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

void DirWidget::resizeEvent (QResizeEvent */*event*/)
{
  const auto newText = fittedPath ();
  if (newText != pathLabel_->text ())
  {
    pathLabel_->setText (newText);
  }
}

QString DirWidget::fittedPath () const
{
  auto path = this->path ();
  const auto nameIndex = path.lastIndexOf (QLatin1Char ('/')) + 1;
  if (!nameIndex)
  {
    return {};
  }

  const auto stretchWidth = controlsLayout_->itemAt (0)->geometry ().width ();
  const auto maxWidth = pathLabel_->width () + 2 * stretchWidth - 10; // 10 just for sure
  const QString prepend = QLatin1String (".../");
  const auto searchStartIndex = prepend.length ();

  QFontMetrics metrics (pathLabel_->font ());
  path = path.left (nameIndex);
  auto width = metrics.boundingRect (path).width ();

  while (width > maxWidth)
  {
    auto index = path.indexOf (QLatin1Char ('/'), searchStartIndex);
    if (index == -1)
    {
      break;
    }
    path = prepend + path.mid (index + 1);
    width = metrics.boundingRect (path).width ();
  }
  return path;
}

void DirWidget::startRenaming ()
{
  const auto index = view ()->currentIndex ();
  const auto nameIndex = index.sibling (index.row (), FileSystemModel::Column::Name);
  view ()->setCurrentIndex (nameIndex);
  view ()->edit (nameIndex);
}

void DirWidget::promptClose ()
{
  auto res = QMessageBox::question (this, {}, tr ("Close tab \"%1\"?").arg (path ()),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    emit closeRequested (this);
  }
}

void DirWidget::promptRemove ()
{
  const auto indexes = view ()->selectionModel ()->selectedRows (FileSystemModel::Column::Name);
  if (indexes.isEmpty ())
  {
    return;
  }
  QStringList names;
  for (const auto &i: indexes)
  {
    names << i.data ().toString ();
  }
  auto res = QMessageBox::question (this, {}, tr ("Remove \"%1\" permanently?")
                                    .arg (names.join (QLatin1String ("\", \""))),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    for (const auto &i: indexes)
    {
      model_->remove (proxy_->mapToSource (i));
    }
  }
}

bool DirWidget::eventFilter (QObject *watched, QEvent *event)
{
  if (event->type () == QEvent::MouseButtonDblClick &&
      (watched == pathLabel_ || watched == dirLabel_))
  {
    togglePathEdition (true);
    return true;
  }
  if (event->type () == QEvent::KeyPress && watched == pathEdit_)
  {
    if (static_cast<QKeyEvent *>(event)->key () == Qt::Key_Escape)
    {
      if (pathEdit_->isModified ())
      {
        togglePathEdition (true); // reset changes
      }
      pathEdit_->hide (); // trigger editionFinished
      return true;
    }
  }
  return false;
}

void DirWidget::togglePathEdition (bool isOn)
{
  pathLabel_->setVisible (!isOn);
  dirLabel_->setVisible (!isOn);
  pathEdit_->setVisible (isOn);

  const auto path = pathLabel_->text () + dirLabel_->text ();
  if (isOn)
  {
    pathEdit_->setText (path);
    pathEdit_->setFocus ();
  }
  else
  {
    const auto newPath = pathEdit_->text ();
    if (newPath != path && QFile::exists (newPath))
    {
      setPath (newPath);
    }
  }
}

DirWidget::ViewMode DirWidget::viewMode () const
{
  return (tableView_ ? ViewMode::Table : ViewMode::List);
}

void DirWidget::setViewMode (ViewMode mode)
{
  auto path = view () ? this->path () : QDir::homePath ();
  switch (mode)
  {
    case ViewMode::List:
      if (tableView_)
      {
        tableView_->deleteLater ();
        tableView_ = nullptr;
      }
      if (!listView_)
      {
        listView_ = new QListView (this);

        listView_->setModel (proxy_);
        listView_->setFlow (QListView::LeftToRight);
        listView_->setWrapping (true);
        listView_->setViewMode (QListView::IconMode);
        listView_->setIconSize ({32,32});
        listView_->setResizeMode (QListView::Adjust);
        listView_->setGridSize ({80,80});
        listView_->setMovement (QListView::Snap);

        listView_->setDragDropOverwriteMode (false);
        listView_->setDefaultDropAction (Qt::MoveAction);
        listView_->setContextMenuPolicy (Qt::CustomContextMenu);
        connect (listView_, &QWidget::customContextMenuRequested,
                 this, &DirWidget::showViewContextMenu);

        layout ()->addWidget (listView_);
      }
      break;

    default:
      if (listView_)
      {
        listView_->deleteLater ();
        listView_ = nullptr;
      }
      if (!tableView_)
      {
        tableView_ = new QTableView (this);

        tableView_->setModel (proxy_);
        tableView_->setSortingEnabled (true);
        tableView_->setSelectionBehavior (QAbstractItemView::SelectRows);
        tableView_->setDragDropOverwriteMode (false);
        tableView_->setDefaultDropAction (Qt::MoveAction);
        tableView_->setContextMenuPolicy (Qt::CustomContextMenu);
        connect (tableView_, &QTableView::doubleClicked,
                 this, &DirWidget::openPath);
        connect (tableView_, &QWidget::customContextMenuRequested,
                 this, &DirWidget::showViewContextMenu);


        tableView_->horizontalHeader ()->setContextMenuPolicy (Qt::CustomContextMenu);
        connect (tableView_->horizontalHeader (), &QWidget::customContextMenuRequested,
                 this, &DirWidget::showHeaderContextMenu);

        layout ()->addWidget (tableView_);
      }
      break;
  }

  setIsLocked (isLocked ());
  setPath (path);
}

QAbstractItemView * DirWidget::view () const
{
  return (tableView_
          ? static_cast<QAbstractItemView *>(tableView_)
          : static_cast<QAbstractItemView *>(listView_) );
}
