#pragma once

#include <QWidget>

class StyleOptionsProxy : public QWidget
{
Q_OBJECT
Q_PROPERTY (QColor activeGlowColor READ activeGlowColor WRITE setActiveGlowColor)
Q_PROPERTY (QColor dirColor READ dirColor WRITE setDirColor)
Q_PROPERTY (QColor inaccessibleDirColor READ inaccessibleDirColor WRITE setInaccessibleDirColor)
Q_PROPERTY (QColor executableColor READ executableColor WRITE setExecutableColor)
Q_PROPERTY (QColor unreadableFileColor READ unreadableFileColor WRITE setUnreadableFileColor)
Q_PROPERTY (QColor fileColor READ fileColor WRITE setFileColor)

public:
  static void init ();
  static StyleOptionsProxy &instance ();

  QColor activeGlowColor () const;
  void setActiveGlowColor (const QColor &color);

  QColor dirColor () const;
  void setDirColor (const QColor &color);

  QColor inaccessibleDirColor () const;
  void setInaccessibleDirColor (const QColor &color);

  QColor executableColor () const;
  void setExecutableColor (const QColor &color);

  QColor unreadableFileColor () const;
  void setUnreadableFileColor (const QColor &color);

  QColor fileColor () const;
  void setFileColor (const QColor &color);

signals:
  void changed ();

protected:
  void changeEvent (QEvent *event) override;

private:
  explicit StyleOptionsProxy (QWidget *parent = nullptr);
  void setDefaults ();

  QColor activeGlowColor_;
  QColor dirColor_;
  QColor inaccessibleDirColor_;
  QColor executableColor_;
  QColor unreadableFileColor_;
  QColor fileColor_;
};
