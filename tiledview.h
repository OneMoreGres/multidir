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

protected:
  void mousePressEvent (QMouseEvent *event) override;
  void mouseReleaseEvent (QMouseEvent *event) override;
  void mouseMoveEvent (QMouseEvent *event) override;
  void dragEnterEvent (QDragEnterEvent *event) override;
  void dragMoveEvent (QDragMoveEvent *event) override;
  void dropEvent (QDropEvent *event) override;

private:
  using Base = QSplitter;
  using Base::addWidget;
  using Base::widget;
  using Base::count;
  using Base::insertWidget;
  using Base::indexOf;

  enum class Zone
  {
    None, Left, Right, Top, Bottom, Center
  };

  //! Calculate drop zone.
  Zone dropZone (const QPoint &pos, const QSize &size) const;
  //! Add to layout in given zone of target widget.
  void insert (QWidget *widget, Zone dropZone, QWidget *target);
  //! Remove from layout.
  void take (QWidget *widget);
  //! Swap widgets.
  void swap (QWidget *first, QWidget *second);

  TiledView * cast (QWidget *widget) const;
  QWidget * biggest () const;
  QWidget * childAt (const QPoint &pos) const;

  QPoint dragStartPos_;
};
