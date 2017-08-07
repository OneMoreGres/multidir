#pragma once

#include <QWidget>
#include <QFutureWatcher>

class QTextEdit;

class Viewer : public QWidget
{
Q_OBJECT
public:
  explicit Viewer (QWidget *parent = nullptr);
  ~Viewer ();

  bool setFile (const QString &name);
  void showFile (const QString &name);

private:
  void readInBackground (const QString &name);
  void setFileContents ();

  QTextEdit *edit_;
  QFutureWatcher<QString> *watcher_;
};
