#include "styleoptionsproxy.h"
#include "debug.h"

#include <QApplication>

namespace
{
StyleOptionsProxy *instance_ = nullptr;
}

void StyleOptionsProxy::init ()
{
  instance_ = new StyleOptionsProxy;
}

StyleOptionsProxy &StyleOptionsProxy::instance ()
{
  return *instance_;
}


StyleOptionsProxy::StyleOptionsProxy (QWidget *parent) :
  QWidget (parent)
{
  setObjectName ("style");
  setDefaults ();
}

void StyleOptionsProxy::setDefaults ()
{
  activeGlowColor_ = QColor (0,153,204);
  currentRowColor_ = QColor (Qt::cyan);
  dirColor_ = QColor (204,255,255);
  inaccessibleDirColor_ = QColor (255,153,153);
  executableColor_ = QColor (255,204,153);
  unreadableFileColor_ = QColor (255,153,153);
  stdErrOutputColor_ = Qt::darkRed;
  runningCommandColor_ = Qt::yellow;
  finishedCommandColor_ = Qt::green;
  erroredCommandColor_ = Qt::red;
}

QColor StyleOptionsProxy::currentRowColor () const
{
  return currentRowColor_;
}

void StyleOptionsProxy::setCurrentRowColor (const QColor &color)
{
  currentRowColor_ = color;
}

QColor StyleOptionsProxy::erroredCommandColor () const
{
  return erroredCommandColor_;
}

void StyleOptionsProxy::setErroredCommandColor (const QColor &color)
{
  erroredCommandColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::finishedCommandColor () const
{
  return finishedCommandColor_;
}

void StyleOptionsProxy::setFinishedCommandColor (const QColor &color)
{
  finishedCommandColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::runningCommandColor () const
{
  return runningCommandColor_;
}

void StyleOptionsProxy::setRunningCommandColor (const QColor &color)
{
  runningCommandColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::stdErrOutputColor () const
{
  return stdErrOutputColor_;
}

void StyleOptionsProxy::setStdErrOutputColor (const QColor &color)
{
  stdErrOutputColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::unreadableFileColor () const
{
  return unreadableFileColor_;
}

void StyleOptionsProxy::setUnreadableFileColor (const QColor &color)
{
  unreadableFileColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::activeGlowColor () const
{
  return activeGlowColor_;
}

void StyleOptionsProxy::setActiveGlowColor (const QColor &color)
{
  activeGlowColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::executableColor () const
{
  return executableColor_;
}

void StyleOptionsProxy::setExecutableColor (const QColor &color)
{
  executableColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::inaccessibleDirColor () const
{
  return inaccessibleDirColor_;
}

void StyleOptionsProxy::setInaccessibleDirColor (const QColor &color)
{
  inaccessibleDirColor_ = color;
  emit changed ();
}

QColor StyleOptionsProxy::dirColor () const
{
  return dirColor_;
}

void StyleOptionsProxy::setDirColor (const QColor &color)
{
  dirColor_ = color;
  emit changed ();
}

void StyleOptionsProxy::changeEvent (QEvent *event)
{
  if (event->type () == QEvent::StyleChange && qApp->styleSheet ().isEmpty ())
  {
    setDefaults ();
    emit changed ();
  }
}

#include "moc_styleoptionsproxy.cpp"
