#include "fileoperationwidget.h"
#include "debug.h"
#include "fileoperation.h"

#include <QFontMetrics>
#include <QProgressBar>
#include <QBoxLayout>

FileOperationWidget::FileOperationWidget (QSharedPointer<FileOperation> operation, QWidget *parent) :
  QWidget (parent),
  progress_ (new QProgressBar (this)),
  operation_ (operation)
{
  ASSERT (operation);

  auto layout = new QHBoxLayout (this);
  layout->setMargin (0);
  layout->addWidget (progress_);


  const auto action = QMap<FileOperation::Action, QString>{
    {FileOperation::Action::Copy, tr ("Copying")},{FileOperation::Action::Link, tr ("Linking")},
    {FileOperation::Action::Move, tr ("Moving")}, {FileOperation::Action::Remove, tr ("Removing")},
    {FileOperation::Action::Trash, tr ("Moving to trash")}
  }.value (operation_->action ());

  ASSERT (!operation_->sources ().isEmpty ());
  auto name = operation_->sources ().first ().fileName ();
  if (operation_->sources ().size () > 1)
  {
    name += ", ...";
  }

  const auto target = operation_->target ().fileName ();

  const auto text = tr ("%1 \"%2\" to \"%3\"").arg (action, name, target);
  progress_->setFormat (text);
  progress_->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Fixed);
  progress_->setMaximumWidth (fontMetrics ().boundingRect (text).width ());
  progress_->setMinimumWidth (fontMetrics ().boundingRect (text).width ());
  progress_->setRange (0, 1);
  progress_->setValue (0);

  connect (operation_.data (), &FileOperation::finished,
           this, &QObject::deleteLater);
}
