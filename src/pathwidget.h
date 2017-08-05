#pragma once

#include <QWidget>
#include <QFileInfo>

class FileSystemModel;

class QLabel;
class QLineEdit;

class PathWidget : public QWidget
{
Q_OBJECT
public:
  PathWidget (FileSystemModel *model, QWidget *parent = nullptr);

  void setReadOnly (bool isReadOnly);
  void setIndex (const QString &index);
  void setPath (const QFileInfo &path);
  QString fullName (int preferredWidth) const;

  void edit ();
  void applyEdition ();
  void rejectEdition ();

  void adjust ();

  bool eventFilter (QObject *watched, QEvent *event) override;

signals:
  void pathChanged (const QFileInfo &newPath);

private:
  QString fittedPath (int maxWidth) const;
  void toggleEdition (bool isOn);

  bool isReadOnly_;
  QFileInfo path_;

  QLabel *indexLabel_;
  QLabel *pathLabel_;
  QLabel *dirLabel_;
  QLineEdit *pathEdit_;
};
