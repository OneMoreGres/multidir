#include "dirwidget.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "copypaste.h"
#include "dirview.h"
#include "fileoperation.h"
#include "openwith.h"
#include "notifier.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "debug.h"
#include "propertieswidget.h"
#include "pathwidget.h"
#include "fileviewer.h"
#include "dirstatuswidget.h"
#include "settingsmanager.h"
#include "shellcommand.h"
#include "navigationhistory.h"

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
#include <QRegularExpression>

namespace
{
const QString qs_extensive = "extensive";
const QString qs_isList = "isList";
const QString qs_dir = "dir";
const QString qs_isLocked = "locked";
const QString qs_showDirs = "showDirs";
const QString qs_showHidden = "showHidden";
const QString qs_showThumbs = "showThumbs";
const QString qs_minSize = "minSize";
}

DirWidget::DirWidget (FileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  proxy_ (new ProxyModel (model, this)),
  view_ (new DirView (*proxy_, this)),
  index_ (),
  path_ (),
  lastCurrentFile_ (),
  pathWidget_ (new PathWidget (model, this)),
  status_ (new DirStatusWidget (proxy_, this)),
  commandPrompt_ (new QLineEdit (this)),
  openConsoleCommand_ (),
  runInConsoleCommand_ (),
  editorCommand_ (),
  siblings_ (),
  navigationHistory_ (new NavigationHistory (this)),
  menu_ (new QMenu (this)),
  isLocked_ (nullptr),
  showDirs_ (nullptr),
  showHidden_ (nullptr),
  extensiveAction_ (nullptr),
  listMode_ (nullptr),
  showThumbs_ (nullptr),
  isMinSizeFixed_ (nullptr),
  viewMenu_ (new QMenu (this)),
  openAction_ (nullptr),
  openWith_ (nullptr),
  viewAction_ (nullptr),
  openInEditorAction_ (nullptr),
  openInTabAction_ (nullptr),
  renameAction_ (nullptr),
  permissionsAction_ (nullptr),
  trashAction_ (nullptr),
  removeAction_ (nullptr),
  showPropertiesAction_ (nullptr),
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

  connect (model_, &QAbstractItemModel::rowsRemoved,
           this, &DirWidget::checkDirExistence);
  connect (model_, &QFileSystemModel::fileRenamed,
           this, &DirWidget::handleDirRename);
  connect (model_, &QFileSystemModel::fileRenamed,
           this, &DirWidget::handleFileRename);
  connect (this, &DirWidget::fileOperation,
           model_, &FileSystemModel::fileOperation);


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

  auto openConsole = makeShortcut (ShortcutManager::OpenConsole, menu_);
  connect (openConsole, &QAction::triggered,
           this, &DirWidget::openConsole);

  auto editPath = makeShortcut (ShortcutManager::ChangePath, menu_);
  connect (editPath, &QAction::triggered,
           pathWidget_, &PathWidget::edit);

  auto runCommand = makeShortcut (ShortcutManager::RunCommand, menu_);
  connect (runCommand, &QAction::triggered,
           this, &DirWidget::showCommandPrompt);


  menu_->addSeparator ();


  isLocked_ = makeShortcut (ShortcutManager::LockTab, menu_, true);
  connect (isLocked_, &QAction::toggled,
           this, &DirWidget::setLocked);
  connect (isLocked_, &QAction::toggled,
           pathWidget_, &PathWidget::setReadOnly);

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
           this, &DirWidget::openSelected);

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
           this, &DirWidget::openInEditor);


  viewAction_ = makeShortcut (ShortcutManager::View, viewMenu_);
  connect (viewAction_, &QAction::triggered,
           this, &DirWidget::viewCurrent);

  viewMenu_->addSeparator ();

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

  permissionsAction_ = makeShortcut (ShortcutManager::ChangePermissions, viewMenu_);
  connect (permissionsAction_, &QAction::triggered,
           view_, &DirView::changeCurrentPermissions);

  trashAction_ = makeShortcut (ShortcutManager::Trash, viewMenu_);
  connect (trashAction_, &QAction::triggered,
           this, &DirWidget::promptTrash);

  removeAction_ = makeShortcut (ShortcutManager::Remove, viewMenu_);
  connect (removeAction_, &QAction::triggered,
           this, &DirWidget::promptRemove);

  viewMenu_->addSeparator ();

  showPropertiesAction_ = makeShortcut (ShortcutManager::ShowProperties, viewMenu_);
  connect (showPropertiesAction_, &QAction::triggered,
           this, &DirWidget::showProperties);


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

  isMinSizeFixed_ = makeShortcut (ShortcutManager::FixMinSize, representMenu, true);
  connect (isMinSizeFixed_, &QAction::toggled,
           this, &DirWidget::fixateSizeAsMinimal);

  upAction_ = makeShortcut (ShortcutManager::MoveUp, nullptr);
  connect (upAction_, &QAction::triggered,
           this, &DirWidget::moveUp);
  auto up = new QToolButton (this);
  up->setDefaultAction (upAction_);

  newFolderAction_ = makeShortcut (ShortcutManager::CreateFolder, nullptr);
  connect (newFolderAction_, &QAction::triggered,
           this, &DirWidget::newFolder);
  auto newFolder = new QToolButton (this);
  newFolder->setDefaultAction (newFolderAction_);

  addAction (navigationHistory_->forwardAction ());
  addAction (navigationHistory_->backwardAction ());
  connect (navigationHistory_, &NavigationHistory::pathChanged,
           this, &DirWidget::setPath);

  connect (pathWidget_, &PathWidget::pathChanged,
           this, &DirWidget::setPath);
  connect (pathWidget_, &PathWidget::editionFinished,
           this, &DirWidget::activate);

  controlsLayout_->addWidget (newFolder);
  controlsLayout_->addWidget (pathWidget_);
  controlsLayout_->addWidget (viewMenuButton);
  controlsLayout_->addWidget (up);


  commandPrompt_->hide ();
  commandPrompt_->installEventFilter (this);
  commandPrompt_->setToolTip (tr ("If starts with '+' - runs command in console. "
                                  "Substitutions: %ID% - tab with ID, "
                                  "%-ID% - current item of tab,"
                                  "%<separator?>*ID% - selected items of tab"));
  connect (commandPrompt_, &QLineEdit::returnPressed,
           this, &DirWidget::execCommandPrompt);
  connect (commandPrompt_, &QLineEdit::editingFinished,
           this, &DirWidget::activate);


  // defaults
  auto layout = new QVBoxLayout (this);
  layout->setMargin (4);
  layout->setSpacing (2);
  layout->addLayout (controlsLayout_);
  layout->addWidget (view_);
  layout->addWidget (commandPrompt_);
  layout->addWidget (status_);


  connect (view_, &DirView::contextMenuRequested,
           this, &DirWidget::showViewContextMenu);
  connect (view_, &DirView::activated,
           this, &DirWidget::openSelected);
  connect (view_, &DirView::movedBackward,
           upAction_, &QAction::trigger);
  connect (view_, &DirView::backgroundActivated,
           this, &DirWidget::openInBackground);
  connect (view_, &DirView::selectionChanged,
           this, &DirWidget::updateActions);
  connect (view_, &DirView::selectionChanged,
           this, &DirWidget::updateStatusSelection);
  connect (view_, &DirView::currentChanged,
           this, &DirWidget::updateCurrentFile);

  installEventFilter (this);

  SettingsManager::subscribeForUpdates (this);
  updateSettings ();
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
  settings.setValue (qs_minSize, isMinSizeFixed () ? minimumSize () : QSize ());
  view_->save (settings);
}

