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
const QString networkDirStart = QLatin1String ("\\\\");

const auto updateUrl = "https://github.com/OneMoreGres/multidir/raw/master/version";
const auto appName = "MultiDir";
const auto version = STR (APP_VERSION);
}
#undef STR
#undef STR2
