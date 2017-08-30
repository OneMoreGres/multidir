#pragma once

#include <QFileInfo>
#include <QUrl>

#include <atomic>

class FileConflictResolver;
class FileOperationModel;

class FileOperation : public QObject
{
Q_OBJECT
public:
  using Infos = QList<QFileInfo>;

  enum class Action
  {
    Copy, Move, Link, Remove, Trash
  };

  FileOperation ();

signals:
  void progress (int percent, FileOperation *operation);
  void finished (bool ok, FileOperation *operation);
  void currentChanged (const QString &name, FileOperation *operation);

private:
  friend class FileOperationModel;
  void startAsync (FileConflictResolver *resolver);
  void abort ();

  bool transfer (const Infos &sources, const QFileInfo &target, int depth);
  bool link (const Infos &sources, const QFileInfo &target);
  bool erase (const Infos &infos, int depth);

  int resolveConflict (const QFileInfo &source, const QFileInfo &target);
  void advance (qint64 size);
  void setCurrent (const QString &name);
  void finish (bool ok);

  bool copy (const QString &oldName, const QString &newName);
  bool rename (const QString &oldName, const QString &newName);

  Infos sources_;
  QFileInfo target_;
  Action action_;
  FileConflictResolver *resolver_;
  int allFileResolution_;
  int allDirResolution_;
  qint64 totalSize_;
  qint64 doneSize_;
  int progress_;
  std::atomic_bool isAborted_;
};
