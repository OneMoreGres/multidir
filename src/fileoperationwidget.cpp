#include "fileoperationwidget.h"
#include "debug.h"
#include "fileoperation.h"

#include <QFontMetrics>
#include <QProgressBar>
#include <QBoxLayout>
#include <QPushButton>

FileOperationWidget::FileOperationWidget (QSharedPointer<FileOperation> operation, QWidget *parent) :
  QWidget (parent),
  progress_ (new QProgressBar (this)),
  operation_ (operation)
{
  ASSERT (operation);

  auto layout = new QHBoxLayout (this);
  layout->setMargin (0);
  layout->setSpacing (0);
  layout->addWidget (progress_);

  auto abortButton = new QPushButton (this);
  abortButton->setFlat (true);
  abortButton->setIcon (QIcon (":/abort.png"));
  connect (abortButton, &QPushButton::pressed,
           operation.data (), &FileOperation::abort);
  layout->addWidget (abortButton);


  const auto action = operation_->action ();
  const auto actionText = QMap<FileOperation::Action, QString>{
    {FileOperation::Action::Copy, tr ("Copying")},{FileOperation::Action::Link, tr ("Linking")},
    {FileOperation::Action::Move, tr ("Moving")}, {FileOperation::Action::Remove, tr ("Removing")},
    {FileOperation::Action::Trash, tr ("Moving to trash")}
  }.value (action);

  ASSERT (!operation_->sources ().isEmpty ());
  auto name = operation_->sources ().first ().fileName ();
  if (operation_->sources ().size () > 1)
  {
    name += ", ...";
  }

  const auto target = operation_->target ().fileName ();

  if (action == FileOperation::Action::Trash || action == FileOperation::Action::Remove)
  {
    template_ = tr ("%1 \"%2\"").arg (actionText, QLatin1String ("%1"));
  }
  else
  {
    template_ = tr ("%1 \"%2\" to \"%3\"").arg (actionText, QLatin1String ("%1"), target);
  }
  current_ = name;
  timerEvent (nullptr);
  progress_->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Fixed);
  progress_->setRange (0, 100);
  progress_->setValue (0);

  connect (operation_.data (), &FileOperation::progress,
           progress_, &QProgressBar::setValue);

  connect (operation_.data (), &FileOperation::finished,
           this, &QObject::deleteLater);
  connect (operation_.data (), &FileOperation::currentChanged,
           this, &FileOperationWidget::setCurrent);

  startTimer (100);
}


void FileOperationWidget::timerEvent (QTimerEvent *)
{
  const auto text = template_.arg (current_);
  if (progress_->format () != text)
  {
    progress_->setFormat (text);
    progress_->setMaximumWidth (fontMetrics ().boundingRect (text).width ());
    progress_->setMinimumWidth (fontMetrics ().boundingRect (text).width ());
  }
}

void FileOperationWidget::setCurrent (const QString &current)
{
  current_ = current;
}
