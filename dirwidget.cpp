#include "dirwidget.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "copypaste.h"
#include "dirview.h"
#include "fileoperation.h"
#include "constants.h"
#include "openwith.h"

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

#include <QDebug>

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
  openInTabAction_ (nullptr),
  renameAction_ (nullptr),
  trashAction_ (nullptr),
  removeAction_ (nullptr),
  cutAction_ (nullptr),
  copyAction_ (nullptr),
  pasteAction_ (nullptr),
  copyPathAction_ (nullptr),
  up_ (new QToolButton (this)),
  newFolder_ (new QToolButton (this)),
  controlsLayout_ (new QHBoxLayout)
{
  proxy_->setDynamicSortFilter (true);

  connect (model, &QAbstractItemModel::rowsRemoved,
           this, &DirWidget::checkDirExistence);
  connect (model, &QFileSystemModel::fileRenamed,
           this, &DirWidget::handleDirRename);


  // menu
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, [this] {menu_->exec (QCursor::pos ());});

  auto openExternal = menu_->addAction (QIcon (":/openExternal.png"), tr ("Open in explorer"));
  connect (openExternal, &QAction::triggered,
           this, [this] {QDesktopServices::openUrl (QUrl::fromLocalFile (
                                                      path ().absoluteFilePath ()));});

  auto editPath = menu_->addAction (QIcon (":/rename.png"), tr ("Change path"));
  connect (editPath, &QAction::triggered,
           this, [this] {togglePathEdition (true);});

  auto openConsole = menu_->addAction (tr ("Open console"));
  connect (openConsole, &QAction::triggered,
           this, [this] {emit consoleRequested (path_.absoluteFilePath ());});


  menu_->addSeparator ();

  auto representMenu = menu_->addMenu (tr ("View"));

  showDirs_ = representMenu->addAction (QIcon (":/folder.png"), tr ("Show directories"));
  showDirs_->setCheckable (true);
  showDirs_->setChecked (proxy_->showDirs ());
  connect (showDirs_, &QAction::toggled,
           this, &DirWidget::setShowDirs);

  showHidden_ = representMenu->addAction (QIcon (":/hidden.png"), tr ("Show hidden"));
  showHidden_->setCheckable (true);
  showHidden_->setChecked (proxy_->showHidden ());
  connect (showHidden_, &QAction::toggled,
           proxy_, &ProxyModel::setShowHidden);

  extensiveAction_ = representMenu->addAction (QIcon (":/extensive.png"), tr ("Extensive mode"));
  extensiveAction_->setCheckable (true);
  extensiveAction_->setChecked (view_->isExtensive ());
  connect (extensiveAction_, &QAction::toggled,
           view_, &DirView::setExtensive);

  listMode_ = representMenu->addAction (QIcon (":/listMode.png"), tr ("List mode"));
  listMode_->setCheckable (true);
  listMode_->setChecked (view_->isList ());
  connect (listMode_, &QAction::toggled,
           view_, &DirView::setIsList);

  showThumbs_ = representMenu->addAction (QIcon (":/showThumbs.png"), tr ("Show thumbnails"));
  showThumbs_->setCheckable (true);
  showThumbs_->setChecked (proxy_->showThumbnails ());
  connect (showThumbs_, &QAction::toggled,
           proxy_, &ProxyModel::setShowThumbnails);


  isLocked_ = menu_->addAction (QIcon (":/lockTab.png"), tr ("Lock"));
  isLocked_->setCheckable (true);
  connect (isLocked_, &QAction::toggled,
           this, &DirWidget::setLocked);

  auto clone = menu_->addAction (QIcon (":/cloneTab.png"), tr ("Clone"));
  connect (clone, &QAction::triggered,
           this, [this]() {emit cloneRequested (this);});

  menu_->addSeparator ();

  auto close = menu_->addAction (QIcon (":/closeTab.png"), tr ("Close..."));
  connect (close, &QAction::triggered,
           this, &DirWidget::promptClose);

  // view menu
  openAction_ = viewMenu_->addAction (tr ("Open"));
  connect (openAction_, &QAction::triggered,
           this, [this]() {openPath (view_->currentIndex ());});

  openWith_ = viewMenu_->addMenu (tr ("Open with"));

  openInTabAction_ = viewMenu_->addAction (tr ("Open in tab"));
  connect (openInTabAction_, &QAction::triggered,
           this, [this]() {emit newTabRequested (current ().absoluteFilePath ());});

  cutAction_ = viewMenu_->addAction (QIcon::fromTheme ("cut"), tr ("Cut"));
  cutAction_->setShortcut (QKeySequence::Cut);
  cutAction_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  this->addAction (cutAction_);
  connect (cutAction_, &QAction::triggered,
           this, &DirWidget::cut);

  copyAction_ = viewMenu_->addAction (QIcon::fromTheme ("copy"), tr ("Copy"));
  copyAction_->setShortcut (QKeySequence::Copy);
  copyAction_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  this->addAction (copyAction_);
  connect (copyAction_, &QAction::triggered,
           this, &DirWidget::copy);

  pasteAction_ = viewMenu_->addAction (QIcon::fromTheme ("paste"), tr ("Paste"));
  pasteAction_->setShortcut (QKeySequence::Paste);
  pasteAction_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  this->addAction (pasteAction_);
  connect (pasteAction_, &QAction::triggered,
           this, &DirWidget::paste);

  copyPathAction_ = viewMenu_->addAction (tr ("Copy path"));
  connect (copyPathAction_, &QAction::triggered,
           this, &DirWidget::copyPath);

  viewMenu_->addSeparator ();

  renameAction_ = viewMenu_->addAction (QIcon (":/rename.png"), tr ("Rename"));
  renameAction_->setShortcut (QKeySequence (Qt::Key_F2));
  renameAction_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  this->addAction (renameAction_);
  connect (renameAction_, &QAction::triggered,
           view_, &DirView::renameCurrent);

  trashAction_ = viewMenu_->addAction (QIcon (":/trash.png"), tr ("Move to trash..."));
  connect (trashAction_, &QAction::triggered,
           this, &DirWidget::promptTrash);

  removeAction_ = viewMenu_->addAction (QIcon (":/remove.png"), tr ("Remove..."));
  connect (removeAction_, &QAction::triggered,
           this, &DirWidget::promptRemove);


  // controls
  up_->setIcon (QIcon (":/up.png"));
  up_->setToolTip (tr ("Move up"));
  connect (up_, &QToolButton::pressed,
           this, [this] {openPath (view_->rootIndex ().parent ());});

  newFolder_->setIcon (QIcon (":/newFolder.png"));
  newFolder_->setToolTip (tr ("Create folder"));
  connect (newFolder_, &QToolButton::pressed,
           this, &DirWidget::newFolder);


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
  controlsLayout_->addWidget (newFolder_);
  controlsLayout_->addWidget (up_);

  controlsLayout_->setStretch (controlsLayout_->indexOf (pathEdit_), 40);


  // defaults
  auto layout = new QVBoxLayout (this);
  layout->addLayout (controlsLayout_);
  layout->addWidget (view_);


  connect (view_, &DirView::contextMenuRequested,
           this, &DirWidget::showViewContextMenu);
  connect (view_, &DirView::doubleClicked,
           this, &DirWidget::openPath);
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
}

