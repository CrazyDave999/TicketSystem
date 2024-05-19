#include "common/utils.hpp"
namespace CrazyDave {
const Date Date::FIRST_DATE = Date{6, 1};
const Date Date::INVALID_DATE = Date{-1, -1};
const Time Time::INVALID_TIME = Time{-1, -1};
const DateTime DateTime::INVALID_DATE_TIME = DateTime{Date::INVALID_DATE, Time::INVALID_TIME};
template <class T>
int cmp(const T &lhs, const T &rhs) {
  if (lhs < rhs)
    return -1;
  else
    return rhs < lhs;
}

template <class T>
int binary_search(const T *first, int len, const T &val) {
  int l = 0, r = len;
  while (l <= r) {
    int mid = (l + r) >> 1;
    int flag = cmp(first[mid], val);
    if (flag < 0) {
      l = mid + 1;
    } else if (flag > 0) {
      r = mid - 1;
    } else {
      return mid;
    }
  }
  return -1;
}

template <class T1, class T2>
int upper_bound(const T1 *first, int len, const T2 &val) {
  // Find the first element that > val.
  int l = 0, r = len - 1;
  int res = len;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (val < first[mid]) {
      res = mid;
      r = mid - 1;
    } else {
      l = mid + 1;
    }
  }
  return res;
}

template <class T1, class T2>
int lower_bound(const T1 *first, int len, const T2 &val) {
  // Find the first element that >= val.
  int l = 0, r = len - 1;
  int res = len;
  while (l <= r) {
    int mid = (l + r) >> 1;
    if (!(first[mid] < val)) {
      res = mid;
      r = mid - 1;
    } else {
      l = mid + 1;
    }
  }
  return res;
}

template <class T>
void insert_at(T *first, int len, int pos, const T &val) {
  for (int i = len; i > pos; --i) {
    first[i] = first[i - 1];
  }
  first[pos] = val;
}

template <class T>
void remove_at(T *first, int len, int pos) {
  for (int i = pos; i < len - 1; ++i) {
    first[i] = first[i + 1];
  }
}

template <class T>
int increase_insert(T *first, int len, const T &val) {
  int pos = lower_bound(first, len, val);
  insert_at(first, len, pos, val);
  return pos;
}

template <class T>
bool increase_remove(T *first, int len, const T &val) {
  int pos = binary_search(first, len, val);
  if (pos != -1) {
    remove_at(first, len, pos);
    return true;
  }
  return false;
}
std::ostream &operator<<(std::ostream &os, const TimeRange &time_range) {
  os << time_range.first << " -> " << time_range.second;
  return os;
}
std::ostream &operator<<(std::ostream &os, const DateTimeRange &date_time_range) {
  os << date_time_range.first << " -> " << date_time_range.second;
  return os;
}
DateTimeRange operator+(const DateTimeRange &d1, const DateTimeRange &d2) {
  return {d1.first + d2.first, d1.second + d2.second};
}

std::ostream &operator<<(std::ostream &os, const StationDateTimeRange &station_date_time_range) {
  os << station_date_time_range.first << " -> " << station_date_time_range.second;
  return os;
}

}