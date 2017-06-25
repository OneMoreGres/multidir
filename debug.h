#pragma once

#include <QDebug>

#include <assert.h>

#define ASSERT(XXX) if (debug::isOn && !(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX; \
    assert (XXX); \
}

#define ASSERT_X(XXX, CONTEXT) if (debug::isOn && !(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX << \
      "Context (" << #CONTEXT << ")" << CONTEXT; \
    assert (XXX); \
}

#define DEBUG if (debug::isOn) qDebug
#define DEBUG_IF(XXX) if (XXX) qDebug ()
#define WARNING qWarning
#define WARNING_IF(XXX) if (XXX) qWarning ()
#define ERROR qCritical
#define ERROR_IF(XXX) if (XXX) qCritical ()
#define INFO qInfo
#define INFO_IF(XXX) if (XXX) qInfo ()
#define LARG(XXX) '(' << XXX << ')'

namespace debug
{

extern std::atomic_bool isOn;
void setDebugMode (bool isOn);

}
