#include "fileconflictresolver.h"
#include "fileoperation.h"
#include "debug.h"
#include "utils.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QBoxLayout>
#include <QDateTime>

FileConflictResolver::FileConflictResolver (QWidget *parent) :
  QDialog (parent),
  sourceLabel_ (new QLabel (this)),
  targetLabel_ (new QLabel (this)),
  source_ (new QPushButton (tr ("Use new"), this)),
  target_ (new QPushButton (tr ("Use existing"), this)),
  rename_ (new QPushButton (tr ("Rename new"), this)),
  abort_ (new QPushButton (tr ("Abort"), this)),
  applyToAll_ (new QCheckBox (tr ("Apply to all"), this))
{
  auto layout = new QGridLayout (this);
  auto font = this->font ();
  font.setBold (true);

  const auto cols = 2;
  auto row = 0;
  {
    auto label = new QLabel (tr ("File operation conflict"), this);
    label->setAlignment (Qt::AlignHCenter);
    font.setPointSize (12);
    label->setFont (font);
    layout->addWidget (label, row, 0, 1, cols);
  }

  ++row;
  {
    auto label = new QLabel (tr ("New file:"), this);
    font.setPointSize (10);
    label->setFont (font);
    sourceLabel_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget (label, row, 0);
    layout->addWidget (sourceLabel_, row, 1);
  }

  ++row;
  {
    auto label = new QLabel (tr ("Existing file:"), this);
    font.setPointSize (10);
    label->setFont (font);
    targetLabel_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget (label, row, 0);
    layout->addWidget (targetLabel_, row, 1);
  }

  ++row;
  auto buttonsLayout = new QHBoxLayout;
  layout->addLayout (buttonsLayout, row, 0, 1, cols);
  buttonsLayout->addWidget (source_);
  buttonsLayout->addWidget (target_);
  buttonsLayout->addWidget (rename_);
  buttonsLayout->addStretch (2);
  buttonsLayout->addWidget (applyToAll_);
  buttonsLayout->addStretch (2);
  buttonsLayout->addWidget (abort_);


  connect (source_, &QPushButton::pressed, this, [this] {done (Source);});
  connect (target_, &QPushButton::pressed, this, [this] {done (Target);});
  connect (rename_, &QPushButton::pressed, this, [this] {done (Rename);});
  connect (abort_, &QPushButton::pressed, this, [this] {done (Abort);});
}

void FileConflictResolver::resolve (const QFileInfo &source, const QFileInfo &target,
                                    int action, int *result)
{
  ASSERT (result);
  auto labelText = [](const QFileInfo &i) {
                     return i.absoluteFilePath ()
                            + tr ("\nModified: ") + i.lastModified ().toString (Qt::ISODate)
                            + tr (". Size: ") + utils::sizeString (i);
                   };

  switch (FileOperation::Action (action))
  {
    case FileOperation::Action::Copy:
    case FileOperation::Action::Move:
    case FileOperation::Action::Link:
      sourceLabel_->setText (labelText (source));
      targetLabel_->setText (labelText (target));
      break;

    case FileOperation::Action::Remove:
    case FileOperation::Action::Trash:
      break;
  }

  setMaximumSize (sizeHint ());

  *result = exec ();
  if (*result == Pending)
  {
    *result = Abort;
  }
  if (applyToAll_->isChecked ())
  {
    *result |= All;
  }
}
