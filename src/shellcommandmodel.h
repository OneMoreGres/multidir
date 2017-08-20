#pragma once

#include <QAbstractListModel>
#include <QColor>
#include <QFileInfo>

class ShellCommandWidget;
class ShellCommand;

class ShellCommandModel : public QAbstractListModel
{
Q_OBJECT
public:
  struct Selection
  {
    QFileInfo path;
    QFileInfo current;
    QList<QFileInfo> selected;
  };

  explicit ShellCommandModel (QObject *parent = nullptr);
  ~ShellCommandModel ();

  bool run (const QString &command, const QMap<QString, Selection> &selectionPerIndex,
            const QFileInfo &workingDir);
  void openConsole (const QFileInfo &path);
  void openInEditor (const QFileInfo &path, const QFileInfo &workingDir);

  int rowCount (const QModelIndex &parent) const override;
  QVariant data (const QModelIndex &index, int role) const override;

  void show (const QModelIndex &index) const;

signals:
  void filled ();
  void emptied ();

public slots:
  void updateSettings ();

private:
  bool run (const ShellCommand &command);
  QModelIndex toIndex (ShellCommandWidget *widget) const;
  ShellCommandWidget * toWidget (const QModelIndex &index) const;

  void add (ShellCommandWidget *widget);
  void update (ShellCommandWidget *widget);
  void remove (QObject *widget);

  QVector<ShellCommandWidget *> widgets_;

  QString commandWrapper_;
  QString openConsoleCommand_;
  QString editorCommand_;

  QColor runningColor_;
  QColor finishedColor_;
  QColor erroredColor_;
};
