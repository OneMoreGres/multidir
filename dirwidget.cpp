#include "dirwidget.h"
#include "proxymodel.h"

#include <QFileSystemModel>
#include <QTableView>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QToolButton>
#include <QHeaderView>
#include <QSettings>

#include <QDebug>

namespace
{
const QString qs_dir = "dir";
const QString qs_isLocked = "locked";
const QString qs_view = "view";
const QString qs_showDirs = "showDirs";
}

DirWidget::DirWidget (QFileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  model_ (model),
  proxy_ (new ProxyModel (model, this)),
  view_ (new QTableView (this)),
  menu_ (new QMenu (this)),
  pathLabel_ (new QLabel (this)),
  dirLabel_ (new QLabel (this)),
  isLocked_ (nullptr),
  up_ (new QToolButton (this)),
  showDirs_ (new QToolButton (this))
{
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &DirWidget::showContextMenu);

  proxy_->setDynamicSortFilter (true);

  view_->setModel (proxy_);
  view_->setSortingEnabled (true);
  connect (view_, &QTableView::doubleClicked,
           this, &DirWidget::openPath);


  isLocked_ = menu_->addAction (tr ("Locked"));
  isLocked_->setCheckable (true);
  connect (isLocked_, &QAction::toggled,
           this, &DirWidget::setIsLocked);

  auto close = menu_->addAction (tr ("Close"));
  connect (close, &QAction::triggered,
           this, [this]() {emit closeRequested (this);});

  auto clone = menu_->addAction (tr ("Clone"));
  connect (clone, &QAction::triggered,
           this, [this]() {emit cloneRequested (this);});


  up_->setIcon (QIcon::fromTheme ("up"));
  connect (up_, &QToolButton::pressed,
           this, &DirWidget::moveUp);

  showDirs_->setIcon (QIcon::fromTheme ("folder"));
  showDirs_->setCheckable (true);
  showDirs_->setChecked (proxy_->showDirs ());
  connect (showDirs_, &QToolButton::toggled,
           this, &DirWidget::toggleShowDirs);


  pathLabel_->setAlignment (pathLabel_->alignment () | Qt::AlignRight);
  auto dirFont = dirLabel_->font ();
  dirFont.setBold (true);
  dirLabel_->setFont (dirFont);

  auto controls = new QHBoxLayout;
  controls->addWidget (pathLabel_);
  controls->addWidget (dirLabel_);
  controls->addWidget (up_);
  controls->addWidget (showDirs_);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (controls);
  layout->addWidget (view_);

  setIsLocked (false);
}

DirWidget::~DirWidget ()
{

}

void DirWidget::save (QSettings &settings) const
{
  settings.setValue (qs_dir, path (view_->rootIndex ()));
  settings.setValue (qs_isLocked, isLocked ());
  settings.setValue (qs_view, view_->horizontalHeader ()->saveState ());
  settings.setValue (qs_showDirs, proxy_->showDirs ());
}

void DirWidget::restore (QSettings &settings)
{
  setPath (settings.value (qs_dir).toString ());
  isLocked_->setChecked (settings.value (qs_isLocked, isLocked ()).toBool ());
  if (settings.contains (qs_view))
  {
    view_->horizontalHeader ()->restoreState (settings.value (qs_view).toByteArray ());
  }
  showDirs_->setChecked (settings.value (qs_showDirs, true).toBool ());
}

void DirWidget::setPath (const QString &path)
{
  QDir dir (path);
  auto index = proxy_->mapFromSource (model_->index (dir.absolutePath ()));
  if (!index.isValid ())
  {
    return;
  }
  view_->setRootIndex (index);
  proxy_->setCurrent (index);
  dirLabel_->setText (dir.dirName ());
  pathLabel_->setText ((dir.cdUp () ? dir.absolutePath () : QString ()) + QDir::separator ());
}

QString DirWidget::path () const
{
  return path (view_->rootIndex ());
}

void DirWidget::setIsLocked (bool isLocked)
{
  up_->setEnabled (!isLocked);
}

void DirWidget::openPath (const QModelIndex &index)
{
  if (!index.isValid ())
  {
    return;
  }

  const auto mapped = proxy_->mapToSource (index);
  if (!(model_->permissions (mapped) & QFile::ExeUser))
  {
    return;
  }

  auto path = model_->filePath (mapped);
  if (!model_->isDir (mapped))
  {
    QDesktopServices::openUrl (QUrl::fromLocalFile (path));
  }
  else if (!isLocked ())
  {
    setPath (path);
  }
}

QString DirWidget::path (const QModelIndex &index) const
{
  return model_->filePath (proxy_->mapToSource (index));
}

bool DirWidget::isLocked () const
{
  return isLocked_->isChecked ();
}

void DirWidget::moveUp ()
{
  QDir dir (path (view_->rootIndex ()));
  if (dir.cdUp ())
  {
    setPath (dir.absolutePath ());
  }
}

void DirWidget::toggleShowDirs (bool show)
{
  proxy_->setShowDirs (show);
}

void DirWidget::showContextMenu ()
{
  menu_->exec (QCursor::pos ());
}
