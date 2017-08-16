#pragma once

#include <QObject>

class QFileInfo;
class QAction;

class NavigationHistory : public QObject
{
Q_OBJECT
public:
  explicit NavigationHistory (QObject *parent = nullptr);

  void addPath (const QFileInfo &path);
  void clear ();

  QAction * forwardAction () const;
  QAction * backwardAction () const;

signals:
  void pathChanged (const QFileInfo &path);

private:
  void moveForward ();
  void moveBackward ();

  QAction *forward_;
  QAction *backward_;
  QList<QFileInfo> history_;
  int currentIndex_;
};
