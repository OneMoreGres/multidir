#pragma once

class QFileInfo;
class QMenu;

class OpenWith
{
public:
  static void init ();
  static void popupateMenu (QMenu &menu, const QFileInfo &file);
};
