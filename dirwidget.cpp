#include "dirwidget.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "copypaste.h"
#include "dirview.h"
#include "fileoperation.h"
#include "constants.h"
#include "openwith.h"
#include "notifier.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "debug.h"

#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QToolButton>
#include <QSettings>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QMessageBox>
#include <QLineEdit>
#include <QApplication>
#include <QClipboard>
#include <QProcess>

namespace
{
const QString qs_extensive = "extensive";
const QString qs_isList = "isList";
const QString qs_dir = "dir";
const QString qs_isLocked = "locked";
const QString qs_showDirs = "showDirs";
const QString qs_showHidden = "showHidden";
const QString qs_showThumbs = "showThumbs";
}

DirWidget::DirWidget (FileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  proxy_ (new ProxyModel (model, this)),
  view_ (new DirView (*proxy_, this)),
  path_ (),
  indexLabel_ (new QLabel (this)),
  pathLabel_ (new QLabel (this)),
  dirLabel_ (new QLabel (this)),
  pathEdit_ (new QLineEdit (this)),
  menu_ (new QMenu (this)),
  isLocked_ (nullptr),
  showDirs_ (nullptr),
  showHidden_ (nullptr),
  extensiveAction_ (nullptr),
  listMode_ (nullptr),
  showThumbs_ (nullptr),
  viewMenu_ (new QMenu (this)),
  openAction_ (nullptr),
  openWith_ (nullptr),
  openInEditorAction_ (nullptr),
  openInTabAction_ (nullptr),
  renameAction_ (nullptr),
  trashAction_ (nullptr),
  removeAction_ (nullptr),
  cutAction_ (nullptr),
  copyAction_ (nullptr),
  pasteAction_ (nullptr),
  copyPathAction_ (nullptr),
  copyToMenu_ (nullptr),
  moveToMenu_ (nullptr),
  linkToMenu_ (nullptr),
  upAction_ (nullptr),
  newFolderAction_ (nullptr),
  controlsLayout_ (new QHBoxLayout)
{
  proxy_->setDynamicSortFilter (true);

  connect (model, &QAbstractItemModel::rowsRemoved,
           this, &DirWidget::checkDirExistence);
  connect (model, &QFileSystemModel::fileRenamed,
           this, &DirWidget::handleDirRename);


  auto nextTab = makeShortcut (ShortcutManager::NextTab, nullptr);
  connect (nextTab, &QAction::triggered,
           this, [this] {emit nextTabRequested (this);});

  // menu
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, [this] {menu_->exec (QCursor::pos ());});

  auto openExternal = makeShortcut (ShortcutManager::OpenInExplorer, menu_);
  connect (openExternal, &QAction::triggered,
           this, [this] {QDesktopServices::openUrl (QUrl::fromLocalFile (
                                                      path ().absoluteFilePath ()));});

  auto editPath = makeShortcut (ShortcutManager::ChangePath, menu_);
  connect (editPath, &QAction::triggered,
           this, [this] {togglePathEdition (true);});

  auto openConsole = makeShortcut (ShortcutManager::OpenConsole, menu_);
  connect (openConsole, &QAction::triggered,
           this, [this] {emit consoleRequested (path_.absoluteFilePath ());});


  menu_->addSeparator ();


  isLocked_ = makeShortcut (ShortcutManager::LockTab, menu_, true);
  connect (isLocked_, &QAction::toggled,
           this, &DirWidget::setLocked);

  auto clone = makeShortcut (ShortcutManager::CloneTab, menu_);
  connect (clone, &QAction::triggered,
           this, [this]() {emit cloneRequested (this);});

  menu_->addSeparator ();

  auto close = makeShortcut (ShortcutManager::CloseTab, menu_);
  connect (close, &QAction::triggered,
           this, &DirWidget::promptClose);

  // view menu
  connect (viewMenu_, &QMenu::aboutToShow,
           this, &DirWidget::updateSiblingActions);
  openAction_ = makeShortcut (ShortcutManager::OpenItem, viewMenu_);
  connect (openAction_, &QAction::triggered,
           this, [this]() {openPath (view_->currentIndex ());});

  openWith_ = viewMenu_->addMenu (tr ("Open with"));

  openInTabAction_ = makeShortcut (ShortcutManager::OpenInTab, viewMenu_);
  connect (openInTabAction_, &QAction::triggered,
           this, [this]() {
    if (current ().isDir ())
    {
      emit newTabRequested (current ().absoluteFilePath ());
    }
  });

  openInEditorAction_ = makeShortcut (ShortcutManager::OpenInEditor, viewMenu_);
  connect (openInEditorAction_, &QAction::triggered,
           this, [this]() {
    if (current ().isFile ())
    {
      emit editorRequested (current ().absoluteFilePath ());
    }
  });

  cutAction_ =  makeShortcut (ShortcutManager::Cut, viewMenu_);
  connect (cutAction_, &QAction::triggered,
           this, &DirWidget::cut);

  copyAction_ = makeShortcut (ShortcutManager::Copy, viewMenu_);
  connect (copyAction_, &QAction::triggered,
           this, &DirWidget::copy);

  pasteAction_ = makeShortcut (ShortcutManager::Paste, viewMenu_);
  connect (pasteAction_, &QAction::triggered,
           this, &DirWidget::paste);

  copyPathAction_ = makeShortcut (ShortcutManager::CopyPath, viewMenu_);
  connect (copyPathAction_, &QAction::triggered,
           this, &DirWidget::copyPath);

  copyToMenu_ = viewMenu_->addMenu (tr ("Copy to..."));
  copyToMenu_->setIcon (ShortcutManager::icon (ShortcutManager::CopyTo));

  moveToMenu_ = viewMenu_->addMenu (tr ("Move to..."));
  moveToMenu_->setIcon (ShortcutManager::icon (ShortcutManager::MoveTo));

  linkToMenu_ = viewMenu_->addMenu (tr ("Link to..."));
  linkToMenu_->setIcon (ShortcutManager::icon (ShortcutManager::LinkTo));

  viewMenu_->addSeparator ();

  renameAction_ = makeShortcut (ShortcutManager::Rename, viewMenu_);
  connect (renameAction_, &QAction::triggered,
           view_, &DirView::renameCurrent);

  trashAction_ = makeShortcut (ShortcutManager::Trash, viewMenu_);
  connect (trashAction_, &QAction::triggered,
           this, &DirWidget::promptTrash);

  removeAction_ = makeShortcut (ShortcutManager::Remove, viewMenu_);
  connect (removeAction_, &QAction::triggered,
           this, &DirWidget::promptRemove);


  // controls
  auto viewMenuButton = new QToolButton (this);
  viewMenuButton->setPopupMode (QToolButton::InstantPopup);
  viewMenuButton->setIcon (QIcon (":/view.png"));
  auto representMenu = new QMenu (tr ("View"), viewMenuButton);
  viewMenuButton->setMenu (representMenu);

  showDirs_ = makeShortcut (ShortcutManager::ShowDirectories, representMenu, true);
  showDirs_->setChecked (proxy_->showDirs ());
  connect (showDirs_, &QAction::toggled,
           this, &DirWidget::setShowDirs);

  showHidden_ = makeShortcut (ShortcutManager::ShowHidden, representMenu, true);
  showHidden_->setChecked (proxy_->showHidden ());
  connect (showHidden_, &QAction::toggled,
           proxy_, &ProxyModel::setShowHidden);

  extensiveAction_ = makeShortcut (ShortcutManager::ExtensiveMode, representMenu, true);
  extensiveAction_->setChecked (view_->isExtensive ());
  connect (extensiveAction_, &QAction::toggled,
           view_, &DirView::setExtensive);

  listMode_ = makeShortcut (ShortcutManager::ListMode, representMenu, true);
  listMode_->setChecked (view_->isList ());
  connect (listMode_, &QAction::toggled,
           view_, &DirView::setIsList);

  showThumbs_ = makeShortcut (ShortcutManager::ShowThumbnails, representMenu, true);
  showThumbs_->setChecked (proxy_->showThumbnails ());
  connect (showThumbs_, &QAction::toggled,
           proxy_, &ProxyModel::setShowThumbnails);

  upAction_ = makeShortcut (ShortcutManager::MoveUp, nullptr, true);
  connect (upAction_, &QAction::triggered,
           this, [this] {openPath (view_->rootIndex ().parent ());});
  auto up = new QToolButton (this);
  up->setDefaultAction (upAction_);

  newFolderAction_ = makeShortcut (ShortcutManager::CreateFolder, nullptr);
  connect (newFolderAction_, &QAction::triggered,
           this, &DirWidget::newFolder);
  auto newFolder = new QToolButton (this);
  newFolder->setDefaultAction (newFolderAction_);


  pathLabel_->setAlignment (pathLabel_->alignment () | Qt::AlignRight);
  pathLabel_->installEventFilter (this);

  auto dirFont = dirLabel_->font ();
  dirFont.setBold (true);
  dirLabel_->setFont (dirFont);
  dirLabel_->installEventFilter (this);

  pathEdit_->installEventFilter (this);
  connect (pathEdit_, &QLineEdit::editingFinished,
           this, &DirWidget::handleEditedPath);

  togglePathEdition (false);

  controlsLayout_->addWidget (newFolder);
  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (indexLabel_);
  controlsLayout_->addWidget (pathLabel_);
  controlsLayout_->addWidget (dirLabel_);
  controlsLayout_->addWidget (pathEdit_);
  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (viewMenuButton);
  controlsLayout_->addWidget (up);

  controlsLayout_->setStretch (controlsLayout_->indexOf (pathEdit_), 40);


  // defaults
  auto layout = new QVBoxLayout (this);
  layout->setMargin (4);
  layout->setSpacing (2);
  layout->addLayout (controlsLayout_);
  layout->addWidget (view_);


  connect (view_, &DirView::contextMenuRequested,
           this, &DirWidget::showViewContextMenu);
  connect (view_, &DirView::activated,
           this, &DirWidget::openPath);
  connect (view_, &DirView::movedBackward,
           upAction_, &QAction::trigger);
  connect (view_, &DirView::backgroundActivated,
           this, &DirWidget::openInBackground);

  installEventFilter (this);
}

