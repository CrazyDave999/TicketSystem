#ifndef BPT_UTILS_HPP
#define BPT_UTILS_HPP
#include <cstring>
#include <fstream>
#include <random>
#include "common/string_utils.hpp"
#include "config.hpp"
#include "data_structures/vector.h"

namespace CrazyDave {
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
template <class T1, class T2>
class pair {
 public:
  T1 first;
  T2 second;
  constexpr pair() : first(), second() {}
  pair(const pair &other) = default;
  pair &operator=(const pair &other) {
    if (this == &other) {
      return *this;
    }
    first = other.first;
    second = other.second;
    return *this;
  }
  pair(pair &&other) noexcept = default;
  pair(const T1 &x, const T2 &y) : first(x), second(y) {}
  template <class U1, class U2>
  pair(U1 &&x, U2 &&y) : first(x), second(y) {}
  template <class U1, class U2>
  explicit pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
  template <class U1, class U2>
  explicit pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}
  auto operator<(const pair &rhs) const -> bool {
    if (first != rhs.first) {
      return first < rhs.first;
    }
    return second < rhs.second;
  }
};
template <size_t L>
class String {
  // 可写字符串类
  char str[L]{'\0'};

 public:
  String() = default;

  String(const char *s) { std::strcpy(str, s); }

  String(const std::string &s) { std::strcpy(str, s.c_str()); }

  operator const char *() const { return str; }

  operator std::string() const { return std::move(std::string(str)); }

  const char *c_str() { return str; }

  char &operator[](int pos) { return str[pos]; }

  String &operator=(const String &rhs) {
    if (this == &rhs) return *this;
    std::strcpy(str, rhs.str);
    return *this;
  }

  String &operator=(const char *s) {
    std::strcpy(str, s);
    return *this;
  }

  String &operator=(const std::string &s) {
    std::strcpy(str, s.c_str());
    return *this;
  }

  bool operator==(const String &rhs) const { return !std::strcmp(str, rhs.str); }

  bool operator!=(const String &rhs) const { return std::strcmp(str, rhs.str); }

  bool operator<(const String &rhs) const { return std::strcmp(str, rhs.str) < 0; }

  template <size_t M>
  auto operator+(const String<M> &rhs) const -> String<L + M> {
    String<L + M> res;
    std::strcpy(res.str, str);
    std::strcat(res.str, rhs.str);
    return std::move(res);
  }

  friend std::istream &operator>>(std::istream &is, String &rhs) { return is >> rhs.str; }

  friend std::ostream &operator<<(std::ostream &os, const String &rhs) { return os << rhs.str; }
};

template <size_t L>
class HashString {
 private:
  static inline auto HashBytes(const char *bytes) -> size_t {
    size_t hash = L;
    for (size_t i = 0; i < L; ++i) {
      hash = ((hash << 5) ^ (hash >> 27)) ^ bytes[i];
    }
    return hash;
  }

 public:
  auto operator()(const String<L> &str) const -> size_t { return HashBytes(str); }
};
static inline auto HashBytes(const char *bytes) -> size_t {
  size_t L = strlen(bytes);
  size_t hash = L;
  for (size_t i = 0; i < L; ++i) {
    hash = ((hash << 5) ^ (hash >> 27)) ^ bytes[i];
  }
  return hash;
}
class File {
  char name[65]{'\0'};
  std::fstream fs;
  bool is_new = false;

 public:
  explicit File(const char *_name) {
    strcpy(name, _name);
    fs.open(name, std::ios::in);
    if (!fs) {
      fs.open(name, std::ios::out);
      is_new = true;
    }
    fs.close();
  }

  void open(std::ios::openmode mode = std::ios::in | std::ios::out) { fs.open(name, mode); }

  void close() { fs.close(); }

  bool get_is_new() const { return is_new; }

  template <class T>
  void read(T &dst, size_t size = sizeof(T)) {
    fs.read(reinterpret_cast<char *>(&dst), (long)size);
  }

  template <class T>
  void write(const T &src, size_t size = sizeof(T)) {
    fs.write(reinterpret_cast<const char *>(&src), (long)size);
  }

  void seekg(int pos) { fs.seekg(pos); }

