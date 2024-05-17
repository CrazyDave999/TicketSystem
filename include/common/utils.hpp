#ifndef BPT_UTILS_HPP
#define BPT_UTILS_HPP

#include <cstring>
#include <fstream>

namespace CrazyDave {
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
  void write(T &src, size_t size = sizeof(T)) {
    fs.write(reinterpret_cast<const char *>(&src), (long)size);
  }

  void seekg(int pos) { fs.seekg(pos); }

  void seekp(int pos) { fs.seekp(pos); }
};

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
struct Time {
  int hour{};
  int minute{};
  friend std::ostream &operator<<(std::ostream &os, const Time &time) {
    if (time.hour < 0) {
      os << "xx:xx";
      return os;
    }
    if (time.hour < 10) {
      os << "0";
    }
    os << time.hour << ":";
    if (time.minute < 10) {
      os << "0";
    }
    os << time.minute;
    return os;
  }
  auto operator+=(int minutes) -> Time & {
    hour += minutes / 60;
    minute += minutes % 60;
    if (minute >= 60) {
      ++hour;
      minute -= 60;
    }
    return *this;
  }
  auto operator+(int minutes) const -> Time {
    Time res = *this;
    res += minutes;
    return res;
  }
};
struct Date {
  static const Date FIRST_DATE;
  int month{};
  int day{};
  friend std::ostream &operator<<(std::ostream &os, const Date &date) {
    if (date.month < 0) {
      os << "xx-xx";
      return os;
    }
    if (date.month < 10) {
      os << "0";
    }
    os << date.month << "-";
    if (date.day < 10) {
      os << "0";
    }
    os << date.day;
    return os;
  }
  //    friend std::istream &operator>>(std::istream &is, Date &date) {}
  auto operator<(const Date &rhs) const -> bool {
    if (month != rhs.month) {
      return month < rhs.month;
    }
    return day < rhs.day;
  }
  auto operator==(const Date &rhs) const -> bool { return month == rhs.month && day == rhs.day; }
  auto operator>(const Date &rhs) const -> bool {
    if (month != rhs.month) {
      return month > rhs.month;
    }
    return day > rhs.day;
  }
  auto operator<=(const Date &rhs) const -> bool { return !(*this > rhs); }
  auto operator>=(const Date &rhs) const -> bool { return !(*this < rhs); }
  auto operator!=(const Date &rhs) const -> bool { return !(*this == rhs); }
  auto operator++() -> Date & {
    ++day;
    if ((month == 6 && day > 30) || (month == 7 && day > 31)) {
      ++month;
      day = 1;
    }
    return *this;
  }
};

struct DateTime {
  Date date;
  Time time;
  friend std::ostream &operator<<(std::ostream &os, const DateTime &date_time) {
    os << date_time.date << " " << date_time.time;
    return os;
  }
  auto operator+=(int minutes) -> DateTime & {
    time += minutes;
    while (time.hour >= 24) {
      ++date;
      time.hour -=24;
    }
    return *this;
  }
};
using TimeRange = pair<Time, Time>;
using DateRange = pair<Date, Date>;
using DateTimeRange = pair<DateTime, DateTime>;
std::ostream &operator<<(std::ostream &os, const DateTimeRange &date_time_range) {
  os << date_time_range.first << " -> " << date_time_range.second;
  return os;
}
}  // namespace CrazyDave
#endif  // BPT_UTILS_HPP