DirWidget::~DirWidget ()
{

}

void DirWidget::save (QSettings &settings) const
{
  settings.setValue (qs_isList, listMode_->isChecked ());
  settings.setValue (qs_dir, path ().absoluteFilePath ());
  settings.setValue (qs_isLocked, isLocked ());
  settings.setValue (qs_extensive, extensiveAction_->isChecked ());
  settings.setValue (qs_showDirs, proxy_->showDirs ());
  settings.setValue (qs_showHidden, proxy_->showHidden ());
  settings.setValue (qs_showThumbs, showThumbs_->isChecked ());
  view_->save (settings);
}

void DirWidget::restore (QSettings &settings)
{
  listMode_->setChecked (settings.value (qs_isList, isLocked ()).toBool ());
  setPath (settings.value (qs_dir).toString ());
  isLocked_->setChecked (settings.value (qs_isLocked, isLocked ()).toBool ());
  extensiveAction_->setChecked (settings.value (qs_extensive, false).toBool ());
  showDirs_->setChecked (settings.value (qs_showDirs, true).toBool ());
  showHidden_->setChecked (settings.value (qs_showHidden, false).toBool ());
  showThumbs_->setChecked (settings.value (qs_showThumbs, false).toBool ());
  view_->restore (settings);
}

QFileInfo DirWidget::path () const
{
  return path_;
}

