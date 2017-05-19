#pragma once

#include <QString>

#define STR2(XXX) #XXX
#define STR(XXX) STR2 (XXX)

namespace constants
{
const int iconSize = 32;
const int iconMinSize = 16;
const int listItemWidth = 120;
const int itemVerticalMargins = 8;

const QString dotdot = "..";

const auto updateUrl = "https://cdn.rawgit.com/OneMoreGres/multidir/master/version";
const auto version = STR (APP_VERSION);
}
#undef STR
#undef STR2