  void seekp(int pos) { fs.seekp(pos); }

  void clear() {
    fs.close();
    fs.open(name, std::ios::in | std::ios::out | std::ios::trunc);
  }
};

struct Time {
  static const Time INVALID_TIME;
  int hour_{};
  int minute_{};
  Time() = default;
  Time(int hour, int minute) : hour_(hour), minute_(minute) {}
  explicit Time(const std::string &str) {
    auto vec = StringUtil::Split(str, ':');
    hour_ = std::stoi(vec[0]);
    minute_ = std::stoi(vec[1]);
  }
  friend std::ostream &operator<<(std::ostream &os, const Time &time) {
    if (time == INVALID_TIME) {
      os << "xx:xx";
      return os;
    }
    if (time.hour_ < 10) {
      os << "0";
    }
    os << time.hour_ << ":";
    if (time.minute_ < 10) {
      os << "0";
    }
    os << time.minute_;
    return os;
  }
  auto operator+=(int minutes) -> Time & {
    hour_ += minutes / 60;
    minute_ += minutes % 60;
    if (minute_ >= 60) {
      ++hour_;
      minute_ -= 60;
    }
    return *this;
  }
  auto operator+(int minutes) const -> Time {
    Time res = *this;
    res += minutes;
    return res;
  }
  auto operator+=(const Time &rhs) -> Time & {
    hour_ += rhs.hour_;
    minute_ += rhs.minute_;
    if (minute_ >= 60) {
      ++hour_;
      minute_ -= 60;
    }
    return *this;
  };
  auto operator<(const Time &rhs) const -> bool {
    if (hour_ != rhs.hour_) {
      return hour_ < rhs.hour_;
    }
    return minute_ < rhs.minute_;
  }
  auto operator==(const Time &rhs) const -> bool { return hour_ == rhs.hour_ && minute_ == rhs.minute_; }
  auto operator>(const Time &rhs) const -> bool {
    if (hour_ != rhs.hour_) {
      return hour_ > rhs.hour_;
    }
    return minute_ > rhs.minute_;
  }
  auto operator<=(const Time &rhs) const -> bool { return !(*this > rhs); }
  auto operator>=(const Time &rhs) const -> bool { return !(*this < rhs); }
  auto operator!=(const Time &rhs) const -> bool { return !(*this == rhs); }
  auto operator-(const Time &rhs) const -> int { return (hour_ - rhs.hour_) * 60 + minute_ - rhs.minute_; }
};
struct Date {
  static const Date FIRST_DATE;
  static const Date INVALID_DATE;
  int month_{};
  int day_{};
  Date() = default;
  Date(int _month, int _day) : month_(_month), day_(_day) {}
  explicit Date(const std::string &str) {
    auto vec = StringUtil::Split(str, '-');
    month_ = std::stoi(vec[0]);
    day_ = std::stoi(vec[1]);
  }
  friend std::ostream &operator<<(std::ostream &os, const Date &date) {
    if (date == INVALID_DATE) {
      os << "xx-xx";
      return os;
    }
    if (date.month_ < 10) {
      os << "0";
    }
    os << date.month_ << "-";
    if (date.day_ < 10) {
      os << "0";
    }
    os << date.day_;
    return os;
  }
  //    friend std::istream &operator>>(std::istream &is, Date &date) {}
  auto operator<(const Date &rhs) const -> bool {
    if (month_ != rhs.month_) {
      return month_ < rhs.month_;
    }
    return day_ < rhs.day_;
  }
  auto operator==(const Date &rhs) const -> bool { return month_ == rhs.month_ && day_ == rhs.day_; }
  auto operator>(const Date &rhs) const -> bool {
    if (month_ != rhs.month_) {
      return month_ > rhs.month_;
    }
    return day_ > rhs.day_;
  }
  auto operator<=(const Date &rhs) const -> bool { return !(*this > rhs); }
  auto operator>=(const Date &rhs) const -> bool { return !(*this < rhs); }
  auto operator!=(const Date &rhs) const -> bool { return !(*this == rhs); }
  auto operator+=(int day) -> Date & {
    day_ += day;
    if (day_ > DAY_NUM[month_]) {
      day_ -= DAY_NUM[month_];
      ++month_;
      if (month_ == 13) {
        month_ = 1;
      }
    }
    return *this;
  }
  auto operator+=(const Date &rhs) -> Date & {
    // 仅考虑小于一个月的情况
    day_ += rhs.day_;
    if (day_ > DAY_NUM[month_]) {
      day_ -= DAY_NUM[month_];
      ++month_;
      if (month_ == 13) {
        month_ = 1;
      }
    }
    return *this;
  }
  auto operator+(const Date &rhs) const -> Date {
    Date res = *this;
    res += rhs;
    return res;
  }
  auto operator+(int day) const -> Date {
    Date res = *this;
    res += day;
    return res;
  }
  auto operator-(const Date &rhs) const -> int { return DAY_PREFIX[month_] + day_ - DAY_PREFIX[rhs.month_] - rhs.day_; }
  auto operator-(int day) const -> Date {
    Date res = *this;
    res.day_ -= day;
    if (res.day_ < 1) {
      --res.month_;
      res.day_ += DAY_NUM[res.month_];
    }
    return res;
  }
};

