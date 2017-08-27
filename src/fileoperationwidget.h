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

protected:
  void timerEvent (QTimerEvent *event) override;

private:
  void setCurrent (const QString &current);

  QProgressBar *progress_;
  QSharedPointer<FileOperation> operation_;
  QString template_;
  QString current_;
};
