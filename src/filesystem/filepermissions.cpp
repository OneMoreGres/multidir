#include "filepermissions.h"

#include <array>

namespace
{
using P = QFile::Permission;
const std::array<QFile::Permission, 9> permissionMap {
  {P::ReadUser, P::WriteUser, P::ExeUser,
   P::ReadGroup, P::WriteGroup, P::ExeGroup,
   P::ReadOther, P::WriteOther, P::ExeOther}
};
const size_t modeSize = 3; // User-Group-Other
const std::array<QChar, 3> charRule {{'r', 'w', 'x'}};
const std::array<uint, 3> numericRules {{4, 2, 1}};
}

QString FilePermissions::toNumericString (QFileDevice::Permissions permissions)
{
  QString result;
  for (size_t mode = 0; mode < modeSize; ++mode)
  {
    uint numeric = 0;
    for (size_t i = 0; i < numericRules.size (); ++i)
    {
      const auto mapIndex = numericRules.size () * mode + i;
      numeric += (permissions & permissionMap [mapIndex]) ? numericRules[i] : 0;
    }
    result += QString::number (numeric);
  }
  return result;
}

QString FilePermissions::toString (QFileDevice::Permissions permissions)
{
  QString result;
  for (size_t mode = 0; mode < modeSize; ++mode)
  {
    for (size_t i = 0; i < charRule.size (); ++i)
    {
      const auto mapIndex = charRule.size () * mode + i;
      result += (permissions & permissionMap [mapIndex]) ? charRule[i] : QLatin1Char ('-');
    }
  }
  return result;
}

QString FilePermissions::toFullString (QFileDevice::Permissions permissions)
{
  return toString (permissions)
         + QLatin1String (" (") + toNumericString (permissions) + QLatin1Char (')');
}

QFileDevice::Permissions FilePermissions::fromNumericString (const QString &text)
{
  QFileDevice::Permissions result;
  const size_t modeCount = std::min (modeSize, size_t (text.length ()));
  for (size_t mode = 0; mode < modeCount; ++mode)
  {
    auto numeric = text.mid (int (mode), 1).toUInt ();
    for (size_t i = 0; i < numericRules.size (); ++i)
    {
      if (numeric >= numericRules[i])
      {
        numeric -= numericRules[i];
        const auto mapIndex = numericRules.size () * mode + i;
        result |= permissionMap[mapIndex];
      }
    }
  }
  return result;
}

QFileDevice::Permissions FilePermissions::fromString (const QString &text)
{
  QFileDevice::Permissions result;
  size_t mode = 0;
  size_t ruleInMode = 0;
  for (auto c = 0, end = text.length (); c < end && mode < modeSize; ++c)
  {
    const auto ch = text.at (c);
    if (ch != QLatin1Char ('-'))
    {
      for (size_t i = 0; i < charRule.size (); ++i)
      {
        if (ch != charRule[i])
        {
          continue;
        }

        if (i < ruleInMode) // rule of next mode
        {
          --c; // recheck in next mode
          ruleInMode += charRule.size ();
        }
        else
        {
          const auto mapIndex = charRule.size () * mode + i;
          result |= permissionMap[mapIndex];
          ruleInMode = i;
        }
        break;
      }
    }

    if (++ruleInMode >= charRule.size ())
    {
      ++mode;
      ruleInMode = 0;
    }
  }
  return result;
}