void DirWidget::setSiblings (const QList<DirWidget *> siblings)
{
  using SM = ShortcutManager;
  auto handle = [this, siblings](SM::Shortcut s, QMenu *menu, Qt::DropAction drop) {
                  for (auto *i: menu->actions ())
                  {
                    removeAction (i);
                  }
                  menu->clear ();

                  const auto common = SM::get (s).toString ();
                  for (auto *i: siblings)
                  {
                    auto action = menu->addAction (i->indexLabel_->text ());
                    action->setData (QVariant::fromValue (i));
                    if (!common.isEmpty ())
                    {
                      action->setShortcut ({common + ',' + i->index ()});
                      action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
                      addAction (action);
                    }
                    connect (action, &QAction::triggered, this, [this, i, drop] {
        using utils::toUrls;
        emit fileOperation (FileOperation::paste (toUrls (selected ()), i->path (), drop));
      });
                  }
                };

  handle (SM::CopyTo, copyToMenu_, Qt::CopyAction);
  handle (SM::MoveTo, moveToMenu_, Qt::MoveAction);
  handle (SM::LinkTo, linkToMenu_, Qt::LinkAction);

  updateSiblingActions ();
}

void DirWidget::updateSiblingActions ()
{
  auto handle = [this](QMenu *menu) {
                  for (auto *i: menu->actions ())
                  {
                    auto *w = i->data ().value<DirWidget *>();
                    ASSERT (w);
                    const auto maxWidth = 100;
                    const auto text = w->indexLabel_->text () + ' ' +
                                      w->fittedPath (maxWidth) + w->dirLabel_->text ();
                    i->setText (text);
                  }
                };

  handle (copyToMenu_);
  handle (moveToMenu_);
  handle (linkToMenu_);
}

