#include "settings.h"
#include "shortcutmanager.h"

#include <QGridLayout>
#include <QKeySequenceEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QStyledItemDelegate>

namespace
{
enum ShortcutColumn
{
  Id, Name, Context, Key, Count
};


class ShortcutDelegate : public QStyledItemDelegate
{
public:
  ShortcutDelegate (QWidget *parent) : QStyledItemDelegate (parent) {}

  QWidget * createEditor (QWidget *parent, const QStyleOptionViewItem & /*option*/,
                          const QModelIndex & /*index*/) const override
  {
    return new QKeySequenceEdit (parent);
  }
  void setEditorData (QWidget *editor, const QModelIndex &index) const override
  {
    if (auto edit = qobject_cast<QKeySequenceEdit *>(editor))
    {
      edit->setKeySequence (index.data ().toString ());
    }
  }
  void setModelData (QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override
  {
    if (auto edit = qobject_cast<QKeySequenceEdit *>(editor))
    {
      model->setData (index, edit->keySequence ().toString ());
    }
  }
};
}


Settings::Settings (QWidget *parent) :
  QDialog (parent),
  tabs_ (new QTabWidget (this)),
  console_ (new QLineEdit (this)),
  editor_ (new QLineEdit (this)),
  checkUpdates_ (new QCheckBox (tr ("Check for updates"), this)),
  imageCache_ (new QSpinBox (this)),
  shortcuts_ (new QTableWidget (this)),
  groupShortcuts_ (new QLineEdit (this)),
  tabShortcuts_ (new QLineEdit (this))
{
  setWindowTitle (tr ("Settings"));

  {
    auto tab = new QWidget (tabs_);
    tabs_->addTab (tab, tr ("General"));
    auto layout = new QGridLayout (tab);

    auto row = 0;
    layout->addWidget (new QLabel (tr ("Console command")), row, 0);
    layout->addWidget (console_, row, 1);
    console_->setToolTip (tr ("%d will be replaced with opening folder"));

    ++row;
    layout->addWidget (new QLabel (tr ("Default editor")), row, 0);
    layout->addWidget (editor_, row, 1);
    editor_->setToolTip (tr ("%p will be replaced with opening path"));

    ++row;
    layout->addWidget (new QLabel (tr ("Image cache size")), row, 0);
    layout->addWidget (imageCache_, row, 1);
    imageCache_->setRange (1, 100);
    imageCache_->setSuffix (tr (" Mb"));

    ++row;
    layout->addWidget (checkUpdates_, row, 0);

    ++row;
    layout->addItem (new QSpacerItem (1,1,QSizePolicy::Expanding, QSizePolicy::Expanding), row, 0);
  }

  {
    auto tab = new QWidget (tabs_);
    tabs_->addTab (tab, tr ("Shortcuts"));
    auto layout = new QGridLayout (tab);

    auto row = 0;
    layout->addWidget (shortcuts_, row, 0, 1, 2);
    shortcuts_->setColumnCount (ShortcutColumn::Count);
    shortcuts_->setHorizontalHeaderLabels ({tr ("Id"), tr ("Name"), tr ("Context"), tr ("Shortcut")});
    shortcuts_->hideColumn (ShortcutColumn::Id);
    loadShortcuts ();
    shortcuts_->resizeColumnsToContents ();
    shortcuts_->setSortingEnabled (true);
    shortcuts_->setSelectionMode (QTableWidget::SingleSelection);
    shortcuts_->setSelectionBehavior (QTableWidget::SelectRows);
    shortcuts_->setItemDelegateForColumn (ShortcutColumn::Key,new ShortcutDelegate (shortcuts_));


    ++row;
    layout->addWidget (new QLabel (tr ("Group ids:")), row, 0);
    layout->addWidget (groupShortcuts_, row, 1);
    groupShortcuts_->setToolTip (tr ("Each character represents ID part of group"
                                     " switch shortcut"));

    ++row;
    layout->addWidget (new QLabel (tr ("Tab ids:")), row, 0);
    layout->addWidget (tabShortcuts_, row, 1);
    tabShortcuts_->setToolTip (tr ("Each character represents ID part of tab"
                                   " switch shortcut"));
  }

  auto layout = new QVBoxLayout (this);
  layout->addWidget (tabs_);
  auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &QDialog::accept);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &Settings::saveShortcuts);
  connect (buttons, &QDialogButtonBox::rejected,
           this, &QDialog::reject);
  layout->addWidget (buttons);

  resize (400,300);
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

QString Settings::editor () const
{
  return editor_->text ();
}

void Settings::setEditor (const QString &editor)
{
  editor_->setText (editor);
}

QString Settings::groupShortcuts () const
{
  return groupShortcuts_->text ();
}

void Settings::setGroupShortcuts (const QString &value)
{
  groupShortcuts_->setText (value);
}

QString Settings::tabShortcuts () const
{
  return tabShortcuts_->text ();
}

void Settings::setTabShortcuts (const QString &value)
{
  tabShortcuts_->setText (value);
}

void Settings::loadShortcuts ()
{
  using SM = ShortcutManager;
  using Item = QTableWidgetItem;
  shortcuts_->setRowCount (SM::ShortcutCount);
  const auto nonEditable = [](const QString &text) {
                             auto item = new Item (text);
                             item->setFlags (item->flags () ^ Qt::ItemIsEditable);
                             return item;
                           };

  for (auto i = 0; i < SM::ShortcutCount; ++i)
  {
    const auto type = SM::Shortcut (i);
    shortcuts_->setItem (i, ShortcutColumn::Id, new Item (Item::UserType + i));
    shortcuts_->setItem (i, ShortcutColumn::Name, nonEditable (SM::name (type)));
    shortcuts_->setItem (i, ShortcutColumn::Context, nonEditable (SM::contextName (type)));
    shortcuts_->setItem (i, ShortcutColumn::Key, new Item (SM::get (type).toString ()));
  }
}

void Settings::saveShortcuts ()
{
  using SM = ShortcutManager;
  using Item = QTableWidgetItem;
  for (auto i = 0; i < SM::ShortcutCount; ++i)
  {
    auto idItem = shortcuts_->item (i, ShortcutColumn::Id);
    const auto type = SM::Shortcut (idItem->type () - Item::UserType);
    SM::set (type, shortcuts_->item (i, ShortcutColumn::Key)->text ());
  }
}
