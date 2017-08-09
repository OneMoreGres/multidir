#pragma once

#include <QWidget>

class QFileInfo;

class QLabel;

class DirStatusWidget : public QWidget
{
Q_OBJECT
public:
  explicit DirStatusWidget (QWidget *parent = nullptr);
  DirStatusWidget (const DirStatusWidget &) = delete;
  DirStatusWidget &operator= (const DirStatusWidget &) = delete;

  void setPath (const QFileInfo &path);
  void setSelection (const QList<QFileInfo> &selection);

private:
  void updateStorage (const QFileInfo &path);
  void updateEntries (const QFileInfo &path);

  QLabel *storage_;
  QLabel *entries_;
  QLabel *selection_;
};
