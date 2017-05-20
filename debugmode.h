#pragma once

#include <QDebug>

#include <assert.h>

#define ASSERT(XXX) if (!(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX; \
    assert (XXX); \
}

#define ASSERT_X(XXX, CONTEXT) if (!(XXX)) { \
    qCritical () << "Assertion failed at" << __FILE__ << __LINE__ << ":" << #XXX << \
      "Context (" << #CONTEXT << ")" << CONTEXT; \
    assert (XXX); \
}

namespace debug_mode
{

void setEnabled (bool isOn);

}
