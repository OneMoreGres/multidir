#pragma once

#include <QDebug>

#include <assert.h>
#include <atomic>

#define ASSERT(XXX) if (debug::isOn && !(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX; \
    assert (XXX); \
}

#define ASSERT_X(XXX, CONTEXT) if (debug::isOn && !(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX << \
      "Context (" << #CONTEXT << ")" << CONTEXT; \
    assert (XXX); \
}

#define LDEBUG if (debug::isOn) qDebug
#define LDEBUG_IF(XXX) if (XXX) qDebug ()
#define LWARNING qWarning
#define LWARNING_IF(XXX) if (XXX) qWarning ()
#define LERROR qCritical
#define LERROR_IF(XXX) if (XXX) qCritical ()
#define LINFO qInfo
#define LINFO_IF(XXX) if (XXX) qInfo ()
#define LARG(XXX) '(' << XXX << ')'

namespace debug
{

extern std::atomic_bool isOn;
void setDebugMode (bool isOn);

}
