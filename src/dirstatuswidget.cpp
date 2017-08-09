#include "dirstatuswidget.h"
#include "storagemanager.h"
#include "utils.h"

#include <QFileInfo>
#include <QLabel>
#include <QBoxLayout>

DirStatusWidget::DirStatusWidget (QWidget *parent) :
  QWidget (parent),
  storage_ (new QLabel (this)),
  entries_ (new QLabel (this)),
  selection_ (new QLabel (this))
{
  storage_->setToolTip (tr ("Available/total space"));
  entries_->setToolTip (tr ("Total files"));
  selection_->setToolTip (tr ("Selected files"));

  auto layout = new QHBoxLayout (this);
  layout->addWidget (storage_);
  layout->addWidget (entries_);
  layout->addWidget (selection_);

  layout->setMargin (0);
}

void DirStatusWidget::setPath (const QFileInfo &path)
{
  updateStorage (path);
  updateEntries (path);
  selection_->clear ();
}

void DirStatusWidget::setSelection (const QList<QFileInfo> &selection)
{
  if (!selection.isEmpty ())
  {
    using namespace utils;
    const auto size = std::accumulate (selection.cbegin (), selection.cend (), qint64 (0),
                                       [](qint64 sum, const QFileInfo &i) {
                                         return i.isFile () ? sum + i.size () : sum;
                                       });
    selection_->setText (tr ("*%1 (%2)").arg (selection.size ())
                         .arg (sizeString (size)));
  }
  else
  {
    selection_->clear ();
  }
}

void DirStatusWidget::updateStorage (const QFileInfo &path)
{
  const auto storage = StorageManager::storage (path);
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

void DirStatusWidget::updateEntries (const QFileInfo &path)
{
  const auto files = QDir (path.absoluteFilePath ()).entryInfoList (QDir::Files);
  const auto size = std::accumulate (files.cbegin (), files.cend (), qint64 (0),
                                     [](qint64 sum, const QFileInfo &i) {
                                       return sum + i.size ();
                                     });
  using namespace utils;
  entries_->setText (tr ("#%1 (%2)").arg (files.size ()).arg (sizeString (size)));
}

#include "moc_dirstatuswidget.cpp"
