#include "grouper.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <algorithm>
#include <ctime>
#include <execution>
#include <random>

#include "group.h"

Grouper::Grouper(QObject *parent) : QAbstractListModel(parent) {
  connect(
      &grouppingFutureWatcher_,
      &QFutureWatcher<std::vector<std::shared_ptr<BookmarksGroup>>>::finished,
      this, &Grouper::onGroupped);
  connect(&generationFutureWatcher_, &QFutureWatcher<void>::finished, this,
          &Grouper::onGenerated);
}

Grouper::~Grouper() {
  if (!generationFutureWatcher_.isFinished()) {
    generationFutureWatcher_.waitForFinished();
  }
  if (!grouppingFutureWatcher_.isFinished()) {
    grouppingFutureWatcher_.waitForFinished();
  }
}

int Grouper::rowCount(const QModelIndex &parent) const {
  return static_cast<int>(groups_.size());
}

QVariant Grouper::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }

  auto row = index.row();
  if (row < 0 || row >= groups_.size()) {
    return {};
  }

  auto group = groups_[row].get();

  switch (role) {
    case Qt::ToolTipRole:
      return group->toolTip();
    case Qt::BackgroundRole:
      return group->background();
    case NameRole:
      return group->name();
    case StartRole:
      return group->start();
    case DurationRole:
      return group->duration();
  }

  return {};
}

QHash<int, QByteArray> Grouper::roleNames() const {
  auto roles = QAbstractListModel::roleNames();

  roles[Qt::BackgroundRole] = "background";
  roles[NameRole] = "name";
  roles[StartRole] = "start";
  roles[DurationRole] = "duration";

  return roles;
}

void Grouper::setBookmarks(BookmarksContainer &&bookmarks) {
  {
    QMutexLocker ml{&mutex_};
    bookmarks_ = std::move(bookmarks);
  }
  doGroupping();
}

BookmarksContainer Grouper::bookmarks() const {
  QMutexLocker ml{&mutex_};
  return bookmarks_;
}

int Grouper::timelineDuration() const { return timelineDuration_; }

void Grouper::setTimelineDuration(int timelineDuration) {
  if (timelineDuration_ >= 0 && timelineDuration_ != timelineDuration) {
    timelineDuration_ = timelineDuration;
    doGroupping();
    emit timelineDurationChanged();
  }
}

int Grouper::groupInterval() const { return groupInterval_; }

void Grouper::setGroupInterval(int groupInterval) {
  if (groupInterval >= 0 && groupInterval_ != groupInterval) {
    groupInterval_ = groupInterval;
    doGroupping();
    emit groupIntervalChanged();
  }
}

bool Grouper::busy() const { return busy_; }

void Grouper::generateBookmarks(int count) {
  setBusy(true);
  generationFutureWatcher_.setFuture(QtConcurrent::run([this, count] {
    BookmarksContainer bk;
    bk.reserve(count);

    constexpr int maxDuration = 3 * 60 * 60 * 1000;
    std::default_random_engine defEngine(time(0));
    std::uniform_int_distribution<int> timelineDistribution(0,
                                                            timelineDuration_);
    std::uniform_int_distribution<int> durationDistribution(1, maxDuration);

    for (int i = 0; i < count; i++) {
      auto startTime = timelineDistribution(defEngine);
      auto duration = durationDistribution(defEngine);
      auto durationToTheEnd = timelineDuration_ - startTime;
      bk.push_back({"Bookmark ", startTime,
                    duration > durationToTheEnd ? durationToTheEnd : duration});
    }

    std::sort(std::execution::par, bk.begin(), bk.end());

    for (int i = 0; i < bk.size(); i++) {
      bk[i].name = bk[i].name.append(std::to_string(i));
    }

    setBookmarks(std::move(bk));
  }));
}

void Grouper::onGenerated() {
  if (grouppingFutureWatcher_.isFinished()) {
    setBusy(false);
  }
}

void Grouper::doGroupping() {
  if (grouppingFutureWatcher_.isRunning()) {
    return;
  }

  grouppingFutureWatcher_.setFuture(QtConcurrent::run([this] {
    std::vector<std::shared_ptr<BookmarksGroup>> groups;

    QMutexLocker ml{&mutex_};
    grouppingInterval_ = groupInterval_;
    grouppingBookmarksCount_ = static_cast<int>(bookmarks_.size());

    if (bookmarks_.empty()) {
      return groups;
    }

    if (grouppingInterval_ == 0) {
      groups.reserve(bookmarks_.size());
      for (auto b = ++bookmarks_.cbegin();
           b != bookmarks_.cend() && grouppingInterval_ == groupInterval_;
           b++) {
        groups.push_back(BookmarksGroup::makeGroup(b, b));
      }
    } else {
      groups.reserve(std::ceil(timelineDuration_ / grouppingInterval_));

      auto groupBeginItr = bookmarks_.cbegin();
      auto groupEndTime = groupBeginItr->start + grouppingInterval_;
      auto lambda = [](const Bookmark &b, int v) { return b.start < v; };
      // first out of group element
      auto groupEndItr = std::lower_bound(groupBeginItr, bookmarks_.cend(),
                                          groupEndTime, lambda);
      while (groupEndItr != bookmarks_.cend() &&
             grouppingInterval_ == groupInterval_) {
        groups.push_back(BookmarksGroup::makeGroup(groupBeginItr, groupEndItr));
        groupBeginItr = groupEndItr;
        groupEndTime = groupBeginItr->start + grouppingInterval_;
        groupEndItr = std::lower_bound(groupBeginItr, bookmarks_.cend(),
                                       groupEndTime, lambda);
      }

      groups.push_back(BookmarksGroup::makeGroup(groupBeginItr, groupEndItr));
    }

    if (groups.size() == groups_.size() &&
        std::equal(groups.begin(), groups.end(), groups_.begin(),
                   [](const std::shared_ptr<BookmarksGroup> &l,
                      std::shared_ptr<BookmarksGroup> &r) {
                     return l->start() == r->start() &&
                            l->duration() == r->duration();
                   })) {
      return groups_;
    }

    return groups;
  }));
}

void Grouper::onGroupped() {
  auto newGroups = grouppingFutureWatcher_.result();
  if (newGroups == groups_) {
    return;
  }

  if (grouppingInterval_ != groupInterval_ ||
      grouppingBookmarksCount_ != bookmarks_.size()) {
    doGroupping();
  } else {
    beginResetModel();
    groups_ = std::move(newGroups);
    endResetModel();
  }

  if (busy_) {
    setBusy(false);
  }
}

void Grouper::setBusy(bool busy) {
  if (busy_ != busy) {
    busy_ = busy;
    emit busyChanged();
  }
}
