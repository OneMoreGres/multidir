#include "catch.hpp"
#include "filepermissions.h"

namespace
{
using FP = FilePermissions;

const auto ru = QFile::Permission::ReadUser;
const auto wu = QFile::Permission::WriteUser;
const auto xu = QFile::Permission::ExeUser;

const auto rg = QFile::Permission::ReadGroup;
const auto wg = QFile::Permission::WriteGroup;
const auto xg = QFile::Permission::ExeGroup;

const auto ro = QFile::Permission::ReadOther;
const auto wo = QFile::Permission::WriteOther;
const auto xo = QFile::Permission::ExeOther;
}

TEST_CASE ("serialization/deserialization", "[file permission]")
{
  SECTION ("rwx str")
  {
    const auto original = ru | wu | xu;
    const auto str = FP::toString (original);
    REQUIRE (str == "rwx------");
    REQUIRE (FP::fromString (str) == original);
  }
  SECTION ("rwx num")
  {
    const auto original = ru | wu | xu;
    const auto num = FP::toNumericString (original);
    REQUIRE (num == "700");
    REQUIRE (FP::fromNumericString (num) == original);
  }

  SECTION ("rwxrwxrwx str")
  {
    const auto original = ru | wu | xu | rg | wg | xg | ro | wo | xo;
    const auto str = FP::toString (original);
    REQUIRE (str == "rwxrwxrwx");
    REQUIRE (FP::fromString (str) == original);
  }
  SECTION ("rwxrwxrwx num")
  {
    const auto original = ru | wu | xu | rg | wg | xg | ro | wo | xo;
    const auto num = FP::toNumericString (original);
    REQUIRE (num == "777");
    REQUIRE (FP::fromNumericString (num) == original);
  }

  SECTION ("rrr str")
  {
    const auto original = ru | rg | ro;
    const auto str = FP::toString (original);
    REQUIRE (str == "r--r--r--");
    REQUIRE (FP::fromString (str) == original);
  }
  SECTION ("rrr num")
  {
    const auto original = ru | rg | ro;
    const auto num = FP::toNumericString (original);
    REQUIRE (num == "444");
    REQUIRE (FP::fromNumericString (num) == original);
  }
}

TEST_CASE ("just parsing", "[file permission]")
{
  SECTION ("r")
  {
    const auto expected = ru;
    REQUIRE (FP::fromString ("r") == expected);
  }
  SECTION ("-r")
  {
    const auto expected = rg;
    REQUIRE (FP::fromString ("-r") == expected);
  }
  SECTION ("--ww")
  {
    const auto expected = wg | wo;
    REQUIRE (FP::fromString ("--ww") == expected);
  }
  SECTION ("www")
  {
    const auto expected = wu | wg | wo;
    REQUIRE (FP::fromString ("www") == expected);
  }
  SECTION ("xxx")
  {
    const auto expected = xu | xg | xo;
    REQUIRE (FP::fromString ("xxx") == expected);
  }
  SECTION ("rwrr")
  {
    const auto expected = ru | wu | rg | ro;
    REQUIRE (FP::fromString ("rwrr") == expected);
  }
  SECTION ("")
  {
    const auto expected = QFile::Permissions ();
    REQUIRE (FP::fromString ("") == expected);
  }
  SECTION ("wrwrwr")
  {
    const auto expected = wu | rg | wg | ro | wo;
    REQUIRE (FP::fromString ("wrwrwr") == expected);
  }

  SECTION ("6")
  {
    const auto expected = ru | wu;
    REQUIRE (FP::fromNumericString ("6") == expected);
  }
  SECTION ("06")
  {
    const auto expected = rg | wg;
    REQUIRE (FP::fromNumericString ("06") == expected);
  }
  SECTION ("2")
  {
    const auto expected = wu;
    REQUIRE (FP::fromNumericString ("2") == expected);
  }
}
