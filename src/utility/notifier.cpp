#include "notifier.h"
#include "debug.h"

#include <QStatusBar>

QStatusBar *Notifier::bar_ = nullptr;

void Notifier::setMain (QStatusBar *bar)
{
  bar_ = bar;
}

void Notifier::error (const QString &text)
{
  LERROR () << text;
  const auto timeout = 4000;
  QMetaObject::invokeMethod (bar_, "showMessage",
                             Q_ARG (QString, text), Q_ARG (int, timeout));
}
