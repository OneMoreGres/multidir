#pragma once

#include <QFileInfo>
#include <QUrl>

#include <atomic>

class FileConflictResolver;

class FileOperation : public QObject
{
Q_OBJECT
public:
  using Ptr = QSharedPointer<FileOperation>;
  using Infos = QList<QFileInfo>;

  enum class Action
  {
    Copy, Move, Link, Remove, Trash
  };

  FileOperation ();

  const QList<QFileInfo> &sources () const;
  const QFileInfo &target () const;
  const Action &action () const;

  void startAsync (FileConflictResolver *resolver);

  static Ptr paste (const QList<QUrl> &urls, const QFileInfo &target, Qt::DropAction action);
  static Ptr remove (const QList<QFileInfo> &infos);
  static Ptr trash (const QList<QFileInfo> &infos);

  void abort ();

signals:
  void progress (int percent);
  void finished (bool ok);

private:
  bool transfer (const Infos &sources, const QFileInfo &target, int depth);
  bool link (const Infos &sources, const QFileInfo &target);
  bool erase (const Infos &infos, int depth);

  int resolveConflict (const QFileInfo &source, const QFileInfo &target);
  void advance (qint64 size);

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
