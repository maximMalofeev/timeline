#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <string>

struct Bookmark {
  std::string name;
  int start;
  int duration;
};

inline bool operator<(const Bookmark& l, const Bookmark& r) {
  return l.start < r.start;
}
inline bool operator<(const Bookmark& l, int r) {
  return l.start < r;
}

inline bool operator==(const Bookmark& l, const Bookmark& r) {
  return l.name == r.name && l.start == r.start && l.duration == r.duration;
}

#endif  // BOOKMARK_H
