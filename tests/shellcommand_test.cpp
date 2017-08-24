#include "catch_ext.h"
#include "shellcommand.h"

namespace
{
class ShellCommandProxy : public ShellCommand
{
public:
  static QStringList parse (const QString &command) {return ShellCommand::parse (command);}
};
}

TEST_CASE ("parsing", "[shell command]")
{
  SECTION ("single")
  {
    QStringList expected {"test"};
    const auto actual = ShellCommandProxy::parse ("test");
    REQUIRE (expected == actual);
  }
  SECTION ("multiple")
  {
    QStringList expected {"test", "two", "three"};
    const auto actual = ShellCommandProxy::parse ("test two three");
    REQUIRE (expected == actual);
  }
  SECTION ("single quoted")
  {
    QStringList expected {"test", "two three"};
    const auto actual = ShellCommandProxy::parse ("test 'two three'");
    REQUIRE (expected == actual);
  }
  SECTION ("double quoted")
  {
    QStringList expected {"test", "two three"};
    const auto actual = ShellCommandProxy::parse ("test \"two three\"");
    REQUIRE (expected == actual);
  }
  SECTION ("mix quoted")
  {
    QStringList expected {"test", "two 'three'"};
    const auto actual = ShellCommandProxy::parse ("test \"two 'three'\"");
    REQUIRE (expected == actual);
  }
  SECTION ("mix quoted 2")
  {
    QStringList expected {"test", "two \"three\""};
    const auto actual = ShellCommandProxy::parse ("test 'two \"three\"'");
    REQUIRE (expected == actual);
  }
  SECTION ("single quoted")
  {
    QStringList expected {"test two three"};
    const auto actual = ShellCommandProxy::parse ("'test two three'");
    REQUIRE (expected == actual);
  }
  SECTION ("two quotes")
  {
    QStringList expected {{"test"}, {"two"}, {"three"}};
    const auto actual = ShellCommandProxy::parse ("test \"two\" \"three\"");
    REQUIRE (expected == actual);
  }
  SECTION ("space escaped")
  {
    QStringList expected {{"test"}, {"two three"}};
    const auto actual = ShellCommandProxy::parse ("test two\\ three");
    REQUIRE (expected == actual);
  }
  SECTION ("quoted escaped")
  {
    QStringList expected {{"test"}, {"two three"}};
    const auto actual = ShellCommandProxy::parse ("test 'two\\ three'");
    REQUIRE (expected == actual);
  }
}