QString DirWidget::index () const
{
  auto index = indexLabel_->text ();
  if (!index.isEmpty ())
  {
    index = index.mid (1,index.length () - 2);
  }
  return index;
}

void DirWidget::setIndex (const QString &index)
{
  if (!index.isEmpty ())
  {
    indexLabel_->setText ('(' + index + ')');
  }
  else
  {
    indexLabel_->clear ();
  }
}

void DirWidget::setPath (const QFileInfo &path)
{
  openPath (proxy_->mapFromSource (model_->index (path.absoluteFilePath ())));
}

void DirWidget::setNameFilter (const QString &filter)
{
  proxy_->setNameFilter (QLatin1String ("*") + filter + QLatin1String ("*"));
}

void DirWidget::openPath (const QModelIndex &index)
{
  if (!index.isValid ()) // drives
  {
    // do not change proxy current path to disable possible dir filtering
    view_->setRootIndex ({});
    path_ = fileInfo (view_->rootIndex ());
    pathLabel_->clear ();
    dirLabel_->setText (tr ("Drives"));
    return;
  }

  const auto info = fileInfo (index);
  if (!info.isDir ())
  {
    if (info.permission (QFile::ExeUser))
    {
      if (!QProcess::startDetached (info.absoluteFilePath (), {}, info.absolutePath ()))
      {
        Notifier::error (tr ("Failed to open '%1'").arg (info.absoluteFilePath ()));
      }
    }
    else
    {
      QDesktopServices::openUrl (QUrl::fromLocalFile (info.absoluteFilePath ()));
    }
    return;
  }

  if (isLocked () || !info.permission (QFile::ExeUser))
  {
    return;
  }

  if (info.fileName () == constants::dotdot)
  {
    openPath (index.parent ().parent ());
  }
  else
  {
    auto newIndex = index.sibling (index.row (), FileSystemModel::Name);
    const auto previous = view_->rootIndex ();
    const auto moveUp = previous.parent () == newIndex;
    view_->setRootIndex (newIndex);
    view_->setCurrentIndex (moveUp ? previous : QModelIndex ());
    proxy_->setCurrent (newIndex);
    path_ = fileInfo (view_->rootIndex ());

    const auto absolutePath = info.absoluteFilePath ();
    auto nameIndex = absolutePath.lastIndexOf (QLatin1Char ('/')) + 1;
    dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);

    updatePathLabel ();
  }
}

