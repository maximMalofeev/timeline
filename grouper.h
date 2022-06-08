#ifndef GROUPER_H
#define GROUPER_H

#include <QAbstractListModel>
#include <QColor>
#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QtConcurrent>
#include <list>
#include <vector>

#include "bookmark.h"
#include "group.h"

typedef std::vector<Bookmark> BookmarksContainer;
typedef Group<BookmarksContainer::const_iterator> BookmarksGroup;

class Grouper : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int timelineDuration READ timelineDuration WRITE
                 setTimelineDuration NOTIFY timelineDurationChanged)
  Q_PROPERTY(double groupInterval READ groupInterval WRITE setGroupInterval
                 NOTIFY groupIntervalChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

 public:
  enum GrouperRoles {
    NameRole = Qt::UserRole + 1,
    StartRole,
    DurationRole,
  };

  explicit Grouper(QObject *parent = nullptr);
  ~Grouper();

  int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  // bookmarks must be sorted
  void setBookmarks(BookmarksContainer &&bookmarks);
  BookmarksContainer bookmarks() const;

  // in milliseconds
  int timelineDuration() const;
  void setTimelineDuration(int timelineDuration);
  int groupInterval() const;
  void setGroupInterval(int groupInterval);
  bool busy() const;
  int displayStart() const;
  void setDisplayStart(int displayStart);
  int displayEnd() const;
  void setDisplayEnd(int displayEnd);

  Q_INVOKABLE void generateBookmarks(int count);

 signals:
  void timelineDurationChanged();
  void groupIntervalChanged();
  void busyChanged();
  void displayStartChanged();
  void displayEndChanged();

 private:
  BookmarksContainer bookmarks_;
  std::vector<std::shared_ptr<BookmarksGroup>> groups_;
  int timelineDuration_{};
  int groupInterval_{};
  bool busy_{};

  int grouppingInterval_{};
  int grouppingBookmarksCount_{};
  mutable QMutex mutex_;
  QFutureWatcher<void> generationFutureWatcher_;
  QFutureWatcher<std::vector<std::shared_ptr<BookmarksGroup>>>
      grouppingFutureWatcher_;

  void onGenerated();
  void doGroupping();
  void onGroupped();
  void setBusy(bool busy);
};

#endif  // GROUPER_H
