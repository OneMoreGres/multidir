#include "navigationhistory.h"
#include "shortcutmanager.h"
#include "debug.h"

#include <QFileInfo>
#include <QAction>

NavigationHistory::NavigationHistory (QObject *parent) :
  QObject (parent),
  forward_ (new QAction ({}, this)),
  backward_ (new QAction ({}, this)),
  history_ (),
  currentIndex_ (-1)
{
  ShortcutManager::add (ShortcutManager::HistoryForward, forward_);
  forward_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  connect (forward_, &QAction::triggered,
           this, &NavigationHistory::moveForward);

  ShortcutManager::add (ShortcutManager::HistoryBackward, backward_);
  backward_->setShortcutContext (Qt::WidgetWithChildrenShortcut);
  connect (backward_, &QAction::triggered,
           this, &NavigationHistory::moveBackward);
}

void NavigationHistory::addPath (const QFileInfo &path)
{
  if (history_.isEmpty ())
  {
    ++currentIndex_;
    history_ << path;
    return;
  }

  if (history_[currentIndex_] == path)
  {
    return;
  }

  for (auto i = 0, end = history_.size () - currentIndex_ - 1; i < end; ++i)
  {
    history_.pop_back ();
  }

  ++currentIndex_;
  history_ << path;


  const auto maxSize = 50;
  if (currentIndex_ > maxSize)
  {
    history_.pop_front ();
    --currentIndex_;
  }
}

void NavigationHistory::clear ()
{
  history_.clear ();
  currentIndex_ = -1;
}

QAction * NavigationHistory::forwardAction () const
{
  return forward_;
}

QAction * NavigationHistory::backwardAction () const
{
  return backward_;
}

void NavigationHistory::moveForward ()
{
  if (currentIndex_ + 1 < history_.size ())
  {
    emit pathChanged (history_[++currentIndex_]);
  }
}

void NavigationHistory::moveBackward ()
{
  if (currentIndex_ >= 1)
  {
    emit pathChanged (history_[--currentIndex_]);
  }
}

#include "moc_navigationhistory.cpp"