QFileInfo DirWidget::path () const
{
  return path_;
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
      QProcess::startDetached (info.absoluteFilePath (), {}, info.absolutePath ());
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
    view_->setRootIndex (index);
    proxy_->setCurrent (index);
    path_ = fileInfo (view_->rootIndex ());

    pathLabel_->setText (fittedPath ());
    const auto absolutePath = info.absoluteFilePath ();
    auto nameIndex = absolutePath.lastIndexOf (QLatin1Char ('/')) + 1;
    dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);
  }
}

QString DirWidget::fittedPath () const
{
  auto path = this->path ().absoluteFilePath ();
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

void DirWidget::resizeEvent (QResizeEvent *)
{
  const auto newText = fittedPath ();
  if (newText != pathLabel_->text ())
  {
    pathLabel_->setText (newText);
  }
}

bool DirWidget::eventFilter (QObject *watched, QEvent *event)
{
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

void DirWidget::togglePathEdition (bool isOn)
{
  pathEdit_->setVisible (isOn);
  pathLabel_->setVisible (!isOn);
  dirLabel_->setVisible (!isOn);

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
  auto index = view_->currentIndex ();
  if (!index.isValid ())
  {
    index = view_->rootIndex ();
  }
  const auto urls = CopyPaste::clipboardUrls ();
  if (!index.isValid () || urls.isEmpty ())
  {
    return;
  }
  const auto action = CopyPaste::clipboardAction ();
  emit fileOperation (FileOperation::paste (urls, fileInfo (index), action));
}

void DirWidget::copyPath ()
{
  const auto info = fileInfo (view_->currentIndex ());
  QApplication::clipboard ()->setText (info.absoluteFilePath ());
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
  newFolder_->setEnabled (dirs && !lock);
  up_->setEnabled (!lock);

  pasteAction_->setEnabled (!lock);
  cutAction_->setEnabled (!lock);
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
