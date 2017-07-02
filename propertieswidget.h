#pragma once

#include <QWidget>

class QFileInfo;

class PropertiesWidget : public QWidget
{
Q_OBJECT
public:
  PropertiesWidget (const QFileInfo &info, QWidget *parent = nullptr);
};
