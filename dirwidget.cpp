#include "dirwidget.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "copypaste.h"
#include "dirview.h"
#include "trash.h"
#include "constants.h"

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

#include <QDebug>

namespace
{
const QString qs_extensive = "extensive";
const QString qs_isList = "isList";
const QString qs_dir = "dir";
const QString qs_isLocked = "locked";
const QString qs_showDirs = "showDirs";
const QString qs_showThumbs = "showThumbs";
}

DirWidget::DirWidget (FileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  proxy_ (new ProxyModel (model, this)),
  view_ (new DirView (*proxy_, this)),
  pathLabel_ (new QLabel (this)),
  dirLabel_ (new QLabel (this)),
  pathEdit_ (new QLineEdit (this)),
  menu_ (new QMenu (this)),
  isLocked_ (nullptr),
  showDirs_ (nullptr),
  extensiveAction_ (nullptr),
  listMode_ (nullptr),
  showThumbs_ (nullptr),
  viewMenu_ (new QMenu (this)),
  openAction_ (nullptr),
  openInTabAction_ (nullptr),
  renameAction_ (nullptr),
  trashAction_ (nullptr),
  removeAction_ (nullptr),
  cutAction_ (nullptr),
  copyAction_ (nullptr),
  pasteAction_ (nullptr),
  up_ (new QToolButton (this)),
  newFolder_ (new QToolButton (this)),
  controlsLayout_ (new QHBoxLayout)
{
  proxy_->setDynamicSortFilter (true);


  // menu
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

  auto representMenu = menu_->addMenu (tr ("View"));

  showDirs_ = representMenu->addAction (QIcon (":/folder.png"), tr ("Show directories"));
  showDirs_->setCheckable (true);
  showDirs_->setChecked (proxy_->showDirs ());
  connect (showDirs_, &QAction::toggled,
           this, &DirWidget::setShowDirs);

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

  viewMenu_->addSeparator ();

  renameAction_ = viewMenu_->addAction (QIcon (":/rename.png"), tr ("Rename"));
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
           this, &DirWidget::moveUp);

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
  settings.setValue (qs_dir, path ());
  settings.setValue (qs_isLocked, isLocked ());
  settings.setValue (qs_extensive, extensiveAction_->isChecked ());
  settings.setValue (qs_showDirs, proxy_->showDirs ());
  settings.setValue (qs_showThumbs, showThumbs_->isChecked ());
}

void DirWidget::restore (QSettings &settings)
{
  listMode_->setChecked (settings.value (qs_isList, isLocked ()).toBool ());
  setPath (settings.value (qs_dir).toString ());
  isLocked_->setChecked (settings.value (qs_isLocked, isLocked ()).toBool ());
  extensiveAction_->setChecked (settings.value (qs_extensive, false).toBool ());
  showDirs_->setChecked (settings.value (qs_showDirs, true).toBool ());
  showThumbs_->setChecked (settings.value (qs_showThumbs, false).toBool ());
}

QString DirWidget::path () const
{
  return model_->filePath (proxy_->mapToSource (view_->rootIndex ()));
}

void DirWidget::setPath (const QString &path)
{
  if (path.isEmpty ())
  {
    // do not change proxy current path to disable possible dir filtering
    view_->setRootIndex ({});
    pathLabel_->clear ();
    dirLabel_->setText (tr ("Drives"));
    return;
  }

  auto absolutePath = QDir (path).absolutePath ();
  auto index = proxy_->mapFromSource (model_->index (absolutePath));
  if (!index.isValid ())
  {
    return;
  }
  view_->setRootIndex (index);
  proxy_->setCurrent (index);

  pathLabel_->setText (fittedPath ());
  auto nameIndex = absolutePath.lastIndexOf (QLatin1Char ('/')) + 1;
  dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);
}

void DirWidget::setNameFilter (const QString &filter)
{
  proxy_->setNameFilter (QLatin1String ("*") + filter + QLatin1String ("*"));
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
      if (!path.endsWith (constants::dotdot))
      {
        setPath (path);
      }
      else
      {
        moveUp ();
      }
    }
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

void DirWidget::moveUp ()
{
  const auto path = this->path ();
  if (path.isEmpty ())
  {
    return;
  }
  QDir dir (path);
  if (dir.cdUp ())
  {
    setPath (dir.absolutePath ());
  }
  else
  {
    setPath ({});
  }
}

void DirWidget::newFolder ()
{
  QDir dir (path ());
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
  auto res = QMessageBox::question (this, {}, tr ("Close tab \"%1\"?").arg (path ()),
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
  QStringList names;
  for (const auto &i: indexes)
  {
    names << i.data ().toString ();
  }
  auto res = QMessageBox::question (this, {}, tr ("Move files \"%1\" to trash?")
                                    .arg (names.join (QLatin1String ("\", \""))),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (res == QMessageBox::Yes)
  {
    for (const auto &i: indexes)
    {
      Trash::trash (model_->fileInfo (proxy_->mapToSource (i)));
    }
  }
}

void DirWidget::promptRemove ()
{
  const auto indexes = view_->selectedRows ();
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

QList<QFileInfo> DirWidget::selected () const
{
  QList<QFileInfo> infos;
  for (const auto &i: view_->selectedRows ())
  {
    infos << model_->fileInfo (proxy_->mapToSource (i));
  }
  return infos;
}

QFileInfo DirWidget::current () const
{
  return model_->fileInfo (proxy_->mapToSource (view_->currentIndex ()));
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
  if (!index.isValid ())
  {
    return;
  }
  CopyPaste::paste (model_->fileInfo (proxy_->mapToSource (index)));
}

void DirWidget::showViewContextMenu ()
{
  const auto index = view_->currentIndex ();
  const auto isDotDot = index.isValid () && index.data () == constants::dotdot;
  const auto isDir = current ().isDir ();
  openAction_->setEnabled (index.isValid ());
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
