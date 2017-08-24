#pragma once

#include <QStringList>
#include <catch.hpp>

namespace Catch
{

template<>
inline std::string toString (const QStringList &s)
{
  return s.join (',').toStdString ();
}

}
