#include <QtTest>

#include "../bookmark.h"
#include "../grouper.h"

class TstGrouper : public QObject {
  Q_OBJECT

 public:
  TstGrouper();
  ~TstGrouper();

 private slots:
  void test_group_getter_and_setter();
  void test_grouping_non_overlapped_bookmarks();
  void test_grouping_overlapped_bookmarks();
};

TstGrouper::TstGrouper() {}

TstGrouper::~TstGrouper() {}

void TstGrouper::test_group_getter_and_setter() {
  Bookmark b1{"b1", 0, 1000};
  Bookmark b2{"b2", 200, 20000};
  Bookmark b3{"b3", 5200, 120000};

  Grouper grouper;
  grouper.setBookmarks({b1, b2, b3});

  auto bookmarks = grouper.bookmarks();
  auto bookmarksItr = bookmarks.cbegin();
  QCOMPARE(*(bookmarksItr++), b1);
  QCOMPARE(*(bookmarksItr++), b2);
  QCOMPARE(*(bookmarksItr), b3);
}

void TstGrouper::test_grouping_non_overlapped_bookmarks() {
  Grouper grouper;
  grouper.setTimelineDuration(1000);
  grouper.setGroupInterval(100);

  Bookmark b1{"b1", 0, 100};
  Bookmark b2{"b2", 200, 100};
  Bookmark b3{"b3", 500, 100};

  QSignalSpy spy{&grouper, SIGNAL(modelReset())};
  grouper.setBookmarks({b1, b2, b3});
  if (grouper.rowCount() == 0) {
    QVERIFY(spy.wait());
  }

  QCOMPARE(grouper.rowCount(), 3);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::NameRole)
               .toString()
               .toStdString(),
           b1.name);
  QCOMPARE(grouper.data(grouper.index(1), Grouper::NameRole)
               .toString()
               .toStdString(),
           b2.name);
  QCOMPARE(grouper.data(grouper.index(2), Grouper::NameRole)
               .toString()
               .toStdString(),
           b3.name);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::StartRole).toInt(),
           b1.start);
  QCOMPARE(grouper.data(grouper.index(1), Grouper::StartRole).toInt(),
           b2.start);
  QCOMPARE(grouper.data(grouper.index(2), Grouper::StartRole).toInt(),
           b3.start);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::DurationRole).toInt(),
           b1.duration);
  QCOMPARE(grouper.data(grouper.index(1), Grouper::DurationRole).toInt(),
           b2.duration);
  QCOMPARE(grouper.data(grouper.index(2), Grouper::DurationRole).toInt(),
           b3.duration);
}

void TstGrouper::test_grouping_overlapped_bookmarks() {
  Grouper grouper;
  grouper.setTimelineDuration(1000);
  grouper.setGroupInterval(100);

  Bookmark b1{"b1", 0, 100};
  Bookmark b2{"b2", 50, 100};
  Bookmark b3{"b3", 500, 100};

  QSignalSpy spy{&grouper, SIGNAL(modelReset())};
  grouper.setBookmarks({b1, b2, b3});
  if (grouper.rowCount() == 0) {
    QVERIFY(spy.wait());
  }

  QCOMPARE(grouper.rowCount(), 2);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::NameRole)
               .toString()
               .toStdString(),
           "2");
  QCOMPARE(grouper.data(grouper.index(1), Grouper::NameRole)
               .toString()
               .toStdString(),
           b3.name);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::StartRole).toInt(),
           b1.start);
  QCOMPARE(grouper.data(grouper.index(1), Grouper::StartRole).toInt(),
           b3.start);
  QCOMPARE(grouper.data(grouper.index(0), Grouper::DurationRole).toInt(),
           b2.start + b2.duration);
  QCOMPARE(grouper.data(grouper.index(1), Grouper::DurationRole).toInt(),
           b3.duration);
}

QTEST_MAIN(TstGrouper)

#include "tst_grouper.moc"
