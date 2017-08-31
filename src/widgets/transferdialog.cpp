#include "transferdialog.h"
#include "debug.h"
#include "utils.h"
#include "filesystemcompleter.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>

TransferDialog::TransferDialog (FileSystemModel *model, QWidget *parent) :
  QDialog (parent),
  title_ (new QLabel (this)),
  destination_ (new QLineEdit (this)),
  buttons_ (new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
{
  auto completer = new FileSystemCompleter (model, this);
  destination_->setCompleter (completer);

  auto layout = new QFormLayout (this);
  layout->addRow (title_);
  layout->addRow (tr ("To:"), destination_);
  layout->addRow (buttons_);

  connect (buttons_, &QDialogButtonBox::accepted,
           this, &QDialog::accept);
  connect (buttons_, &QDialogButtonBox::rejected,
           this, &QDialog::reject);
}

int TransferDialog::prompt (Qt::DropAction action, const QList<QFileInfo> &sources)
{
  ASSERT (!sources.isEmpty ());
  const auto actionText = QMap<Qt::DropAction, QString>{
    {Qt::CopyAction, tr ("Copy")},
    {Qt::MoveAction, tr ("Move")},
    {Qt::LinkAction, tr ("Link")}
  }.value (action);
  ASSERT (!actionText.isEmpty ());

  const auto names = utils::fileNames (sources);

  title_->setText (actionText + ' ' + names);
  destination_->setText (sources.first ().absolutePath ());
  destination_->selectAll ();
  destination_->setFocus ();
  return exec ();
}

QString TransferDialog::destination () const
{
  return destination_->text ();
}
