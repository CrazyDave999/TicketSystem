#ifndef TICKET_SYSTEM_TRAIN_HPP
#define TICKET_SYSTEM_TRAIN_HPP
#include <iostream>
#include <sstream>
#include <string>
#include "common/management_system.hpp"
#include "common/utils.hpp"
#include "data_structures/BPT.hpp"
#include "data_structures/linked_hashmap.h"
#include "data_structures/vector.h"
#include "storage/index/b_plus_tree.h"
#include "train/queue_system.hpp"

namespace CrazyDave {
class ManagementSystem;
enum class QueryType { TIME, COST };
enum class Status { SUCCESS, PENDING, REFUNDED };
class TrainSystem;
class QueueSystem;
class Train {
  friend TrainSystem;

 private:
  String<20> train_id_{};
  int station_num_{};
  String<44> stations_[100]{};  // [station]
  int seat_num_{};
  int prices_[99]{};               // [station]
  DateTimeRange time_ranges_[100];  // [station], {arrival time, leaving time},
                                    // record the offset time from the start_time
  DateRange sale_date_range_{};
  char type_{};
  bool is_released_{};

 public:
  Train() = default;
  Train(const std::string &train_id, int seat_num, const vector<std::string> &stations, const vector<int> &prices,
        const Time &start_time, const vector<int> &travel_times, const vector<int> &stop_over_times,
        DateRange sale_date, char type);

  auto operator<(const Train &rhs) const -> bool { return train_id_ < rhs.train_id_; }
};
struct Seat {
  size_t train_hs_{};
  int seat_num_[100][92]{};  // [station][date] date 是从始发站出发的日期 index
  bool operator<(const Seat &rhs) const { return train_hs_ < rhs.train_hs_; }
};
class TrainSystem {
  struct Record {
    size_t train_hs{};
    int index{};
    auto operator<(const Record &rhs) const -> bool { return train_hs < rhs.train_hs; }
  };
  struct Trade {
    int time_stamp_{};
    Status status_{};
    String<20> train_id_{};
    DateTime leaving_time_{};
    DateTime arrival_time_{};
    String<44> station_1_{};
    String<44> station_2_{};
    int station_index_1_{};
    int station_index_2_{};
    int price_{};
    int num_{};
    int date_index_{};

   public:
    Trade() = default;
    Trade(int time_stamp, const Status &status, const String<20> &train_id, const DateTime &leaving_time,
          const DateTime &arrival_time, const String<44> &station_1, int station_index_1, const String<44> &station_2,
          int station_index_2, int price, int num, int date_index)
        : time_stamp_(time_stamp),
          status_(status),
          train_id_(train_id),
          leaving_time_(leaving_time),
          arrival_time_(arrival_time),
          station_1_(station_1),
          station_2_(station_2),
          station_index_1_(station_index_1),
          station_index_2_(station_index_2),
          price_(price),
          num_(num),
          date_index_(date_index) {}
    friend std::ostream &operator<<(std::ostream &os, const Trade &trade) {
      switch (trade.status_) {
        case Status::SUCCESS:
          os << "[success]";
          break;
        case Status::PENDING:
          os << "[pending]";
          break;
        case Status::REFUNDED:
          os << "[refunded]";
          break;
      }
      os << " " << trade.train_id_ << " " << trade.station_1_ << " " << trade.leaving_time_ << " -> "
         << trade.station_2_ << " " << trade.arrival_time_ << " " << trade.price_ << " " << trade.num_ << "\n";
      return os;
    }
    auto operator<(const Trade &rhs) const -> bool { return time_stamp_ > rhs.time_stamp_; }
  };

  struct TicketResult {
    String<20> train_id{};
    DateTimeRange range{};

    int price;
    int max_num;
    [[nodiscard]] auto total_time() const -> int { return range.second - range.first; }
  };
  struct TransferResult {
    TicketResult res_1{};
    TicketResult res_2{};
    String<44> mid_station{};
    [[nodiscard]] auto total_time() const -> int { return res_2.range.second - res_1.range.first; }
    [[nodiscard]] auto total_price() const -> int { return res_1.price + res_2.price; }
  };

 private:
#ifdef DEBUG_FILE_IN_TMP
  MyBPlusTree<size_t, Seat> seat_storage_{"tmp/se1", "tmp/se2", "tmp/se3", "tmp/se4"};
  MyBPlusTree<size_t, Train> train_storage_{"tmp/tr1","tmp/tr2","tmp/tr3","tmp/tr4"};
  BPT<size_t, Trade> trade_storage_{"tmp/trd", 0, 300, 30};
  BPT<size_t, Record> station_storage_{"tmp/st", 0, 300, 30};
#else
  //  MyBPlusTree<size_t, Train> train_storage_{"tr1", "tr2", "tr3", "tr4"};
  //  MyBPlusTree<size_t, Trade> trade_storage_{"trd1", "trd2", "trd3", "trd4"};
  //  MyBPlusTree<size_t, Record> station_storage_{"st1", "st2", "st3", "st4"};
  MyBPlusTree<size_t, Seat> seat_storage_{"se1", "se2", "se3", "se4"};
  MyBPlusTree<size_t, Train> train_storage_{"tr1","tr2","tr3","tr4"};
  BPT<size_t, Trade> trade_storage_{"trd", 0, 100, 30};
  BPT<size_t, Record> station_storage_{"st", 0, 100, 30};

#endif
  QueueSystem q_sys_;
  ManagementSystem *m_sys_{};

  /*
   * 检查候补队列，将能够补票的所有订单补票
   */
  void check_queue(size_t train_hs, int station_index_1, int station_index_2, int date_index);

 public:
  TrainSystem() = default;
  explicit TrainSystem(ManagementSystem *m_sys);
  void load_management_system(ManagementSystem *m_sys);
  auto add_train(const std::string &train_id, int seat_num, const vector<std::string> &stations,
                 const vector<int> &prices, const Time &start_time, const vector<int> &travel_times,
                 const vector<int> &stop_over_times, const DateRange &sale_date, char type) -> bool;

  auto delete_train(const std::string &train_id) -> bool;
  auto release_train(const std::string &train_id) -> bool;
  auto query_train(const std::string &train_id, const Date &date) -> bool;
  void query_ticket(const std::string &station_1, const std::string &station_2, const Date &date,
                    const QueryType &type);

  auto query_transfer(const std::string &station_1, const std::string &station_2, const Date &date,
                      const QueryType &type) -> bool;

  auto buy_ticket(int time_stamp, const std::string &user_name, const std::string &train_id, const Date &date, int num,
                  const std::string &station_1, const std::string &station_2, bool wait) -> bool;
  auto query_order(const std::string &user_name) -> bool;
  auto refund_ticket(const std::string &user_name, int n) -> bool;
  void clear();

  /*
   * debugging functions
   */
#ifdef DEBUG_FILE_IN_TMP
//  void print_queue() {
//    std::cout << "user_name train_id station_index_1 station_index_2 date_index num trade_index\n";
//    for (auto it = q_sys_.begin(); it != q_sys_.end(); ++it) {
//      std::cout << *it;
//    }
//  }
#endif
};
}  // namespace CrazyDave
#endif  // TICKET_SYSTEM_TRAIN_HPP
