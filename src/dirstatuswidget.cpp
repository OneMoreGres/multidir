#include "dirstatuswidget.h"
#include "storagemanager.h"
#include "utils.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "debug.h"
#include "settingsmanager.h"
#include "constants.h"

#include <QFileInfo>
#include <QLabel>
#include <QBoxLayout>

DirStatusWidget::DirStatusWidget (ProxyModel *model, QWidget *parent) :
  QWidget (parent),
  path_ (),
  model_ (model),
  storage_ (new QLabel (this)),
  entries_ (new QLabel (this)),
  selection_ (new QLabel (this))
{
  connect (model_, &ProxyModel::currentChanged,
           this, &DirStatusWidget::updatePath);
  connect (model_, &ProxyModel::contentsChanged,
           this, &DirStatusWidget::updateEntries);

  {
    auto font = this->font ();
    font.setPointSize (font.pointSize () - 2);
    setFont (font);
  }

  storage_->setToolTip (tr ("Available/total space"));
  entries_->setToolTip (tr ("Total files/folders"));
  selection_->setToolTip (tr ("Selected files/folders"));

  auto layout = new QHBoxLayout (this);
  layout->addWidget (storage_);
  layout->addWidget (entries_);
  layout->addWidget (selection_);

  layout->setMargin (0);

  SettingsManager::subscribeForUpdates (this);
  updateSettings ();

  startTimer (5000);
}

void DirStatusWidget::updateSettings ()
{
  SettingsManager settings;
  storage_->setEnabled (settings.get (SettingsManager::ShowFreeSpace).toBool ());
  entries_->setEnabled (settings.get (SettingsManager::ShowFilesInfo).toBool ());
  selection_->setEnabled (settings.get (SettingsManager::ShowSelectionInfo).toBool ());

  storage_->setVisible (storage_->isEnabled ());
  entries_->setVisible (entries_->isEnabled ());
  selection_->setVisible (selection_->isEnabled ());

  if (storage_->isEnabled () || entries_->isEnabled () || selection_->isEnabled ())
  {
    show ();
  }
  else
  {
    hide ();
  }
}

void DirStatusWidget::updateSelection (const QList<QFileInfo> &selection)
{
  if (!selection_->isEnabled ())
  {
    return;
  }

  if (selection.isEmpty ())
  {
    selection_->clear ();
    return;
  }

  qint64 size = 0;
  auto files = 0;
  auto dirs = 0;
  for (const auto &i: selection)
  {
    const auto isDir = i.isDir ();
    files += int (!isDir);
    dirs += int (isDir);
    if (!isDir)
    {
      size += i.size ();
    }
  }
  selection_->setText (tr ("*%1/%2 (%3)").arg (files).arg (dirs)
                       .arg (utils::sizeString (size)));
}

void DirStatusWidget::updatePath ()
{
  path_ = model_->currentPath ();
  updateStorage ();
  updateEntries ();
  selection_->clear ();
}

void DirStatusWidget::updateStorage ()
{
  if (!storage_->isEnabled ())
  {
    return;
  }

  const auto storage = StorageManager::storage (path_);
  if (storage)
  {
    using namespace utils;
    const auto free = storage->bytesAvailable ();
    const auto total = storage->bytesTotal ();
    const auto percent = total > 0 ? free * 100. / total : 0.0;
    storage_->setText (tr ("%1/%2 (%3%)").arg (sizeString (free), sizeString (total))
                       .arg (percent, 0, 'f', 0));
  }
  else
  {
    storage_->clear ();
  }
}

void DirStatusWidget::updateEntries ()
{
  if (!entries_->isEnabled ())
  {
    return;
  }

  qint64 size = 0;
  auto files = 0;
  auto dirs = 0;
  for (auto i = 0, end = model_->count (); i < end; ++i)
  {
    const auto isDir = model_->isDir (i);
    files += int (!isDir);
    dirs += int (isDir);
    if (!isDir)
    {
      size += model_->fileSize (i);
    }
    else if (model_->fileName (i) == constants::dotdot)
    {
      --dirs;
    }
  }
  entries_->setText (tr ("#%1/%2 (%3)").arg (files).arg (dirs)
                     .arg (utils::sizeString (size)));
}

void DirStatusWidget::timerEvent (QTimerEvent */*event*/)
{
  updateStorage ();
}

#include "moc_dirstatuswidget.cpp"
