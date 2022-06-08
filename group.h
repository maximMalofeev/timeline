#ifndef GROUP_H
#define GROUP_H

#include <QColor>
#include <QString>
#include <QTime>
#include <memory>

template <typename Itr>
class Group {
 public:
  virtual ~Group();

  int start() const { return start_; }
  int duration() const { return duration_; }
  int end() const { return start_ + duration_; }
  QString name() const { return name_; }
  QColor background() const { return background_; }
  QString toolTip() const { return toolTip_; }

  static std::unique_ptr<Group<Itr>> makeGroup(Itr first, Itr last);

 protected:
  int start_{};
  int duration_{};
  QString name_;
  QColor background_;
  QString toolTip_;
};

template <typename Itr>
class OneBookmarkGroup : public Group<Itr> {
 public:
  explicit OneBookmarkGroup(Itr first);
  ~OneBookmarkGroup();
};

template <typename Itr>
class ManyBookmarksGroup : public Group<Itr> {
 public:
  explicit ManyBookmarksGroup(Itr first, Itr last);
  ~ManyBookmarksGroup();
};

template <typename Itr>
Group<Itr>::~Group() {}

template <typename Itr>
std::unique_ptr<Group<Itr>> Group<Itr>::makeGroup(Itr first, Itr last) {
  if (first + 1 == last) {
    return std::make_unique<OneBookmarkGroup<Itr>>(first);
  } else {
    return std::make_unique<ManyBookmarksGroup<Itr>>(first, last);
  }
}

template <typename Itr>
OneBookmarkGroup<Itr>::OneBookmarkGroup(Itr first) {
  start_ = first->start;
  duration_ = first->duration;
  name_ = QString::fromStdString(first->name);
  background_ = "green";
  toolTip_ =
      QString{"Name: %1\nStart time: %2\nDuration: %3"}
          .arg(name_)
          .arg(QTime::fromMSecsSinceStartOfDay(first->start).toString())
          .arg(QTime::fromMSecsSinceStartOfDay(first->duration).toString());
}

template <typename Itr>
OneBookmarkGroup<Itr>::~OneBookmarkGroup() {}

template <typename Itr>
ManyBookmarksGroup<Itr>::ManyBookmarksGroup(Itr first, Itr last) {
  start_ = first->start;

  QElapsedTimer timer;
  timer.start();
  auto maxElement =
      std::max_element(std::execution::par, first, last, [](auto &l, auto &r) {
        return (l.start + l.duration) < (r.start + r.duration);
      });
  duration_ = maxElement->start + maxElement->duration - first->start;

  auto distance = std::distance(first, last);
  name_ = QString::number(distance);

  QStringList bookmarksList;
  bookmarksList.reserve(distance);
  while (first != last && bookmarksList.size() < 15) {
    bookmarksList << QString::fromStdString(first->name);
    first++;
  }

  if (first != last) {
    bookmarksList << QString{"+ %1 other bookmarks"}.arg(distance - 15);
  }
  toolTip_ = bookmarksList.join('\n');

  background_ = "deepskyblue";
}

template <typename Itr>
ManyBookmarksGroup<Itr>::~ManyBookmarksGroup() {}

#endif