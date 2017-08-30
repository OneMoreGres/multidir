#pragma once

#include <QWidget>

class StyleOptionsProxy : public QWidget
{
Q_OBJECT
Q_PROPERTY (QColor activeGlowColor READ activeGlowColor WRITE setActiveGlowColor)
Q_PROPERTY (QColor currentRowColor READ currentRowColor WRITE setCurrentRowColor)
Q_PROPERTY (QColor dirColor READ dirColor WRITE setDirColor)
Q_PROPERTY (QColor inaccessibleDirColor READ inaccessibleDirColor WRITE setInaccessibleDirColor)
Q_PROPERTY (QColor executableColor READ executableColor WRITE setExecutableColor)
Q_PROPERTY (QColor unreadableFileColor READ unreadableFileColor WRITE setUnreadableFileColor)
Q_PROPERTY (QColor stdErrOutputColor READ stdErrOutputColor WRITE setStdErrOutputColor)
Q_PROPERTY (QColor runningCommandColor READ runningCommandColor WRITE setRunningCommandColor)
Q_PROPERTY (QColor finishedCommandColor READ finishedCommandColor WRITE setFinishedCommandColor)
Q_PROPERTY (QColor erroredCommandColor READ erroredCommandColor WRITE setErroredCommandColor)

public:
  static void init ();
  static StyleOptionsProxy &instance ();

  QColor activeGlowColor () const;
  void setActiveGlowColor (const QColor &color);

  QColor currentRowColor () const;
  void setCurrentRowColor (const QColor &color);

  QColor dirColor () const;
  void setDirColor (const QColor &color);

  QColor inaccessibleDirColor () const;
  void setInaccessibleDirColor (const QColor &color);

  QColor executableColor () const;
  void setExecutableColor (const QColor &color);

  QColor unreadableFileColor () const;
  void setUnreadableFileColor (const QColor &color);

  QColor stdErrOutputColor () const;
  void setStdErrOutputColor (const QColor &color);

  QColor runningCommandColor () const;
  void setRunningCommandColor (const QColor &color);

  QColor finishedCommandColor () const;
  void setFinishedCommandColor (const QColor &color);

  QColor erroredCommandColor () const;
  void setErroredCommandColor (const QColor &color);

signals:
  void changed ();

protected:
  void changeEvent (QEvent *event) override;

private:
  explicit StyleOptionsProxy (QWidget *parent = nullptr);
  void setDefaults ();

  QColor activeGlowColor_;

  QColor currentRowColor_;
  QColor dirColor_;
  QColor inaccessibleDirColor_;
  QColor executableColor_;
  QColor unreadableFileColor_;

  QColor stdErrOutputColor_;

  QColor runningCommandColor_;
  QColor finishedCommandColor_;
  QColor erroredCommandColor_;
};
