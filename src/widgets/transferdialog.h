#pragma once

#include <QDialog>

class FileSystemModel;

class QFileInfo;
class QLabel;
class QLineEdit;
class QDialogButtonBox;

class TransferDialog : public QDialog
{
public:
  TransferDialog (FileSystemModel *model, QWidget *parent = nullptr);

  int prompt (Qt::DropAction action, const QList<QFileInfo> &sources);
  QString destination () const;

private:
  FileSystemModel *model_;

  QLabel *title_;
  QLineEdit *destination_;
  QDialogButtonBox *buttons_;
};
