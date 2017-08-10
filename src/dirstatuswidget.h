#pragma once

#include <QWidget>
#include <QFileInfo>

class ProxyModel;

class QFileInfo;
class QLabel;

class DirStatusWidget : public QWidget
{
Q_OBJECT
public:
  DirStatusWidget (ProxyModel *model, QWidget *parent = nullptr);
  DirStatusWidget (const DirStatusWidget &) = delete;
  DirStatusWidget &operator= (const DirStatusWidget &) = delete;

  void updateSelection (const QList<QFileInfo> &selection);

private:
  void updatePath ();
  void updateStorage ();
  void updateEntries ();

  QFileInfo path_;
  ProxyModel *model_;
  QLabel *storage_;
  QLabel *entries_;
  QLabel *selection_;
};