void DirWidget::restore (QSettings &settings)
{
  navigationHistory_->clear ();
  listMode_->setChecked (settings.value (qs_isList, isLocked ()).toBool ());
  setPath (settings.value (qs_dir).toString ());
  isLocked_->setChecked (settings.value (qs_isLocked, isLocked ()).toBool ());
  extensiveAction_->setChecked (settings.value (qs_extensive, false).toBool ());
  showDirs_->setChecked (settings.value (qs_showDirs, true).toBool ());
  showHidden_->setChecked (settings.value (qs_showHidden, false).toBool ());
  showThumbs_->setChecked (settings.value (qs_showThumbs, false).toBool ());
  const auto fixedSize = settings.value (qs_minSize).toSize ();
  if (!fixedSize.isEmpty ())
  {
    resize (fixedSize);
    isMinSizeFixed_->setChecked (true);
  }
  view_->restore (settings);
}

QFileInfo DirWidget::path () const
{
  return path_;
}

void DirWidget::setSiblings (const QList<DirWidget *> siblings)
{
  siblings_ = siblings;
  createSiblingActions ();
}

void DirWidget::createSiblingActions ()
{
  using SM = ShortcutManager;
  auto handle = [this](SM::Shortcut s, QMenu *menu, Qt::DropAction drop) {
                  for (auto *i: menu->actions ())
                  {
                    removeAction (i);
                  }
                  menu->clear ();

                  const auto common = SM::get (s).toString ();
                  auto number = 0;
                  for (auto *i: siblings_)
                  {
                    const auto index = i->index ();
                    auto action = menu->addAction (QString::number (++number) + index);
                    action->setData (QVariant::fromValue (i));
                    if (!common.isEmpty () && !index.isEmpty ())
                    {
                      action->setShortcut ({common + ',' + index});
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
}

void DirWidget::updateSiblingActions ()
{
  auto handle = [this](QMenu *menu) {
                  for (auto *i: menu->actions ())
                  {
                    auto *w = i->data ().value<DirWidget *>();
                    ASSERT (w);
                    i->setText (w->fullName (100));
                  }
                };

  handle (copyToMenu_);
  handle (moveToMenu_);
  handle (linkToMenu_);
}

void DirWidget::showCommandPrompt ()
{
  commandPrompt_->selectAll ();
  commandPrompt_->show ();
  commandPrompt_->setFocus ();
}

void DirWidget::execCommandPrompt ()
{
  ShellCommand command (commandPrompt_->text ());
  command.preprocessSelection ("", path_, current (), selected ());
  for (const auto *i: siblings_)
  {
    command.preprocessSelection (i->index (), i->path_, i->current (), i->selected ());
  }
  command.setConsoleWrapper (runInConsoleCommand_);
  command.preprocessFileArguments (path_);
  command.setWorkDir (path_);
  if (!command.run ())
  {
    commandPrompt_->selectAll ();
    return;
  }
  commandPrompt_->hide ();
}

QString DirWidget::index () const
{
  return index_;
}

void DirWidget::setIndex (const QString &index)
{
  index_ = index;
  pathWidget_->setIndex (index);
}

QString DirWidget::fullName (int preferredWidth) const
{
  return pathWidget_->fullName (preferredWidth);
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
  if (isLocked ())
  {
    return;
  }

  if (index.isValid () && proxy_->isDotDot (index))
  {
    openPath (index.parent ().parent ());
    return;
  }

  if (!index.isValid ()) // drives
  {
    view_->setRootIndex ({});
    proxy_->setCurrent ({});
  }
  else
  {
    auto newIndex = index.sibling (index.row (), FileSystemModel::Name);
    const auto previous = view_->rootIndex ();
    const auto moveUp = previous.parent () == newIndex;
    view_->setRootIndex (newIndex);
    view_->setCurrentIndex (moveUp ? previous : view_->firstItem ());
    proxy_->setCurrent (newIndex);
  }
  path_ = fileInfo (view_->rootIndex ());
  pathWidget_->setPath (path_);
  navigationHistory_->addPath (path_);
}

void DirWidget::openFile (const QFileInfo &info)
{
  if (info.isDir ())
  {
    return;
  }

  if (!info.permission (QFile::ExeUser))
  {
    QDesktopServices::openUrl (QUrl::fromLocalFile (info.absoluteFilePath ()));
    return;
  }

  if (!QProcess::startDetached (info.absoluteFilePath (), {}, info.absolutePath ()))
  {
    Notifier::error (tr ("Failed to open '%1'").arg (info.absoluteFilePath ()));
  }
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

QAction * DirWidget::makeShortcut (int shortcutType, QMenu *menu, bool isCheckable)
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
  pathWidget_->adjust ();
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
    return false;
  }

  if (isLocked ())
  {
    return false;
  }

  if (event->type () == QEvent::KeyPress && watched == commandPrompt_)
  {
    if (static_cast<QKeyEvent *>(event)->key () == Qt::Key_Escape)
    {
      commandPrompt_->hide ();
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

void DirWidget::updateSettings ()
{
  SettingsManager settings;
  using Type = SettingsManager::Type;
  openConsoleCommand_ = settings.get (Type::OpenConsoleCommand).toString ().trimmed ();
  runInConsoleCommand_ = settings.get (Type::RunInConsoleCommand).toString ().trimmed ();
  editorCommand_ = settings.get (Type::EditorCommand).toString ().trimmed ();
}

void DirWidget::promptClose ()
{
  auto res = QMessageBox::question (this, {}, tr ("Close tab \"%1\"?")
                                    .arg (path ().absoluteFilePath ()));
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
                                    .arg (names (indexes).join (QLatin1String ("\", \""))));
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
                                    .arg (names (indexes).join (QLatin1String ("\", \""))));
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

void DirWidget::openSelected ()
{
  const auto indexes = view_->selectedRows ();
  if (indexes.size () < 2)
  {
    const auto index = view_->currentIndex ();
    if (indexes.size () == 1 && indexes.first () != index)
    {
      return;
    }

    const auto current = fileInfo (view_->currentIndex ());
    if (current.isDir ())
    {
      openPath (view_->currentIndex ());
      return;
    }
    if (indexes.isEmpty ())
    {
      return;
    }
  }

  for (const auto &i: indexes)
  {
    openFile (fileInfo (i));
  }
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
  if (urls.isEmpty ())
  {
    return;
  }
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

void DirWidget::showProperties ()
{
  const auto info = view_->currentIndex ().isValid () ? current () : path_;
  auto *w = new PropertiesWidget (info);
  w->setAttribute (Qt::WA_DeleteOnClose, true);
  w->show ();
  auto global = mapToGlobal (rect ().center ());
  w->move (global.x () - w->width () / 2, global.y () - w->height () / 2);
}

void DirWidget::viewCurrent ()
{
  if (!current ().isDir ())
  {
    auto view = new FileViewer;
    view->showFile (current ().absoluteFilePath ());
  }
  else
  {
    setPath (current ());
  }
}

void DirWidget::moveUp ()
{
  openPath (view_->rootIndex ().parent ());
}

void DirWidget::openConsole ()
{
  ShellCommand command (openConsoleCommand_);
  command.preprocessFileArguments (path_);
  command.setWorkDir (path_);
  command.run ();
}

void DirWidget::openInEditor ()
{
  ShellCommand command (editorCommand_);
  command.preprocessFileArguments (current (), true);
  command.setWorkDir (path_);
  command.run ();
}

bool DirWidget::isMinSizeFixed () const
{
  return isMinSizeFixed_->isChecked ();
}

void DirWidget::fixateSizeAsMinimal (bool isOn)
{
  if (isOn)
  {
    setMinimumSize (size ());
  }
  else
  {
    setMinimumSize ({});
  }
}

void DirWidget::showViewContextMenu ()
{
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

void DirWidget::updateStatusSelection ()
{
  status_->updateSelection (selected ());
}

void DirWidget::updateActions ()
{
  const auto dirs = showDirs_->isChecked ();
  const auto locked = isLocked_->isChecked ();
  const auto index = view_->currentIndex ();
  const auto isDotDot = index.isValid () && proxy_->isDotDot (index);
  const auto isDir =  proxy_->isDir (index.row ());
  const auto isValid = index.isValid ();
  const auto isSingleSelected = (view_->selectedRows ().size () <= 1);

  newFolderAction_->setEnabled (dirs && !locked);
  upAction_->setEnabled (!locked);

  openAction_->setEnabled (true);
  openWith_->setEnabled (isValid && !isDir && isSingleSelected);
  if (openWith_->isEnabled ())
  {
    OpenWith::popupateMenu (*openWith_, current ());
  }
  viewAction_->setEnabled (isValid && !isDir && isSingleSelected);
  openInTabAction_->setEnabled (isValid && isDir && isSingleSelected);
  openInEditorAction_->setEnabled (isValid && !isDir && isSingleSelected);

  copyPathAction_->setEnabled (isValid && isSingleSelected);

  cutAction_->setEnabled (!locked && isValid);
  copyAction_->setEnabled (isValid);
  pasteAction_->setEnabled (!locked);

  copyToMenu_->setEnabled (isValid && !isDotDot);
  moveToMenu_->setEnabled (!locked && isValid && !isDotDot);
  linkToMenu_->setEnabled (isValid && !isDotDot);

  permissionsAction_->setEnabled (!locked && isValid && !isDotDot && isSingleSelected);
  renameAction_->setEnabled (!locked && isValid && !isDotDot && isSingleSelected);
  removeAction_->setEnabled (!locked && isValid && !isDotDot);
  trashAction_->setEnabled (!locked && isValid && !isDotDot);

  showPropertiesAction_->setEnabled (isValid && isSingleSelected);
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

void DirWidget::updateCurrentFile ()
{
  lastCurrentFile_ = current ().fileName ();
}

void DirWidget::handleDirRename (const QString &path, const QString &old, const QString &now)
{
  if (path_.absoluteFilePath () == (path + '/' + old))
  {
    setPath (path + '/' + now);
  }
}

void DirWidget::handleFileRename (const QString &path, const QString &old, const QString &now)
{
  if (path_ == path && old == lastCurrentFile_)
  {
    lastCurrentFile_ = now;
    view_->setCurrentIndex (proxy_->mapFromSource (model_->index (path + '/' + now)));
  }
}

#include "moc_dirwidget.cpp"
