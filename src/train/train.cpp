#include "train/train.hpp"
namespace CrazyDave {

TrainSystem::TrainSystem(ManagementSystem *m_sys) : m_sys_(m_sys) {}
auto TrainSystem::add_train(const std::string &train_id, int seat_num, const vector<std::string> &stations,
                            const vector<int> &prices, const Time &start_time, const vector<int> &travel_times,
                            const vector<int> &stop_over_times, const DateRange &sale_date, const char type) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  vector<TrainMeta> meta_vec;
  meta_storage_.find(train_hs, meta_vec);
  if (!meta_vec.empty()) {
    return false;
  }
  TrainMeta meta{train_id, (int)stations.size(), seat_num, sale_date, type};
  TrainArray array{meta.station_num_, stations, prices, start_time, travel_times, stop_over_times};
  t_io_.insert_array(train_hs, meta, array);
  meta_storage_.insert(train_hs, meta);
  return true;
}
auto TrainSystem::delete_train(const std::string &train_id) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  vector<TrainMeta> meta_vec;
  meta_storage_.find(train_hs, meta_vec);
  if (meta_vec.empty()) {
    return false;
  }
  auto &meta = meta_vec[0];
  if (meta.is_released_) {
    return false;
  }
  meta_storage_.remove(train_hs, meta);
  t_io_.remove_array(train_hs, meta.index_);
  return true;
}
auto TrainSystem::release_train(const std::string &train_id) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  vector<TrainMeta> meta_vec;
  meta_storage_.find(train_hs, meta_vec);
  if (meta_vec.empty()) {
    return false;
  }
  auto &meta = meta_vec[0];
  if (meta.is_released_) {
    return false;
  }
  meta_storage_.remove(train_hs, meta);
  meta.is_released_ = true;
  meta_storage_.insert(train_hs, meta);
  TrainArray array;
  t_io_.read_array(meta.index_, array);
  for (short i = 0; i < meta.station_num_; ++i) {
    auto station_hs = HashBytes(array.stations_[i].c_str());
    station_storage_.insert(station_hs, {train_hs, i, array.time_ranges_[i], array.prices_[i]});
  }
  int date_num = meta.sale_date_range_.second - meta.sale_date_range_.first + 1;
  for (int i = 0; i < date_num; ++i) {
    DateInfo seat;
    seat.date_index_ = i;
    std::fill_n(seat.seat_num_, meta.station_num_, meta.seat_num_);
    date_info_storage_.insert({train_hs, i}, seat);
  }

  return true;
}
auto TrainSystem::query_train(const std::string &train_id, const Date &date) -> bool {
  auto train_hs = HashBytes(train_id.c_str());
  vector<TrainMeta> meta_vec;
  meta_storage_.find(train_hs, meta_vec);
  if (meta_vec.empty()) {
    return false;
  }
  auto &meta = meta_vec[0];

  if (meta.sale_date_range_.first > date || meta.sale_date_range_.second < date) {
    return false;
  }

  std::cout << meta.train_id_ << " " << meta.type_ << "\n";
  DateTimeRange start_time{{date, {}}, {date, {}}};
  TrainArray array;
  t_io_.read_array(meta.index_, array);
  if (!meta.is_released_) {
    for (int i = 0; i < meta.station_num_; ++i) {
      std::cout << array.stations_[i] << " " << start_time + array.time_ranges_[i] << " " << array.prices_[i] << " ";
      if (i < meta.station_num_ - 1) {
        std::cout << meta.seat_num_;
      } else {
        std::cout << "x";
      }
      std::cout << "\n";
    }
    return true;
  }
  int j = date - meta.sale_date_range_.first;
  vector<DateInfo> seat_vec;
  date_info_storage_.find({train_hs, j}, seat_vec);
  auto &seat_num = seat_vec[0].seat_num_;
  for (int i = 0; i < meta.station_num_; ++i) {
    std::cout << array.stations_[i] << " " << start_time + array.time_ranges_[i] << " " << array.prices_[i] << " ";
    if (i < meta.station_num_ - 1) {
      std::cout << seat_num[i];
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
  linked_hashmap<size_t, Record> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_hs, rec});
  }
  for (auto &rec_1 : record_vec_1) {
    vector<TrainMeta> meta_vec;
    meta_storage_.find(rec_1.train_hs, meta_vec);
    auto &meta = meta_vec[0];
    int i1 = rec_1.index_;  // station index
    if (i1 == meta.station_num_ - 1 || !meta.is_released_) {
      continue;
    }

    int offset = rec_1.time_range_.second.date.day_;
    auto depart_date = date - offset;  // 发车日期
    if (meta.sale_date_range_.first > depart_date || meta.sale_date_range_.second < depart_date) {
      continue;
    }
    auto it = rec_map.find(rec_1.train_hs);
    if (it == rec_map.end()) {
      continue;
    }
    auto &rec_2 = it->second;
    int i2 = rec_2.index_;  // station index
    if (i1 >= i2) {         // 倒过来开？
      continue;
    }
    int min_num = meta.seat_num_;
    int j = depart_date - meta.sale_date_range_.first;  // date index

    vector<DateInfo> seat_vec;
    date_info_storage_.find({rec_1.train_hs, j}, seat_vec);

    auto &seat_num = seat_vec[0].seat_num_;
    for (int i = i1; i < i2; ++i) {
      min_num = std::min(min_num, seat_num[i]);
    }

    auto depart_date_time = DateTime{date, rec_1.time_range_.second.time};
    DateTime arrive_date_time{depart_date + rec_2.time_range_.first.date.day_, rec_2.time_range_.first.time};
    res_vec.push_back({meta.train_id_, {depart_date_time, arrive_date_time}, rec_2.price_ - rec_1.price_, min_num});
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
  TransferResult *res = nullptr;
  auto station_hs_1 = HashBytes(station_1.c_str());
  auto station_hs_2 = HashBytes(station_2.c_str());
  vector<Record> record_vec_1;
  vector<Record> record_vec_2;
  station_storage_.find(station_hs_1, record_vec_1);
  station_storage_.find(station_hs_2, record_vec_2);

  linked_hashmap<size_t, int> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_hs, rec.index_});
  }

  for (auto &rec_1 : record_vec_1) {
    vector<TrainMeta> meta_vec_1;
    meta_storage_.find(rec_1.train_hs, meta_vec_1);
    auto &meta_1 = meta_vec_1[0];
    if (!meta_1.is_released_) {
      continue;
    }
    TrainArray array_1;
    t_io_.read_array(meta_1.index_, array_1);
    int i1 = rec_1.index_;  // station index
    int offset_1 = array_1.time_ranges_[i1].second.date.day_;
    auto depart_date_1 = date - offset_1;                    // train_1发车日期
    int j1 = depart_date_1 - meta_1.sale_date_range_.first;  // date index
    if (meta_1.sale_date_range_.first > depart_date_1 || meta_1.sale_date_range_.second < depart_date_1) {
      continue;
    }
    int min_num_1 = meta_1.seat_num_;
    vector<DateInfo> seat_vec_1;
    date_info_storage_.find({rec_1.train_hs, j1}, seat_vec_1);
    auto &seat_num_1 = seat_vec_1[0].seat_num_;

    for (int i = i1 + 1; i < meta_1.station_num_; ++i) {
      min_num_1 = std::min(min_num_1, seat_num_1[i - 1]);
      auto &station_3 = array_1.stations_[i];
      auto station_hs_3 = HashBytes(station_3.c_str());
      vector<Record> record_vec_3;
      station_storage_.find(station_hs_3, record_vec_3);
      for (auto &rec_3 : record_vec_3) {
        auto it = rec_map.find(rec_3.train_hs);
        if (it == rec_map.end() || rec_3.train_hs == rec_1.train_hs) {
          continue;
        }

        vector<TrainMeta> meta_vec_2;
        meta_storage_.find(rec_3.train_hs, meta_vec_2);
        auto &meta_2 = meta_vec_2[0];

        if (!meta_2.is_released_) {
          continue;
        }
        TrainArray array_2;
        t_io_.read_array(meta_2.index_, array_2);
        int i3 = rec_3.index_;  // station index of train_2
        int offset_2 = array_2.time_ranges_[i3].second.date.day_;

        // 检测train1到达早于train2发车
        auto depart_date_2 = depart_date_1 + array_1.time_ranges_[i].first.date.day_ - offset_2;
        if (array_2.time_ranges_[i3].second.time < array_1.time_ranges_[i].first.time) {
          depart_date_2 += 1;
        }
        if (depart_date_2 < meta_2.sale_date_range_.first) {
          depart_date_2 = meta_2.sale_date_range_.first;
        }

        if (meta_2.sale_date_range_.second < depart_date_2) {
          continue;
        }
        int i2 = it->second;
        if (i3 >= i2) {
          continue;
        }
        int j2 = depart_date_2 - meta_2.sale_date_range_.first;  // date index
        int min_num_2 = meta_2.seat_num_;
        vector<DateInfo> seat_vec_2;
        date_info_storage_.find({rec_3.train_hs, j2}, seat_vec_2);
        auto &seat_num_2 = seat_vec_2[0].seat_num_;

        for (int k = i3; k < i2; ++k) {
          min_num_2 = std::min(min_num_2, seat_num_2[k]);
        }
        DateTime depart_date_time_1{date, array_1.time_ranges_[i1].second.time};  // 从station_1出发的时间
        DateTime arrive_date_time_1{depart_date_1 + array_1.time_ranges_[i].first.date.day_,
                                    array_1.time_ranges_[i].first.time};  // 到达station_3的时间
        DateTime depart_date_time_2{depart_date_2 + array_2.time_ranges_[i3].second.date.day_,
                                    array_2.time_ranges_[i3].second.time};  // 从station_3出发的时间
        DateTime arrive_date_time_2{depart_date_2 + array_2.time_ranges_[i2].first.date.day_,
                                    array_2.time_ranges_[i2].first.time};  // 到达station_2的时间
        auto *res_1 = new TransferResult{{meta_1.train_id_,
                                          {
                                              depart_date_time_1,
                                              arrive_date_time_1,
                                          },
                                          array_1.prices_[i] - array_1.prices_[i1],
                                          min_num_1},
                                         {meta_2.train_id_,
                                          {
                                              depart_date_time_2,
                                              arrive_date_time_2,
                                          },
                                          array_2.prices_[i2] - array_2.prices_[i3],
                                          min_num_2},
                                         station_3};
        if (res == nullptr) {
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

            if (cmp(*res_1, *res)) {
              res = res_1;
            } else {
              delete res_1;
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
            if (cmp(*res_1, *res)) {
              res = res_1;
            } else {
              delete res_1;
            }
          }
        }
      }
    }
  }
  if (success) {
    std::cout << res->res_1.train_id << " " << station_1 << " " << res->res_1.range.first << " -> " << res->mid_station
              << " " << res->res_1.range.second << " " << res->res_1.price << " " << res->res_1.max_num << "\n";
    std::cout << res->res_2.train_id << " " << res->mid_station << " " << res->res_2.range.first << " -> " << station_2
              << " " << res->res_2.range.second << " " << res->res_2.price << " " << res->res_2.max_num << "\n";
    delete res;
    return true;
  }
  delete res;
  return false;
}
auto TrainSystem::buy_ticket(int time_stamp, const std::string &user_name, const std::string &train_id,
                             const Date &date, int num, const std::string &station_1, const std::string &station_2,
                             bool wait) -> bool {
  if (!m_sys_->check_is_login(user_name)) {
    return false;
  }

  auto train_hs = HashBytes(train_id.c_str());
  vector<TrainMeta> meta_vec;
  meta_storage_.find(train_hs, meta_vec);
  auto &meta = meta_vec[0];
  if (!meta.is_released_ || num > meta.seat_num_) {
    return false;
  }

  TrainArray array;
  t_io_.read_array(meta.index_, array);
  short i1 = -1, i2 = -1, j;
  int min_num = meta.seat_num_;
  Date depart_date;

  vector<DateInfo> seat_vec;

  for (short i = 0; i < meta.station_num_; ++i) {
    if (array.stations_[i] == station_2) {
      i2 = i;
      break;
    }
    if (array.stations_[i] == station_1) {
      i1 = i;
      int offset = array.time_ranges_[i].second.date.day_;
      depart_date = date - offset;
      j = depart_date - meta.sale_date_range_.first;
      if (meta.sale_date_range_.first > depart_date || meta.sale_date_range_.second < depart_date) {
        return false;
      }
      date_info_storage_.find({train_hs, j}, seat_vec);
    }
    if (i1 != -1) {
      min_num = std::min(min_num, seat_vec[0].seat_num_[i]);
    }
  }

  if (i1 == -1 || i2 == -1) {
    return false;
  }
  auto user_hs = HashBytes(user_name.c_str());
  if (min_num >= num) {
    auto &seat_num = seat_vec[0].seat_num_;
    date_info_storage_.remove({train_hs, j}, seat_vec[0]);
    for (int i = i1; i < i2; ++i) {
      seat_num[i] -= num;
    }
    date_info_storage_.insert({train_hs, j}, seat_vec[0]);
    std::cout << (array.prices_[i2] - array.prices_[i1]) * num << "\n";
    trade_storage_.insert(
        user_hs, Trade{time_stamp, Status::SUCCESS, train_id, DateTime{depart_date, {}} + array.time_ranges_[i1].second,
                       DateTime{depart_date, {}} + array.time_ranges_[i2].first, station_1, i1, station_2, i2,
                       array.prices_[i2] - array.prices_[i1], num, j});

    return true;
  } else {
    if (wait) {
      vector<Trade> trade_vec;
      trade_storage_.find(user_hs, trade_vec);
      q_sys_.push({user_hs, train_hs, i1, i2, j, num, (int)trade_vec.size()});
      trade_storage_.insert(user_hs, Trade{time_stamp, Status::PENDING, train_id,
                                           DateTime{depart_date, {}} + array.time_ranges_[i1].second,
                                           DateTime{depart_date, {}} + array.time_ranges_[i2].first, station_1, i1,
                                           station_2, i2, array.prices_[i2] - array.prices_[i1], num, j});
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
    vector<DateInfo> seat_vec;
    date_info_storage_.find({train_hs, trade.date_index_}, seat_vec);
    auto &seat = seat_vec[0];
    auto &seat_num = seat.seat_num_;

    date_info_storage_.remove({train_hs, trade.date_index_}, seat);
    for (int i = trade.station_index_1_; i < trade.station_index_2_; ++i) {
      seat_num[i] += trade.num_;
    }
    date_info_storage_.insert({train_hs, trade.date_index_}, seat);
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

    vector<DateInfo> seat_vec;
    date_info_storage_.find({train_hs, date_index}, seat_vec);
    auto &seat = seat_vec[0];
    auto &seat_num = seat.seat_num_;
    int min_num = seat_num[it->station_index_1_];
    for (int i = it->station_index_1_; i < it->station_index_2_; ++i) {
      min_num = std::min(min_num, seat_num[i]);
    }
    if (min_num < it->num_) {
      ++it;
      continue;
    }
    // 补票成功
    auto query = *it;
    q_sys_.erase(it++);

    date_info_storage_.remove({train_hs, date_index}, seat);
    for (int i = query.station_index_1_; i < query.station_index_2_; ++i) {
      seat_num[i] -= query.num_;
    }
    date_info_storage_.insert({train_hs, date_index}, seat);
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