QString DirWidget::fittedPath (int maxWidth) const
{
  auto path = this->path ().absoluteFilePath ();
  const auto nameIndex = path.lastIndexOf (QLatin1Char ('/')) + 1;
  if (!nameIndex)
  {
    return {};
  }

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

void DirWidget::newFolder ()
{
  QDir dir (path ().absoluteFilePath ());
  auto name = tr ("New");
  auto i = 0;
  while (dir.exists (name))
  {
    name = tr ("New") + QString::number (++i);
  }

  auto index = model_->mkdir (proxy_->mapToSource (view_->rootIndex ()), name);
  view_->setCurrentIndex (proxy_->mapFromSource (index));
  view_->renameCurrent ();
}

QAction * DirWidget::makeShortcut (int shortcutType, QMenu *menu,bool isCheckable)
{
  auto action = new QAction (this);
  ShortcutManager::add (ShortcutManager::Shortcut (shortcutType), action);
  action->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  addAction (action);
  if (isCheckable)
  {
    action->setCheckable (true);
  }
  if (menu)
  {
    menu->addAction (action);
  }
  return action;
}

void DirWidget::resizeEvent (QResizeEvent *)
{
  updatePathLabel ();
}

bool DirWidget::eventFilter (QObject *watched, QEvent *event)
{
  if (watched == this && event->type () == QEvent::MouseButtonPress)
  {
    auto casted = static_cast<QMouseEvent *>(event);
    if (casted->button () == Qt::LeftButton)
    {
      activate ();
    }
    else if (casted->button () == Qt::MiddleButton)
    {
      emit cloneRequested (this);
    }
    return true;
  }

  if (isLocked ())
  {
    return false;
  }
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

void DirWidget::activate ()
{
  view_->activate ();
}

void DirWidget::adjustItems ()
{
  view_->adjustItems ();
}

void DirWidget::togglePathEdition (bool isOn)
{
  pathEdit_->setVisible (isOn);
  indexLabel_->setVisible (!isOn);
  pathLabel_->setVisible (!isOn);
  dirLabel_->setVisible (!isOn);

  const auto path = this->path ().absoluteFilePath ();
  if (isOn)
  {
    pathEdit_->setText (path);
    pathEdit_->setFocus ();
    pathEdit_->selectAll ();
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

void DirWidget::handleEditedPath ()
{
  if (QFile::exists (pathEdit_->text ()))
  {
    togglePathEdition (false);
  }
}

void DirWidget::promptClose ()
{
  auto res = QMessageBox::question (this, {}, tr ("Close tab \"%1\"?")
                                    .arg (path ().absoluteFilePath ()),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    emit closeRequested (this);
  }
}

void DirWidget::promptTrash ()
{
  const auto indexes = view_->selectedRows ();
  if (indexes.isEmpty ())
  {
    return;
  }
  auto res = QMessageBox::question (this, {}, tr ("Move files \"%1\" to trash?")
                                    .arg (names (indexes).join (QLatin1String ("\", \""))),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    QList<QFileInfo> infos;
    for (const auto &i: indexes)
    {
      infos << model_->fileInfo (proxy_->mapToSource (i));
    }
    emit fileOperation (FileOperation::trash (infos));
  }
}

void DirWidget::promptRemove ()
{
  const auto indexes = view_->selectedRows ();
  if (indexes.isEmpty ())
  {
    return;
  }
  auto res = QMessageBox::question (this, {}, tr ("Remove \"%1\" permanently?")
                                    .arg (names (indexes).join (QLatin1String ("\", \""))),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    QList<QFileInfo> infos;
    for (const auto &i: indexes)
    {
      infos << model_->fileInfo (proxy_->mapToSource (i));
    }
    emit fileOperation (FileOperation::remove (infos));
  }
}

QList<QFileInfo> DirWidget::selected () const
{
  QList<QFileInfo> infos;
  for (const auto &i: view_->selectedRows ())
  {
    infos << fileInfo (i);
  }
  return infos;
}

QFileInfo DirWidget::current () const
{
  return fileInfo (view_->currentIndex ());
}

QFileInfo DirWidget::fileInfo (const QModelIndex &index) const
{
  if (!index.isValid ())
  {
    return {};
  }

  auto result = (index.model () == model_)
                ? model_->fileInfo (index)
                : model_->fileInfo (proxy_->mapToSource (index));

#ifdef Q_OS_WIN
  if (result.isDir ())
  {
    const auto path = result.filePath ();
    if (path.endsWith (QLatin1Char (':')) && path.length () == 2)
    {
      result = QFileInfo (path + QLatin1Char ('/'));
    }
  }
#endif

  return result;
}

QStringList DirWidget::names (const QList<QModelIndex> &indexes) const
{
  QStringList names;
  for (const auto &i: indexes)
  {
    names << i.data ().toString ();
  }
  return names;
}

void DirWidget::cut ()
{
  CopyPaste::cut (selected ());
}

void DirWidget::copy ()
{
  CopyPaste::copy (selected ());
}

void DirWidget::paste ()
{
  auto target = current ();
  if (!target.exists () || !target.isDir ())
  {
    target = path ();
  }
  const auto urls = CopyPaste::clipboardUrls ();
  const auto action = CopyPaste::clipboardAction ();
  emit fileOperation (FileOperation::paste (urls, target, action));
}

void DirWidget::copyPath ()
{
  const auto info = fileInfo (view_->currentIndex ());
  QApplication::clipboard ()->setText (info.absoluteFilePath ());
}

void DirWidget::openInBackground (const QModelIndex &index)
{
  const auto info = fileInfo (index);
  if (!info.exists ())
  {
    return;
  }
  if (info.isDir ())
  {
    emit newTabRequested (info);
  }
  else
  {
    openPath (index);
  }
}

void DirWidget::showViewContextMenu ()
{
  const auto index = view_->currentIndex ();
  const auto isDotDot = index.isValid () && index.data () == constants::dotdot;
  const auto isDir = current ().isDir ();
  openAction_->setEnabled (index.isValid ());
  openWith_->setEnabled (index.isValid () && !isDir);
  if (openWith_->isEnabled ())
  {
    OpenWith::popupateMenu (*openWith_, current ());
  }
  copyPathAction_->setEnabled (index.isValid ());
  openInTabAction_->setEnabled (isDir);
  renameAction_->setEnabled (index.isValid () && !isLocked () && !isDotDot);
  removeAction_->setEnabled (index.isValid () && !isLocked () && !isDotDot);
  trashAction_->setEnabled (index.isValid () && !isLocked () && !isDotDot);

  viewMenu_->exec (QCursor::pos ());
}

bool DirWidget::isLocked () const
{
  return isLocked_->isChecked ();
}

void DirWidget::setLocked (bool isLocked)
{
  view_->setLocked (isLocked);
  updateActions ();
}

void DirWidget::setShowDirs (bool on)
{
  proxy_->setShowDirs (on);
  updateActions ();
}

void DirWidget::updateActions ()
{
  const auto dirs = showDirs_->isChecked ();
  const auto lock = isLocked_->isChecked ();
  newFolderAction_->setEnabled (dirs && !lock);
  upAction_->setEnabled (!lock);

  pasteAction_->setEnabled (!lock);
  cutAction_->setEnabled (!lock);
}

void DirWidget::updatePathLabel ()
{
  const auto stretchWidth = controlsLayout_->itemAt (1)->geometry ().width ();
  const auto maxWidth = pathLabel_->width () + 2 * stretchWidth - 10; // 10 just for sure
  const auto newPath = fittedPath (maxWidth);
  if (newPath != pathLabel_->text ())
  {
    pathLabel_->setText (newPath);
  }
}

void DirWidget::checkDirExistence ()
{
  const auto path = this->path ().absoluteFilePath ();
  if (QFileInfo::exists (path))
  {
    return;
  }

  auto parts = path.split ('/');
  if (parts.isEmpty ())
  {
    setPath (QDir::homePath ());
    return;
  }

  parts.pop_back ();
  auto newPath = parts.join ('/');
  while (!parts.isEmpty () && !QFileInfo::exists (newPath))
  {
    parts.pop_back ();
    newPath = parts.join ('/');
  }
  setPath (newPath);
}

void DirWidget::handleDirRename (const QString &path, const QString &old, const QString &now)
{
  if (this->path ().absoluteFilePath () == (path + '/' + old))
  {
    setPath (path + '/' + now);
  }
}
