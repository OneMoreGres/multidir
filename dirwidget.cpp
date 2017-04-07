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
#include <QResizeEvent>
#include <QFontMetrics>

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
  showDirs_ (new QToolButton (this)),
  controlsLayout_ (new QHBoxLayout)
{
  setContextMenuPolicy (Qt::CustomContextMenu);
  connect (this, &QWidget::customContextMenuRequested,
           this, &DirWidget::showContextMenu);

  proxy_->setDynamicSortFilter (true);

  view_->setModel (proxy_);
  view_->setSortingEnabled (true);
  view_->setSelectionBehavior (QAbstractItemView::SelectRows);
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

  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (pathLabel_);
  controlsLayout_->addWidget (dirLabel_);
  controlsLayout_->addStretch (1);
  controlsLayout_->addWidget (up_);
  controlsLayout_->addWidget (showDirs_);

  auto layout = new QVBoxLayout (this);
  layout->addLayout (controlsLayout_);
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
  auto absolutePath = QDir (path).absolutePath ();
  auto index = proxy_->mapFromSource (model_->index (absolutePath));
  if (!index.isValid ())
  {
    return;
  }
  view_->setRootIndex (index);
  proxy_->setCurrent (index);

  pathLabel_->setText (fittedPath ());
  auto nameIndex = absolutePath.lastIndexOf (QDir::separator ()) + 1;
  dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);
}

QString DirWidget::path () const
{
  return path (view_->rootIndex ());
}

void DirWidget::setNameFilter (const QString &filter)
{
  proxy_->setNameFilter ("*" + filter + "*");
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
  const auto nameIndex = path.lastIndexOf (QDir::separator ()) + 1;
  if (!nameIndex)
  {
    return {};
  }

  const auto stretchWidth = controlsLayout_->itemAt (0)->geometry ().width ();
  const auto pathText = pathLabel_->text ();

  if (stretchWidth == 0 || pathText.isEmpty () || pathText.startsWith (QLatin1String ("...")))
  {
    const auto maxWidth = pathLabel_->width () + 2 * stretchWidth - 10;
    const QString prepend = QLatin1String ("...") + QDir::separator ();
    const auto searchStartIndex = prepend.length ();

    QFontMetrics metrics (pathLabel_->font ());
    path = path.left (nameIndex);
    auto width = metrics.width (path);

    while (width > maxWidth)
    {
      auto index = path.indexOf (QDir::separator (), searchStartIndex);
      if (index == -1)
      {
        break;
      }
      path = prepend + path.mid (index + 1);
      width = metrics.width (path);
    }
    return path;
  }
  return pathText;
}
