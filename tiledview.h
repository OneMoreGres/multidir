#pragma once

#include <QSplitter>

class QSettings;
class QSplitter;

class TiledView : public QSplitter
{
Q_OBJECT
public:
  explicit TiledView (QWidget *parent = 0);
  ~TiledView ();

  void add (QWidget &widget);
  void remove (QWidget &widget);
  QList<QWidget *> widgets () const;

  void save (QSettings &settings) const;
  void restore (QSettings &settings);

signals:
  void tileSwapped ();

private:
  using Base = QSplitter;
  using Base::addWidget;
  using Base::widget;
  using Base::count;
  using Base::insertWidget;
  using Base::indexOf;

  TiledView * cast (QWidget *widget) const;
  QWidget * biggest () const;

};
