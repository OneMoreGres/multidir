#pragma once

#include <QWidget>
#include <QFutureWatcher>

class QTextEdit;

class FileViewer : public QWidget
{
Q_OBJECT
public:
  explicit FileViewer (QWidget *parent = nullptr);
  ~FileViewer ();

  bool setFile (const QString &name);
  void showFile (const QString &name);

private:
  void readInBackground (const QString &name);
  void setFileContents ();

  QTextEdit *edit_;
  QFutureWatcher<QString> *watcher_;
};
