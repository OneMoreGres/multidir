#pragma once

#include <QString>

class QStatusBar;

class Notifier
{
public:
  static void setMain (QStatusBar *bar);
  static void error (const QString &text);

private:
  static QStatusBar *bar_;
};
