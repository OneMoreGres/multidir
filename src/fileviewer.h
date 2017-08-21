#pragma once

#include <QWidget>
#include <QFutureWatcher>

class QTextEdit;
class QSettings;

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

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

  QTextEdit *edit_;
  QFutureWatcher<QString> *watcher_;
};
