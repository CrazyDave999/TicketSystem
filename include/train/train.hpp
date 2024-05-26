#ifndef TICKET_SYSTEM_TRAIN_HPP
#define TICKET_SYSTEM_TRAIN_HPP
#include <iostream>
#include <string>
#include <utility>
#include "common/management_system.hpp"
#include "common/utils.hpp"

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
class TrainIO;
class TrainMeta {
  friend TrainSystem;
  friend TrainIO;
  String<20> train_id_{};
  int station_num_{};
  int seat_num_{};
  DateRange sale_date_range_{};
  char type_{};
  bool is_released_{};
  size_t index_{};  // the position in storage file
 public:
  TrainMeta() = default;
  TrainMeta(const std::string &train_id, int station_num, int seat_num, DateRange sale_date_range, char type)
      : train_id_(train_id),
        station_num_(station_num),
        seat_num_(seat_num),
        sale_date_range_(std::move(sale_date_range)),
        type_(type) {}
  auto operator<(const TrainMeta &rhs) const -> bool { return train_id_ < rhs.train_id_; }
};
class TrainArray {
  friend TrainSystem;
  friend TrainIO;

 private:
  String<30> stations_[100]{};      // [station]
  int prices_[99]{};                // [station]
  DateTimeRange time_ranges_[100];  // [station], {arrival time, leaving time},
                                    // record the offset time from the start_time

 public:
  TrainArray() = default;
  TrainArray(int station_num, const vector<std::string> &stations, const vector<int> &prices, const Time &start_time,
             const vector<int> &travel_times, const vector<int> &stop_over_times) {
    DateTimeRange time_range{DateTime::INVALID_DATE_TIME, {{}, start_time}};
    int price_sum = 0;
    for (int i = 0; i < station_num; ++i) {
      stations_[i] = stations[i];
      prices_[i] = price_sum;
      if (i < station_num - 1) {
        price_sum += prices[i];
      }
      if (i == station_num - 1) {
        time_range.second = DateTime::INVALID_DATE_TIME;
      }
      time_ranges_[i] = time_range;
      if (i < station_num - 1) {
        time_range.first = time_range.second + travel_times[i];
      }
      if (i < station_num - 2) {
        time_range.second = time_range.first + stop_over_times[i];
      }
    }
  }

  //  auto operator<(const Train &rhs) const -> bool { return train_id_ < rhs.train_id_; }
};
struct DateInfo {
  int date_index_{};
  int seat_num_[100]{};  // [station][date] date 是从始发站出发的日期 index
  auto operator!=(const DateInfo &rhs) const -> bool { return date_index_ != rhs.date_index_; }
  auto operator<(const DateInfo &rhs) const -> bool { return date_index_ < rhs.date_index_; }
};

class TrainIO {
 private:
#ifdef DEBUG_FILE_IN_TMP
  BPT<size_t, size_t> index_storage_{"tmp/idx", 0, 15, 5};
  File train_storage_{"tmp/trn_st"};
  File seat_storage_{"tmp/s_st"};
#else
  BPT<size_t, size_t> index_storage_{"idx", 0, 60, 5};
  File array_storage_{"arr_st"};
  File garbage_storage_{"arr_gb"};
#endif
  static const size_t OFFSET = sizeof(size_t);
  static const size_t SIZE_OF_ARRAY = sizeof(TrainArray);
  size_t max_index_{0};
  list<size_t> queue_{};

 public:
  TrainIO() {
    array_storage_.open();
    garbage_storage_.open();
    if (!garbage_storage_.get_is_new()) {
      garbage_storage_.seekg(0);
      size_t size;
      garbage_storage_.read(size);
      garbage_storage_.read(max_index_);
      for (size_t i = 0; i < size; ++i) {
        int index;
        garbage_storage_.read(index);
        queue_.push_back(index);
      }
    }
  }
  ~TrainIO() {
    garbage_storage_.seekp(0);
    size_t size = queue_.size();
    garbage_storage_.write(size);
    garbage_storage_.write(max_index_);
    for (size_t i = 0; i < size; ++i) {
      size_t index = queue_.back();
      queue_.pop_back();
      garbage_storage_.write(index);
    }
    array_storage_.close();
    garbage_storage_.close();
  }

  auto allocate_index() -> size_t {
    if (!queue_.empty()) {
      size_t index = queue_.front();
      queue_.pop_front();
      return index;
    }
    return ++max_index_;
  }
  void deallocate_index(size_t index) { queue_.push_back(index); }
  void insert_array(size_t train_hs, TrainMeta &meta, TrainArray &array) {
    size_t index = allocate_index();
    index_storage_.insert(train_hs, index);
    meta.index_ = index;
    array_storage_.seekp(OFFSET + index * SIZE_OF_ARRAY);
    array_storage_.write(array);
  }

  void remove_array(size_t train_hs, size_t index) {
    index_storage_.remove(train_hs, index);
    deallocate_index(index);
  }

  void read_array(size_t index, TrainArray &array) {
    array_storage_.seekg(OFFSET + index * SIZE_OF_ARRAY);
    array_storage_.read(array);
  }
};
class TrainSystem {
  struct Record {
    size_t train_hs{};
    int index_{};
    DateTimeRange time_range_{};
    int price_{};
    auto operator<(const Record &rhs) const -> bool { return train_hs < rhs.train_hs; }
  };
  struct Trade {
    int time_stamp_{};
    Status status_{};
    String<20> train_id_{};
    DateTime leaving_time_{};
    DateTime arrival_time_{};
    String<30> station_1_{};
    String<30> station_2_{};
    int station_index_1_{};
    int station_index_2_{};
    int price_{};
    int num_{};
    int date_index_{};

   public:
    Trade() = default;
    Trade(int time_stamp, const Status &status, const String<20> &train_id, const DateTime &leaving_time,
          const DateTime &arrival_time, const String<30> &station_1, int station_index_1, const String<30> &station_2,
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
    String<30> mid_station{};
    [[nodiscard]] auto total_time() const -> int { return res_2.range.second - res_1.range.first; }
    [[nodiscard]] auto total_price() const -> int { return res_1.price + res_2.price; }
  };

 private:
#ifdef DEBUG_FILE_IN_TMP
  MyBPlusTree<size_t, Seat> seat_storage_{"tmp/se1", "tmp/se2", "tmp/se3", "tmp/se4"};
  MyBPlusTree<size_t, Train> train_storage_{"tmp/tr1", "tmp/tr2", "tmp/tr3", "tmp/tr4"};
  BPT<size_t, Trade> trade_storage_{"tmp/trd", 0, 300, 30};
  BPT<size_t, Record> station_storage_{"tmp/st", 0, 300, 30};
#else

  BPT<size_t, TrainMeta> meta_storage_{"mta", 0, 60, 5};
  BPT<size_t, Trade> trade_storage_{"trd", 0, 60, 5};
  BPT<size_t, Record> station_storage_{"st", 0, 60, 5};
  BPT<pair<size_t, int>, DateInfo> date_info_storage_{"se", 0, 100, 5};

#endif
  QueueSystem q_sys_;
  ManagementSystem *m_sys_{};
  TrainIO t_io_{};

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
