#include "settingseditor.h"
#include "shortcutmanager.h"
#include "translationloader.h"
#include "settingsmanager.h"
#include "debug.h"

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
#include <QSettings>
#include <QComboBox>
#include <QPixmapCache>

namespace
{
enum ShortcutColumn
{
  Id, Name, Context, Key, IsGlobal, Count
};

const QString qs_size = "settings/size";


class ShortcutDelegate : public QStyledItemDelegate
{
public:
  explicit ShortcutDelegate (QWidget *parent) : QStyledItemDelegate (parent) {}

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


SettingsEditor::SettingsEditor (QWidget *parent) :
  QDialog (parent),
  tabs_ (new QTabWidget (this)),
  console_ (new QLineEdit (this)),
  editor_ (new QLineEdit (this)),
  checkUpdates_ (new QCheckBox (tr ("Check for updates"), this)),
  startInBackground_ (new QCheckBox (tr ("Start in background"), this)),
  imageCache_ (new QSpinBox (this)),
  languages_ (new QComboBox (this)),
  shortcuts_ (new QTableWidget (this)),
  groupShortcuts_ (new QLineEdit (this)),
  tabShortcuts_ (new QLineEdit (this)),
  showFreeSpace_ (new QCheckBox (tr ("Show free space"), this)),
  showFilesInfo_ (new QCheckBox (tr ("Show files info"), this)),
  showSelectionInfo_ (new QCheckBox (tr ("Show selection info"), this)),
  editorToSettings_ ()
{
  setWindowTitle (tr ("Settings"));

  init ();

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
    imageCache_->setRange (1, 500);
    imageCache_->setSuffix (tr (" Mb"));

    ++row;
    layout->addWidget (checkUpdates_, row, 0);
    layout->addWidget (startInBackground_, row, 1);

    ++row;
    layout->addWidget (new QLabel (tr ("Language")), row, 0);
    layout->addWidget (languages_, row, 1);
    languages_->setToolTip (tr ("Restart required"));

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
    shortcuts_->setHorizontalHeaderLabels ({tr ("Id"), tr ("Name"), tr ("Context"),
                                            tr ("Shortcut"), tr ("Global")});
    shortcuts_->hideColumn (ShortcutColumn::Id);
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

  {
    auto tab = new QWidget (tabs_);
    tabs_->addTab (tab, tr ("View"));
    auto layout = new QGridLayout (tab);

    auto row = 0;
    layout->addWidget (showFreeSpace_, row, 0);

    ++row;
    layout->addWidget (showFilesInfo_, row, 0);

    ++row;
    layout->addWidget (showSelectionInfo_, row, 0);

    ++row;
    layout->addItem (new QSpacerItem (1,1,QSizePolicy::Expanding, QSizePolicy::Expanding), row, 0);
  }


  auto layout = new QVBoxLayout (this);
  layout->addWidget (tabs_);
  auto buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect (buttons, &QDialogButtonBox::accepted,
           this, &QDialog::accept);
  connect (buttons, &QDialogButtonBox::rejected,
           this, &QDialog::reject);
  layout->addWidget (buttons);

  connect (this, &QDialog::accepted,
           this, &SettingsEditor::save);

  load ();

  QSettings qsettings;
  restoreState (qsettings);
}

SettingsEditor::~SettingsEditor ()
{
  QSettings qsettings;
  saveState (qsettings);
}

void SettingsEditor::init ()
{
  using S = SettingsManager;
  editorToSettings_[console_] = S::ConsoleCommand;
  editorToSettings_[editor_] = S::EditorCommand;
  editorToSettings_[checkUpdates_] = S::CheckUpdates;
  editorToSettings_[startInBackground_] = S::StartInBackground;
  editorToSettings_[imageCache_] = S::ImageCacheSize;

  editorToSettings_[groupShortcuts_] = S::GroupIds;
  editorToSettings_[tabShortcuts_] = S::TabIds;

  editorToSettings_[showFreeSpace_] = S::ShowFreeSpace;
  editorToSettings_[showFilesInfo_] = S::ShowFilesInfo;
  editorToSettings_[showSelectionInfo_] = S::ShowSelectionInfo;
}

void SettingsEditor::saveState (QSettings &settings) const
{
  settings.setValue (qs_size, size ());
}

void SettingsEditor::restoreState (QSettings &settings)
{
  const auto size = settings.value (qs_size).toSize ();
  if (!size.isEmpty ())
  {
    resize (size);
  }
}

void SettingsEditor::initOrphanSettings ()
{
  TranslationLoader::load ();

  QSettings qsettings;
  ShortcutManager::setDefaults ();
  ShortcutManager::restore (qsettings);

  SettingsManager settings;
  QPixmapCache::setCacheLimit (std::max (1, settings.get (SettingsManager::ImageCacheSize).toInt ()));
}

void SettingsEditor::updateOrphanSettings ()
{
  SettingsManager settings;
  QPixmapCache::setCacheLimit (std::max (1, settings.get (SettingsManager::ImageCacheSize).toInt ()));

}

void SettingsEditor::loadShortcuts ()
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
    shortcuts_->setItem (i, ShortcutColumn::IsGlobal,
                         nonEditable (SM::isGlobal (type) ? tr ("Yes") : QLatin1String ("")));
  }
  shortcuts_->resizeColumnsToContents ();
}

