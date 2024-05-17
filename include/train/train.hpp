#ifndef TICKET_SYSTEM_TRAIN_HPP
#define TICKET_SYSTEM_TRAIN_HPP
#include <iostream>
#include <string>
#include "common/utils.hpp"
#include "data_structures/BPT.hpp"
#include "data_structures/vector.h"
namespace CrazyDave {

class Train {
 private:
  String<21> train_id_;
  int station_num_{};
  String<41> stations_[100];  // [station]
  int seat_num_{};
  int left_seat_num_[100][100]{};  // [station][date]
  int prices_[100]{};              // [station]
  TimeRange time_range_[100];      // [station]
  DateRange sale_date_;
  char type_{};
  bool is_released_{};

 public:
  Train() = default;
  Train(const std::string &train_id, int seat_num, const vector<std::string> &stations, const vector<int> &prices,
        const Time &start_time, const vector<int> &travel_times, const vector<int> &stop_over_times,
        DateRange sale_date, const char type);

  friend auto train_cmp_1(const Train &t1, const Train &t2) -> bool { return t1.train_id_ < t2.train_id_; }
};
class TrainSystem {
  enum class QueryType { TIME, TRAIN_ID };
  class Trade {
    enum class Status { SUCCESS, PENDING, REFUNDED };
    Status status_;
    String<21> train_id_;
    DateTime leaving_time_;
    DateTime arrival_time_;
    String<41> station_1_;
    String<41> station_2_;
    int price_;
    int num_;

   public:
    Trade(const Status &status, const String<21> &train_id, const DateTime &leaving_time, const DateTime &arrival_time,
          const String<41> &station_1, const String<41> &station_2, int price, int num)
        : status_(status),
          train_id_(train_id),
          leaving_time_(leaving_time),
          arrival_time_(arrival_time),
          station_1_(station_1),
          station_2_(station_2),
          price_(price),
          num_(num) {}
    friend std::ostream &operator<<(std::ostream &os, const Trade &trade) {
      os << "[";
      switch (trade.status_) {
        case Status::SUCCESS:
          os << "[success]";
          break;
        case Status::PENDING:
          os << "[pending]";
          break;
        case Status::REFUNDED:
          os << "refunded";
          break;
      }
      os << " " << trade.train_id_ << " " << trade.station_1_ << " " << trade.leaving_time_ << " -> "
         << trade.station_2_ << " " << trade.arrival_time_ << " " << trade.price_ << " " << trade.num_;
      return os;
    }
  };

 private:
  BPlusTree<String<21>, Train> train_storage_{"tmp/tr1", "tmp/tr2", "tmp/tr3", "tmp/tr4"};
  BPlusTree<String<41>, String<21>> station_storage_{"tmp/st1", "tmp/st2", "tmp/st3", "tmp/st4"};
  BPlusTree<String<21>, Trade> trade_storage_{"tmp/trade1", "tmp/trade2", "tmp/trade3", "tmp/trade4"};

 public:
  auto add_train(const std::string &train_id, int seat_num, const vector<std::string> &stations,
                 const vector<int> &prices, const Time &start_time, const vector<int> &travel_times,
                 const vector<int> &stop_over_times, const DateRange &sale_date, const char type) -> bool;

  auto delete_train(const std::string &train_id) -> bool;
  auto release_train(const std::string &train_id) -> bool;
  auto query_train(const std::string &train_id, const Date &date) -> bool;
  void query_ticket(const std::string &station_1, const std::string &station_2, const Date &date,
                    const QueryType &type);

  auto query_transfer(const std::string &station_1, const std::string &station_2, const Date &date,
                      const QueryType &type) -> bool;

  auto buy_ticket(const std::string &user_name, const std::string &train_id, const Date &date, const int num,
                  const std::string &station_1, const std::string &station_2, const bool wait);
  auto query_order(const std::string &user_name) -> bool;
  auto refund_ticket(const std::string &user_name, const int num) -> bool;
  void clean();
  void exit();
};
}  // namespace CrazyDave
#endif  // TICKET_SYSTEM_TRAIN_HPP