struct DateTime {
  static const DateTime INVALID_DATE_TIME;
  Date date{};
  Time time{};
  friend std::ostream &operator<<(std::ostream &os, const DateTime &date_time) {
    os << date_time.date << " " << date_time.time;
    return os;
  }
  auto operator+=(int minutes) -> DateTime & {
    time += minutes;
    date += time.hour_ / 24;
    time.hour_ %= 24;
    return *this;
  }
  auto operator+(int minutes) const -> DateTime {
    DateTime res = *this;
    res += minutes;
    return res;
  }
  auto operator+=(const DateTime &rhs) -> DateTime & {
    if (rhs == INVALID_DATE_TIME) {
      *this = INVALID_DATE_TIME;
      return *this;
    }
    if (*this == INVALID_DATE_TIME) {
      return *this;
    }
    time += rhs.time;
    date += time.hour_ / 24;
    time.hour_ %= 24;
    date += rhs.date;
    return *this;
  }
  auto operator+(const DateTime &rhs) const -> DateTime {
    if (*this == INVALID_DATE_TIME || rhs == INVALID_DATE_TIME) {
      return INVALID_DATE_TIME;
    }
    DateTime res = *this;
    res += rhs;
    return res;
  }
  auto operator<(const DateTime &rhs) const -> bool {
    if (date != rhs.date) {
      return date < rhs.date;
    }
    return time < rhs.time;
  }
  auto operator==(const DateTime &rhs) const -> bool { return date == rhs.date && time == rhs.time; }
  auto operator>(const DateTime &rhs) const -> bool {
    if (date != rhs.date) {
      return date > rhs.date;
    }
    return time > rhs.time;
  }
  auto operator<=(const DateTime &rhs) const -> bool { return !(*this > rhs); }
  auto operator>=(const DateTime &rhs) const -> bool { return !(*this < rhs); }
  auto operator!=(const DateTime &rhs) const -> bool { return !(*this == rhs); }
  auto operator-(const DateTime &rhs) const -> int {  // 单位: min
    return (date - rhs.date) * 1440 + (time - rhs.time);
  }
};
using TimeRange = pair<Time, Time>;
using DateRange = pair<Date, Date>;
using DateTimeRange = pair<DateTime, DateTime>;
std::ostream &operator<<(std::ostream &os, const CrazyDave::TimeRange &time_range);
std::ostream &operator<<(std::ostream &os, const CrazyDave::DateTimeRange &date_time_range);
DateTimeRange operator+(const DateTimeRange &d1, const DateTimeRange &d2);
struct StationDateTime {
  String<40> station;
  DateTime date_time;
  friend std::ostream &operator<<(std::ostream &os, const StationDateTime &station_date_time) {
    os << station_date_time.station << " " << station_date_time.date_time;
    return os;
  }
};
using StationDateTimeRange = pair<StationDateTime, StationDateTime>;
std::ostream &operator<<(std::ostream &os, const CrazyDave::StationDateTimeRange &station_date_time_range);

}  // namespace CrazyDave
#endif  // BPT_UTILS_HPP
