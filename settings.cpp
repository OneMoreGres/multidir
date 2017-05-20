#include "settings.h"

#include <QGridLayout>
#include <QKeySequenceEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

Settings::Settings (QWidget *parent) :
  QDialog (parent),
  hotkey_ (new QKeySequenceEdit (this)),
  console_ (new QLineEdit (this)),
  checkUpdates_ (new QCheckBox (tr ("Check for updates"), this)),
  imageCache_ (new QSpinBox (this))
{
  setWindowTitle (tr ("Settings"));
  auto layout = new QGridLayout (this);

  auto row = 0;
  layout->addWidget (new QLabel (tr ("Toggle hotkey")), row, 0);
  layout->addWidget (hotkey_, row, 1);

  ++row;
  layout->addWidget (new QLabel (tr ("Console command")), row, 0);
  layout->addWidget (console_, row, 1);
  console_->setToolTip (tr ("%d will be replaced with opening folder"));

  ++row;
  layout->addWidget (new QLabel (tr ("Image cache size")), row, 0);
  layout->addWidget (imageCache_, row, 1);
  imageCache_->setRange (1, 100);
  imageCache_->setSuffix (tr (" Mb"));

  ++row;
  layout->addWidget (checkUpdates_, row, 0);

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

QString Settings::console () const
{
  return console_->text ();
}

void Settings::setConsole (const QString &console)
{
  console_->setText (console);
}

bool Settings::checkUpdates () const
{
  return checkUpdates_->isChecked ();
}

void Settings::setCheckUpdates (bool isOn)
{
  checkUpdates_->setChecked (isOn);
}

int Settings::imageCacheSizeKb () const
{
  return imageCache_->value () * 1024;;
}

void Settings::setImageCacheSize (int sizeKb)
{
  imageCache_->setValue (sizeKb / 1024);
}
