#include "settings.h"

#include <QGridLayout>
#include <QKeySequenceEdit>
#include <QDialogButtonBox>
#include <QLabel>

Settings::Settings (QWidget *parent) :
  QDialog (parent),
  hotkey_ (new QKeySequenceEdit (this))
{
  setWindowTitle (tr ("Settings"));
  auto layout = new QGridLayout (this);

  auto row = 0;
  layout->addWidget (new QLabel (tr ("Toggle hotkey")), row, 0);
  layout->addWidget (hotkey_, row, 1);

  ++row;
  layout->addItem (new QSpacerItem (1,1,QSizePolicy::Expanding, QSizePolicy::Expanding), row, 0);

  ++row;
  auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &QDialog::accept);
  connect (buttons, &QDialogButtonBox::rejected,
           this, &QDialog::reject);
  layout->addWidget (buttons, row, 0, 1, 2);
}

QKeySequence Settings::hotkey () const
{
  return hotkey_->keySequence ();
}

void Settings::setHotkey (const QKeySequence &hotkey)
{
  hotkey_->setKeySequence (hotkey);
}
