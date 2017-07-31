#pragma once

#include <QWidget>

class FileOperation;

class QProgressBar;

class FileOperationWidget : public QWidget
{
public:
  FileOperationWidget (QSharedPointer<FileOperation> operation, QWidget *parent = nullptr);
  FileOperationWidget (const FileOperationWidget &r) = delete;
  FileOperationWidget &operator= (const FileOperationWidget &r) = delete;

private:
  QProgressBar *progress_;
  QSharedPointer<FileOperation> operation_;
};
