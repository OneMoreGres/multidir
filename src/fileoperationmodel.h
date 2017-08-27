#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QSharedPointer>

#include <memory>
#include <vector>

class FileOperation;
class FileConflictResolver;

class FileOperationModel : public QAbstractListModel
{
Q_OBJECT
public:
  class Bundle
  {
  public:
    Bundle (std::unique_ptr<FileOperation> &&operation);

    QString target;
    QString current;
    int action;
    int progress;

  private:
    friend class FileOperationModel;
    std::unique_ptr<FileOperation> operation;
  };


  explicit FileOperationModel (QObject *parent = nullptr);

  void paste (const QList<QUrl> &urls, const QFileInfo &target, Qt::DropAction action);
  void remove (const QList<QFileInfo> &infos);
  void trash (const QList<QFileInfo> &infos);

  int rowCount (const QModelIndex &parent) const override;
  QVariant data (const QModelIndex &index, int role) const override;

  void abort (const QModelIndex &index);

signals:
  void filled ();
  void emptied ();

private:
  Bundle * toBundle (const QModelIndex &index);
  const Bundle * toBundle (const QModelIndex &index) const;
  QModelIndex toIndex (FileOperation *operation) const;

  void setCurrentFile (const QString &current, FileOperation *operation);
  void setCurrentProgress (int progress, FileOperation *operation);
  void setFinished (bool ok, FileOperation *operation);

  void add (const QList<QFileInfo> &sources, const QFileInfo &target, int action);
  void remove (FileOperation *operation);

  std::vector<Bundle> operations_;
  std::unique_ptr<FileConflictResolver> conflictResolver_;
};

Q_DECLARE_METATYPE (const FileOperationModel::Bundle *)
