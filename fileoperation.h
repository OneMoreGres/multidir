#pragma once

#include <QFuture>
#include <QFileInfo>
#include <QUrl>


class FileOperation
{
public:
  enum class Action
  {
    Copy, Move, Link, Remove, Trash
  };

  FileOperation ();

  const QList<QFileInfo> &sources () const;
  const QFileInfo &target () const;
  const Action &action () const;
  const QFuture<bool> &future () const;

  bool isFinished () const;

  static QSharedPointer<FileOperation> paste (const QList<QUrl> &urls, const QFileInfo &target,
                                              Qt::DropAction action);
  static QSharedPointer<FileOperation> remove (const QList<QFileInfo> &infos);
  static QSharedPointer<FileOperation> trash (const QList<QFileInfo> &infos);

private:
  QList<QFileInfo> sources_;
  QFileInfo target_;
  Action action_;
  QFuture<bool> future_;
};
