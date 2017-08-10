#include "dirstatuswidget.h"
#include "storagemanager.h"
#include "utils.h"
#include "proxymodel.h"
#include "filesystemmodel.h"
#include "debug.h"

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

  storage_->setToolTip (tr ("Available/total space"));
  entries_->setToolTip (tr ("Total files"));
  selection_->setToolTip (tr ("Selected files"));

  auto layout = new QHBoxLayout (this);
  layout->addWidget (storage_);
  layout->addWidget (entries_);
  layout->addWidget (selection_);

  layout->setMargin (0);
}

void DirStatusWidget::setSelection (const QList<QFileInfo> &selection)
{
  if (!selection.isEmpty ())
  {
    const auto size = std::accumulate (selection.cbegin (), selection.cend (), qint64 (0),
                                       [](qint64 sum, const QFileInfo &i) {
                                         return i.isFile () ? sum + i.size () : sum;
                                       });
    selection_->setText (tr ("*%1 (%2)").arg (selection.size ())
                         .arg (utils::sizeString (size)));
  }
  else
  {
    selection_->clear ();
  }
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
  qint64 size = 0;
  int count = 0;
  for (auto i = 0, end = model_->count (); i < end; ++i)
  {
    if (!model_->isDir (i))
    {
      ++count;
      size += model_->fileSize (i);
    }
  }
  entries_->setText (tr ("#%1 (%2)").arg (count).arg (utils::sizeString (size)));
}

#include "moc_dirstatuswidget.cpp"
