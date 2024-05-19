#include "common/utils.hpp"
namespace CrazyDave {
const Date Date::FIRST_DATE = Date{6, 1};
const Date Date::INVALID_DATE = Date{-1, -1};
const Time Time::INVALID_TIME = Time{-1, -1};
const DateTime DateTime::INVALID_DATE_TIME = DateTime{Date::INVALID_DATE, Time::INVALID_TIME};

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