void SettingsEditor::saveShortcuts ()
{
  using SM = ShortcutManager;
  using Item = QTableWidgetItem;
  for (auto i = 0; i < SM::ShortcutCount; ++i)
  {
    auto idItem = shortcuts_->item (i, ShortcutColumn::Id);
    const auto type = SM::Shortcut (idItem->type () - Item::UserType);
    SM::set (type, shortcuts_->item (i, ShortcutColumn::Key)->text ());
  }

  QSettings qsettings;
  ShortcutManager::save (qsettings);
}

void SettingsEditor::loadLanguage ()
{
  languages_->addItems (TranslationLoader::availableLanguages ());
  languages_->setCurrentText (TranslationLoader::language ());
}

void SettingsEditor::saveLanguage ()
{
  TranslationLoader::setLanguage (languages_->currentText ());
}

void SettingsEditor::load ()
{
  loadShortcuts ();
  loadLanguage ();

  SettingsManager settings;
  using Type = SettingsManager::Type;
  for (auto i = editorToSettings_.begin (), end = editorToSettings_.end (); i != end; ++i)
  {
    if (auto casted = qobject_cast<QCheckBox *>(i.key ()))
    {
      casted->setChecked (settings.get (Type (i.value ())).toBool ());
    }
    else if (auto casted = qobject_cast<QLineEdit *>(i.key ()))
    {
      casted->setText (settings.get (Type (i.value ())).toString ());
    }
    else if (auto casted = qobject_cast<QSpinBox *>(i.key ()))
    {
      casted->setValue (settings.get (Type (i.value ())).toInt ());
    }
  }
}

void SettingsEditor::save ()
{
  saveShortcuts ();
  saveLanguage ();

  SettingsManager settings;
  using Type = SettingsManager::Type;
  for (auto i = editorToSettings_.cbegin (), end = editorToSettings_.cend (); i != end; ++i)
  {
    if (auto casted = qobject_cast<const QCheckBox *>(i.key ()))
    {
      settings.set (Type (i.value ()), casted->isChecked ());
    }
    else if (auto casted = qobject_cast<QLineEdit *>(i.key ()))
    {
      settings.set (Type (i.value ()), casted->text ());
    }
    else if (auto casted = qobject_cast<QSpinBox *>(i.key ()))
    {
      settings.set (Type (i.value ()), casted->value ());
    }
  }

  updateOrphanSettings ();

  settings.triggerUpdate ();
}

#include "moc_settingseditor.cpp"
