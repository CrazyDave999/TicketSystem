#include "train/train.hpp"

#include <utility>
namespace CrazyDave {

Train::Train(const std::string &train_id, int seat_num, const vector<std::string> &stations, const vector<int> &prices,
             const Time &start_time, const vector<int> &travel_times, const vector<int> &stop_over_times,
             DateRange sale_date, const char type)
    : train_id_(train_id),
      station_num_((int)stations.size()),
      seat_num_(seat_num),
      sale_date_range_(std::move(sale_date)),
      type_(type) {
  DateTimeRange time_range{DateTime::INVALID_DATE_TIME, {{}, start_time}};
  int price_sum = 0;
  for (int i = 0; i < station_num_; ++i) {
    stations_[i] = stations[i];
    prices_[i] = price_sum;
    if (i < station_num_ - 1) {
      price_sum += prices[i];
    }
    if (i == station_num_ - 1) {
      time_range.second = DateTime::INVALID_DATE_TIME;
    }
    time_ranges_[i] = time_range;
    if (i < station_num_ - 1) {
      time_range.first = time_range.second + travel_times[i];
    }
    if (i < station_num_ - 2) {
      time_range.second = time_range.first + stop_over_times[i];
    }
  }
}

TrainSystem::TrainSystem(ManagementSystem *m_sys) : m_sys_(m_sys) {}
auto TrainSystem::add_train(const std::string &train_id, int seat_num, const vector<std::string> &stations,
                            const vector<int> &prices, const Time &start_time, const vector<int> &travel_times,
                            const vector<int> &stop_over_times, const DateRange &sale_date, const char type) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  if (t_io_.check_exist(train_hs)) {
    return false;
  }
  Train train{train_id, seat_num, stations, prices, start_time, travel_times, stop_over_times, sale_date, type};
  t_io_.insert_train(train_hs, train);
  return true;
}
auto TrainSystem::delete_train(const std::string &train_id) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  if (!t_io_.check_exist(train_hs)) {
    return false;
  }
  Train train;
  t_io_.read_train(train_hs, train);
  if (train.is_released_) {
    return false;
  }
  t_io_.remove_train(train);
  return true;
}
auto TrainSystem::release_train(const std::string &train_id) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  if (!t_io_.check_exist(train_hs)) {
    return false;
  }
  Train train;
  t_io_.read_train(train_hs, train);
  if (train.is_released_) {
    return false;
  }
  train.is_released_ = true;
  t_io_.write_train(train);
  for (int i = 0; i < train.station_num_; ++i) {
    auto station_hs = HashBytes(train.stations_[i].c_str());
    station_storage_.insert(station_hs, {train_hs, i});
  }
  Seat seat{};

  for (int i = 0; i < train.station_num_; ++i) {
    for (int j = 0; j < 92; ++j) {
      seat.seat_num_[i][j] = train.seat_num_;
    }
  }
  t_io_.insert_seat(train_hs, seat);
  return true;
}
auto TrainSystem::query_train(const std::string &train_id, const Date &date) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  if (!t_io_.check_exist(train_hs)) {
    return false;
  }
  Train train;
  t_io_.read_train(train_hs, train);
  if (train.sale_date_range_.first > date || train.sale_date_range_.second < date) {
    return false;
  }

  std::cout << train.train_id_ << " " << train.type_ << "\n";
  DateTimeRange start_time{{date, {}}, {date, {}}};
  if (!train.is_released_) {
    for (int i = 0; i < train.station_num_; ++i) {
      std::cout << train.stations_[i] << " " << start_time + train.time_ranges_[i] << " " << train.prices_[i] << " ";
      if (i < train.station_num_ - 1) {
        std::cout << train.seat_num_;
      } else {
        std::cout << "x";
      }
      std::cout << "\n";
    }
    return true;
  }
  int j = date - train.sale_date_range_.first;
  Seat seat;
  t_io_.read_seat(train_hs, seat);
  auto &seat_num = seat.seat_num_;
  for (int i = 0; i < train.station_num_; ++i) {
    std::cout << train.stations_[i] << " " << start_time + train.time_ranges_[i] << " " << train.prices_[i] << " ";
    if (i < train.station_num_ - 1) {
      std::cout << seat_num[i][j];
    } else {
      std::cout << "x";
    }
    std::cout << "\n";
  }
  return true;
}
void TrainSystem::query_ticket(const std::string &station_1, const std::string &station_2, const Date &date,
                               const QueryType &type) {
  vector<TicketResult> res_vec;
  auto station_hs_1 = HashBytes(station_1.c_str());
  auto station_hs_2 = HashBytes(station_2.c_str());
  vector<Record> record_vec_1;
  vector<Record> record_vec_2;

  station_storage_.find(station_hs_1, record_vec_1);
  station_storage_.find(station_hs_2, record_vec_2);
  linked_hashmap<size_t, int> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_hs, rec.index});
  }
  for (auto &rec : record_vec_1) {
    Train train;
    t_io_.read_train(rec.train_hs, train);
    int i1 = rec.index;  // station index
    if (i1 == train.station_num_ - 1 || !train.is_released_) {
      continue;
    }
    int offset = train.time_ranges_[i1].second.date.day_;
    auto depart_date = date - offset;  // 发车日期
    if (train.sale_date_range_.first > depart_date || train.sale_date_range_.second < depart_date) {
      continue;
    }
    auto it = rec_map.find(rec.train_hs);
    if (it == rec_map.end()) {
      continue;
    }
    int i2 = it->second;  // station index
    if (i1 >= i2) {       // 倒过来开？
      continue;
    }
    int min_num = train.seat_num_;
    int j = depart_date - train.sale_date_range_.first;  // date index
    Seat seat;
    t_io_.read_seat(rec.train_hs, seat);
    auto &seat_num = seat.seat_num_;
    for (int i = i1; i < i2; ++i) {
      min_num = std::min(min_num, seat_num[i][j]);
    }
    auto depart_date_time = DateTime{date, train.time_ranges_[i1].second.time};
    auto arrive_date_time =
        DateTime{depart_date + train.time_ranges_[i2].first.date.day_, train.time_ranges_[i2].first.time};
    res_vec.push_back(
        {train.train_id_, {depart_date_time, arrive_date_time}, train.prices_[i2] - train.prices_[i1], min_num});
  }
  if (type == QueryType::TIME) {
    res_vec.sort([](const TicketResult &r1, const TicketResult &r2) {
      if (r1.total_time() != r2.total_time()) {
        return r1.total_time() < r2.total_time();
      }
      return r1.train_id < r2.train_id;
    });
  } else {
    res_vec.sort([](const TicketResult &r1, const TicketResult &r2) {
      if (r1.price != r2.price) {
        return r1.price < r2.price;
      }
      return r1.train_id < r2.train_id;
    });
  }
  std::cout << res_vec.size() << "\n";
  for (auto &res : res_vec) {
    std::cout << res.train_id << " " << station_1 << " " << res.range.first << " -> " << station_2 << " "
              << res.range.second << " " << res.price << " " << res.max_num << "\n";
  }
}
auto TrainSystem::query_transfer(const std::string &station_1, const std::string &station_2, const Date &date,
                                 const QueryType &type) -> bool {
  bool success = false;
  TransferResult res;
  auto station_hs_1 = HashBytes(station_1.c_str());
  auto station_hs_2 = HashBytes(station_2.c_str());
  vector<Record> record_vec_1;
  vector<Record> record_vec_2;
  station_storage_.find(station_hs_1, record_vec_1);
  station_storage_.find(station_hs_2, record_vec_2);

  linked_hashmap<size_t, int> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_hs, rec.index});
  }

  for (auto &rec_1 : record_vec_1) {
    Train train_1;
    t_io_.read_train(rec_1.train_hs, train_1);
    if (!train_1.is_released_) {
      continue;
    }
    int i1 = rec_1.index;  // station index
    int offset_1 = train_1.time_ranges_[i1].second.date.day_;
    auto depart_date_1 = date - offset_1;                     // train_1发车日期
    int j1 = depart_date_1 - train_1.sale_date_range_.first;  // date index
    if (train_1.sale_date_range_.first > depart_date_1 || train_1.sale_date_range_.second < depart_date_1) {
      continue;
    }
    int min_num_1 = train_1.seat_num_;
    Seat seat_1;
    t_io_.read_seat(rec_1.train_hs, seat_1);
    auto &seat_num_1 = seat_1.seat_num_;

    for (int i = i1 + 1; i < train_1.station_num_; ++i) {
      min_num_1 = std::min(min_num_1, seat_num_1[i - 1][j1]);
      auto &station_3 = train_1.stations_[i];
      auto station_hs_3 = HashBytes(station_3.c_str());
      vector<Record> record_vec_3;
      station_storage_.find(station_hs_3, record_vec_3);
      for (auto &rec_3 : record_vec_3) {
        auto it = rec_map.find(rec_3.train_hs);
        if (it == rec_map.end() || rec_3.train_hs == rec_1.train_hs) {
          continue;
        }
        Train train_2;
        t_io_.read_train(rec_3.train_hs, train_2);
        if (!train_2.is_released_) {
          continue;
        }
        int i3 = rec_3.index;  // station index of train_2
        int offset_2 = train_2.time_ranges_[i3].second.date.day_;

        // 检测train1到达早于train2发车
        auto depart_date_2 = depart_date_1 + train_1.time_ranges_[i].first.date.day_ - offset_2;
        if (train_2.time_ranges_[i3].second.time < train_1.time_ranges_[i].first.time) {
          depart_date_2 += 1;
        }
        if (depart_date_2 < train_2.sale_date_range_.first) {
          depart_date_2 = train_2.sale_date_range_.first;
        }

        int j2 = depart_date_2 - train_2.sale_date_range_.first;  // date index
        if (train_2.sale_date_range_.second < depart_date_2) {
          continue;
        }
        int i2 = it->second;
        if (i3 >= i2) {
          continue;
        }
        int min_num_2 = train_2.seat_num_;
        Seat seat_2;
        t_io_.read_seat(rec_3.train_hs, seat_2);
        auto &seat_num_2 = seat_2.seat_num_;
        for (int k = i3; k < i2; ++k) {
          min_num_2 = std::min(min_num_2, seat_num_2[k][j2]);
        }
        DateTime depart_date_time_1{date, train_1.time_ranges_[i1].second.time};  // 从station_1出发的时间
        DateTime arrive_date_time_1{depart_date_1 + train_1.time_ranges_[i].first.date.day_,
                                    train_1.time_ranges_[i].first.time};  // 到达station_3的时间
        DateTime depart_date_time_2{depart_date_2 + train_2.time_ranges_[i3].second.date.day_,
                                    train_2.time_ranges_[i3].second.time};  // 从station_3出发的时间
        DateTime arrive_date_time_2{depart_date_2 + train_2.time_ranges_[i2].first.date.day_,
                                    train_2.time_ranges_[i2].first.time};  // 到达station_2的时间
        TransferResult res_1{{train_1.train_id_,
                              {
                                  depart_date_time_1,
                                  arrive_date_time_1,
                              },
                              train_1.prices_[i] - train_1.prices_[i1],
                              min_num_1},
                             {train_2.train_id_,
                              {
                                  depart_date_time_2,
                                  arrive_date_time_2,
                              },
                              train_2.prices_[i2] - train_2.prices_[i3],
                              min_num_2},
                             station_3};
        if (!success) {
          success = true;
          res = res_1;
        } else {
          if (type == QueryType::TIME) {
            auto cmp = [](const TransferResult &t1, const TransferResult &t2) {
              if (t1.total_time() != t2.total_time()) {
                return t1.total_time() < t2.total_time();
              }
              if (t1.total_price() != t2.total_price()) {
                return t1.total_price() < t2.total_price();
              }
              if (t1.res_1.train_id != t2.res_1.train_id) {
                return t1.res_1.train_id < t2.res_1.train_id;
              }
              return t1.res_2.train_id < t2.res_2.train_id;
            };
            if (cmp(res_1, res)) {
              res = res_1;
            }
          } else {
            auto cmp = [](const TransferResult &t1, const TransferResult &t2) {
              if (t1.total_price() != t2.total_price()) {
                return t1.total_price() < t2.total_price();
              }
              if (t1.total_time() != t2.total_time()) {
                return t1.total_time() < t2.total_time();
              }
              if (t1.res_1.train_id != t2.res_1.train_id) {
                return t1.res_1.train_id < t2.res_1.train_id;
              }
              return t1.res_2.train_id < t2.res_2.train_id;
            };
            if (cmp(res_1, res)) {
              res = res_1;
            }
          }
        }
      }
    }
  }
  if (success) {
    std::cout << res.res_1.train_id << " " << station_1 << " " << res.res_1.range.first << " -> " << res.mid_station
              << " " << res.res_1.range.second << " " << res.res_1.price << " " << res.res_1.max_num << "\n";
    std::cout << res.res_2.train_id << " " << res.mid_station << " " << res.res_2.range.first << " -> " << station_2
              << " " << res.res_2.range.second << " " << res.res_2.price << " " << res.res_2.max_num << "\n";
    return true;
  }
  return false;
}
auto TrainSystem::buy_ticket(int time_stamp, const std::string &user_name, const std::string &train_id,
                             const Date &date, int num, const std::string &station_1, const std::string &station_2,
                             bool wait) -> bool {
  if (!m_sys_->check_is_login(user_name)) {
    return false;
  }

  auto train_hs = HashBytes(train_id.c_str());
  Train train;
  t_io_.read_train(train_hs, train);
  if (!train.is_released_ || num > train.seat_num_) {
    return false;
  }

  int i1 = -1, i2 = -1, j;
  int min_num = train.seat_num_;
  Date depart_date;
  Seat seat;
  t_io_.read_seat(train_hs, seat);
  auto &seat_num = seat.seat_num_;
  for (int i = 0; i < train.station_num_; ++i) {
    if (train.stations_[i] == station_2) {
      i2 = i;
      break;
    }
    if (train.stations_[i] == station_1) {
      i1 = i;
      int offset = train.time_ranges_[i].second.date.day_;
      depart_date = date - offset;
      j = depart_date - train.sale_date_range_.first;
      if (train.sale_date_range_.first > depart_date || train.sale_date_range_.second < depart_date) {
        return false;
      }
    }
    if (i1 != -1) {
      min_num = std::min(min_num, seat_num[i][j]);
    }
  }

  if (i1 == -1 || i2 == -1) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  if (min_num >= num) {
    for (int i = i1; i < i2; ++i) {
      seat_num[i][j] -= num;
    }
    t_io_.write_seat(train_hs, seat);
    std::cout << (train.prices_[i2] - train.prices_[i1]) * num << "\n";
    trade_storage_.insert(
        user_hs, Trade{time_stamp, Status::SUCCESS, train_id, DateTime{depart_date, {}} + train.time_ranges_[i1].second,
                       DateTime{depart_date, {}} + train.time_ranges_[i2].first, station_1, i1, station_2, i2,
                       train.prices_[i2] - train.prices_[i1], num, j});

    return true;
  } else {
    if (wait) {
      vector<Trade> trade_vec;
      trade_storage_.find(user_hs, trade_vec);
      q_sys_.push({user_hs, train_hs, i1, i2, j, num, (int)trade_vec.size()});
      trade_storage_.insert(user_hs, Trade{time_stamp, Status::PENDING, train_id,
                                           DateTime{depart_date, {}} + train.time_ranges_[i1].second,
                                           DateTime{depart_date, {}} + train.time_ranges_[i2].first, station_1, i1,
                                           station_2, i2, train.prices_[i2] - train.prices_[i1], num, j});
      std::cout << "queue\n";
      return true;
    }
    return false;
  }
}
auto TrainSystem::refund_ticket(const std::string &user_name, int n) -> bool {
  if (!m_sys_->check_is_login(user_name)) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  vector<Trade> trade_vec;
  trade_storage_.find(user_hs, trade_vec);
  if (n > (int)trade_vec.size()) {
    return false;
  }
  auto &trade = trade_vec[n - 1];
  if (trade.status_ == Status::REFUNDED) {
    return false;
  }

  if (trade.status_ == Status::SUCCESS) {
    auto train_hs = HashBytes(trade.train_id_.c_str());

    // 还原座位数量
    Seat seat;
    t_io_.read_seat(train_hs, seat);
    auto &seat_num = seat.seat_num_;

    for (int i = trade.station_index_1_; i < trade.station_index_2_; ++i) {
      seat_num[i][trade.date_index_] += trade.num_;
    }
    t_io_.write_seat(train_hs, seat);
    check_queue(train_hs, trade.station_index_1_, trade.station_index_2_, trade.date_index_);
  } else {
    for (auto it = q_sys_.begin(); it != q_sys_.end(); ++it) {
      if (it->user_hs_ == user_hs && (int)(trade_vec.size() - it->trade_index_) == n) {
        q_sys_.erase(it);
        break;
      }
    }
  }
  trade_storage_.remove(user_hs, trade);
  trade.status_ = Status::REFUNDED;
  trade_storage_.insert(user_hs, trade);
  return true;
}
void TrainSystem::check_queue(size_t train_hs, int station_index_1, int station_index_2, int date_index) {
  auto it = q_sys_.begin();
  while (it != q_sys_.end()) {
    if (it->train_hs_ != train_hs || it->date_index_ != date_index || it->station_index_2_ < station_index_1 ||
        it->station_index_1_ > station_index_2) {
      ++it;
      continue;
    }

    Seat seat;
    t_io_.read_seat(train_hs, seat);
    auto &seat_num = seat.seat_num_;
    int min_num = seat_num[it->station_index_1_][date_index];
    for (int i = it->station_index_1_; i < it->station_index_2_; ++i) {
      min_num = std::min(min_num, seat_num[i][date_index]);
    }
    if (min_num < it->num_) {
      ++it;
      continue;
    }
    // 补票成功
    auto query = *it;
    q_sys_.erase(it++);

    for (int i = query.station_index_1_; i < query.station_index_2_; ++i) {
      seat_num[i][date_index] -= query.num_;
    }
    t_io_.write_seat(train_hs, seat);
    vector<Trade> trade_vec;
    trade_storage_.find(query.user_hs_, trade_vec);
    auto &trade = trade_vec[trade_vec.size() - 1 - query.trade_index_];
    trade_storage_.remove(query.user_hs_, trade);
    trade.status_ = Status::SUCCESS;
    trade_storage_.insert(query.user_hs_, trade);
  }
}
auto TrainSystem::query_order(const std::string &user_name) -> bool {
  if (!m_sys_->check_is_login(user_name)) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  vector<Trade> trade_vec;
  trade_storage_.find(user_hs, trade_vec);
  std::cout << trade_vec.size() << "\n";
  for (auto &trade : trade_vec) {
    std::cout << trade;
  }
  return true;
}
void TrainSystem::clear() {
  //  train_storage_.clear();
  //  trade_storage_.clear();
  //  station_storage_.clear();
  q_sys_.reset();
}
void TrainSystem::load_management_system(ManagementSystem *m_sys) { m_sys_ = m_sys; }

}  // namespace CrazyDave