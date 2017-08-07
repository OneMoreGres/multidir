#include "pathwidget.h"
#include "filesystemcompleter.h"
#include "constants.h"
#include "debug.h"

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDir>
#include <QKeyEvent>

PathWidget::PathWidget (FileSystemModel *model, QWidget *parent) :
  QWidget (parent),
  isReadOnly_ (false),
  path_ (),
  indexLabel_ (new QLabel (this)),
  pathLabel_ (new QLabel (this)),
  dirLabel_ (new QLabel (this)),
  pathEdit_ (new QLineEdit (this))
{
  installEventFilter (this);

  pathLabel_->setAlignment (pathLabel_->alignment () | Qt::AlignRight);

  auto dirFont = dirLabel_->font ();
  dirFont.setBold (true);
  dirLabel_->setFont (dirFont);

  pathEdit_->setCompleter (new FileSystemCompleter (model, this));
  connect (pathEdit_, &QLineEdit::editingFinished,
           this, &PathWidget::applyEdition);
  pathEdit_->installEventFilter (this);


  auto layout = new QHBoxLayout (this);
  layout->addStretch (1);
  layout->addWidget (indexLabel_);
  layout->addWidget (pathLabel_);
  layout->addWidget (dirLabel_);
  layout->addWidget (pathEdit_);
  layout->addStretch (1);
  layout->setStretch (layout->indexOf (pathEdit_), 40);


  toggleEdition (false);
}

void PathWidget::setReadOnly (bool isReadOnly)
{
  isReadOnly_ = isReadOnly;
  rejectEdition ();
}

void PathWidget::setIndex (const QString &index)
{
  rejectEdition ();

  if (!index.isEmpty ())
  {
    indexLabel_->setText ('(' + index + ')');
  }
  else
  {
    indexLabel_->clear ();
  }
}

void PathWidget::setPath (const QFileInfo &path)
{
  rejectEdition ();

  path_ = path;
  const auto absolutePath = path_.absoluteFilePath ();
  if (absolutePath.isEmpty ())
  {
    pathLabel_->clear ();
    dirLabel_->setText (tr ("Drives"));
  }
  else
  {
    auto nameIndex = absolutePath.lastIndexOf (QLatin1Char ('/')) + 1;
    dirLabel_->setText (nameIndex ? absolutePath.mid (nameIndex) : absolutePath);
    adjust ();
  }
}

QString PathWidget::fittedPath (int maxWidth) const
{
  auto path = path_.absoluteFilePath ();
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

QString PathWidget::fullName (int preferredWidth) const
{
  return indexLabel_->text () + ' ' + fittedPath (preferredWidth) + dirLabel_->text ();
}

void PathWidget::toggleEdition (bool isOn)
{
  pathEdit_->setVisible (isOn);
  indexLabel_->setVisible (!isOn);
  pathLabel_->setVisible (!isOn);
  dirLabel_->setVisible (!isOn);
}

void PathWidget::edit ()
{
  toggleEdition (true);

  const auto path = QDir::toNativeSeparators (path_.absoluteFilePath ());
  pathEdit_->setText (path);
  pathEdit_->setFocus ();
  pathEdit_->selectAll ();
}

void PathWidget::applyEdition ()
{
  if (!pathEdit_->isVisible ())
  {
    return;
  }

  toggleEdition (false);

  const auto path = QDir::toNativeSeparators (path_.absoluteFilePath ());
  const auto newPath = pathEdit_->text ();
#ifdef Q_OS_WIN
  if (newPath.startsWith (constants::networkDirStart))
  {
    return;
  }
#endif
  if (newPath != path && QFile::exists (newPath))
  {
    emit pathChanged (QFileInfo (newPath));
  }
  emit editionFinished ();
}

void PathWidget::rejectEdition ()
{
  if (!pathEdit_->isVisible ())
  {
    return;
  }

  toggleEdition (false);
  emit editionFinished ();
}

void PathWidget::adjust ()
{
  const auto stretchWidth = layout ()->itemAt (0)->geometry ().width ();
  const auto maxWidth = pathLabel_->width () + 2 * stretchWidth - 10; // 10 just for sure
  const auto newPath = fittedPath (maxWidth);
  if (newPath != pathLabel_->text ())
  {
    pathLabel_->setText (QDir::toNativeSeparators (newPath));
  }
}


bool PathWidget::eventFilter (QObject *watched, QEvent *event)
{
  if (isReadOnly_)
  {
    return false;
  }

  if (event->type () == QEvent::MouseButtonDblClick && watched == this)
  {
    edit ();
  }
  else if (event->type () == QEvent::KeyPress && watched == pathEdit_)
  {
    if (static_cast<QKeyEvent *>(event)->key () == Qt::Key_Escape)
    {
      rejectEdition ();
      return true;
    }
  }
  return false;
}

#include "moc_pathwidget.cpp"
