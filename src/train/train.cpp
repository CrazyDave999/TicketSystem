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
    for (int j = 0; j < sale_date.second - sale_date.first + 1; ++j) {
      left_seat_num_[i][j] = seat_num;
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
auto Train::to_string(const Date &date) const -> std::string {
  DateTimeRange start_time{{date, {}}, {date, {}}};
  int j = date - sale_date_range_.first;
  std::stringstream ss;
  ss << train_id_ << " " << type_ << "\n";
  for (int i = 0; i < station_num_; ++i) {
    ss << stations_[i] << " " << start_time + time_ranges_[i] << " " << prices_[i] << " ";
    if (i < station_num_ - 1) {
      ss << left_seat_num_[i][j];
    } else {
      ss << "x";
    }
    ss << "\n";
  }
  return ss.str();
}

TrainSystem::TrainSystem(ManagementSystem *m_sys) : m_sys_(m_sys) {}
auto TrainSystem::add_train(const std::string &train_id, int seat_num, const vector<std::string> &stations,
                            const vector<int> &prices, const Time &start_time, const vector<int> &travel_times,
                            const vector<int> &stop_over_times, const DateRange &sale_date, const char type) -> bool {
  auto train_vec = train_storage_.find(train_id);
  if (!train_vec.empty()) {
    return false;
  }
  Train train{train_id, seat_num, stations, prices, start_time, travel_times, stop_over_times, sale_date, type};
  train_storage_.insert(train_id, train);
  return true;
}
auto TrainSystem::delete_train(const std::string &train_id) -> bool {
  auto train_vec = train_storage_.find(train_id);
  if (train_vec.empty() || train_vec[0].is_released_) {
    return false;
  }
  train_storage_.remove(train_id, train_vec[0]);
  return true;
}
auto TrainSystem::release_train(const std::string &train_id) -> bool {
  auto train_vec = train_storage_.find(train_id);
  if (train_vec.empty()) {
    return false;
  }
  auto &train = train_vec[0];
  if (train.is_released_) {
    return false;
  }
  train_storage_.remove(train_id, train);
  train.is_released_ = true;
  train_storage_.insert(train_id, train);
  for (int i = 0; i < train.station_num_; ++i) {
    station_storage_.insert(train.stations_[i], {train_id, i});
  }
  return true;
}
auto TrainSystem::query_train(const std::string &train_id, const Date &date) -> bool {
  auto train_vec = train_storage_.find(train_id);
  if (train_vec.empty()) {
    return false;
  }
  auto &train = train_vec[0];
  if (train.sale_date_range_.first > date || train.sale_date_range_.second < date) {
    return false;
  }
  std::cout << train.to_string(date);
  return true;
}
void TrainSystem::query_ticket(const std::string &station_1, const std::string &station_2, const Date &date,
                               const QueryType &type) {
  vector<TicketResult> res_vec;
  auto record_vec_1 = station_storage_.find(station_1);
  auto record_vec_2 = station_storage_.find(station_2);
  linked_hashmap<std::string, int> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_id_, rec.index});
  }
  for (auto &rec : record_vec_1) {
    auto train = train_storage_.find(rec.train_id_)[0];
    int i1 = rec.index;  // station index
    if (i1 == train.station_num_ - 1 || !train.is_released_) {
      continue;
    }
    int offset = train.time_ranges_[i1].second.date.day_;
    auto depart_date = date - offset;  // 发车日期
    if (train.sale_date_range_.first > depart_date || train.sale_date_range_.second < depart_date) {
      continue;
    }
    auto it = rec_map.find(rec.train_id_);
    if (it == rec_map.end()) {
      continue;
    }
    int i2 = it->second;  // station index
    if (i1 >= i2) {       // 倒过来开？
      continue;
    }
    int min_num = train.seat_num_;
    int j = depart_date - train.sale_date_range_.first;  // date index
    for (int i = i1; i < i2; ++i) {
      min_num = std::min(min_num, train.left_seat_num_[i][j]);
    }
    if (min_num == 0) {
      continue;
    }
    auto depart_date_time = DateTime{date, train.time_ranges_[i1].second.time};
    auto arrive_date_time =
        DateTime{depart_date + train.time_ranges_[i2].first.date.day_, train.time_ranges_[i2].first.time};
    res_vec.push_back({rec.train_id_,
                       {{train.stations_[i1], depart_date_time}, {train.stations_[i2], arrive_date_time}},
                       train.prices_[i2] - train.prices_[i1],
                       min_num});
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
    std::cout << res;
  }
}
auto TrainSystem::query_transfer(const std::string &station_1, const std::string &station_2, const Date &date,
                                 const QueryType &type) -> bool {
#ifdef DEBUG_QUERY_TRANSFER
  bool DEBUG_output_flag = false;
  int DEBUG_indent = 0;
  if (station_1 == "广东省惠阳市" && station_2 == "江苏省高邮市") {
    DEBUG_output_flag = true;
    std::cout << "Debug begin: query_transfer\n";
  }
#endif

  bool success = false;
  TransferResult res;
  auto record_vec_1 = station_storage_.find(station_1);
  auto record_vec_2 = station_storage_.find(station_2);

#ifdef DEBUG_QUERY_TRANSFER
  if (DEBUG_output_flag) {
    std::cout << "Trains pass station_1, aka. " << station_1 << " :\n";
    for (auto &rec : record_vec_1) {
      std::cout << rec.train_id_ << " " << rec.index << "\n";
    }
    std::cout << "Trains pass station_2, aka. " << station_2 << " :\n";
    for (auto &rec : record_vec_2) {
      std::cout << rec.train_id_ << " " << rec.index << "\n";
    }
  }
#endif

  linked_hashmap<std::string, int> rec_map;
  for (auto &rec : record_vec_2) {
    rec_map.insert({rec.train_id_, rec.index});
  }

#ifdef DEBUG_QUERY_TRANSFER
  if (DEBUG_output_flag) {
    std::cout << "For each train that passes station_1:\n";
    ++DEBUG_indent;
  }
#endif
  for (auto &rec_1 : record_vec_1) {
    auto train_1 = train_storage_.find(rec_1.train_id_)[0];
#ifdef DEBUG_QUERY_TRANSFER
    if (DEBUG_output_flag) {
      for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
      std::cout << "Current train:" << train_1.train_id_ << "\n";
    }
#endif
    if (!train_1.is_released_) {
#ifdef DEBUG_QUERY_TRANSFER
      if (DEBUG_output_flag) {
        for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
        std::cout << "Train " << train_1.train_id_ << "is not suitable. Because it is not released.\n";
      }
#endif
      continue;
    }
    int i1 = rec_1.index;  // station index
    int offset_1 = train_1.time_ranges_[i1].second.date.day_;
    auto depart_date_1 = date - offset_1;                     // train_1发车日期
    int j1 = depart_date_1 - train_1.sale_date_range_.first;  // date index
    if (train_1.sale_date_range_.first > depart_date_1 || train_1.sale_date_range_.second < depart_date_1) {
#ifdef DEBUG_QUERY_TRANSFER
      if (DEBUG_output_flag) {
        for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
        std::cout << "Train " << train_1.train_id_ << "is not suitable. Because date, aka. " << date
                  << " is not in sale_date_range, aka. " << train_1.sale_date_range_.first << " , "
                  << train_1.sale_date_range_.second << "\n";
      }
#endif
      continue;
    }
    int min_num_1 = train_1.seat_num_;
#ifdef DEBUG_QUERY_TRANSFER
    if (DEBUG_output_flag) {
      for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
      std::cout << "For each station that after station_1:\n";
      ++DEBUG_indent;
    }
#endif
    for (int i = i1 + 1; i < train_1.station_num_; ++i) {
      min_num_1 = std::min(min_num_1, train_1.left_seat_num_[i - 1][j1]);
      auto &station_3 = train_1.stations_[i];
#ifdef DEBUG_QUERY_TRANSFER
      if (DEBUG_output_flag) {
        for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
        std::cout << "Current station:" << station_3 << "\n";
      }
#endif
      auto record_vec_3 = station_storage_.find(station_3);
#ifdef DEBUG_QUERY_TRANSFER
      if (DEBUG_output_flag) {
        for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
        std::cout << "For each train that passed station_3:\n";
        ++DEBUG_indent;
      }
#endif
      for (auto &rec_3 : record_vec_3) {
        auto it = rec_map.find(rec_3.train_id_);
        if (it == rec_map.end() || rec_3.train_id_ == rec_1.train_id_) {
#ifdef DEBUG_QUERY_TRANSFER
          if (DEBUG_output_flag) {
            for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
            std::cout << "Train " << rec_3.train_id_ << "is not suitable. Because it cannot reach station_2\n";
          }
#endif
          continue;
        }
        auto train_2 = train_storage_.find(rec_3.train_id_)[0];
        if (!train_2.is_released_) {
#ifdef DEBUG_QUERY_TRANSFER
          if (DEBUG_output_flag) {
            for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
            std::cout << "Train " << train_2.train_id_ << "is not suitable. Because it is not released\n";
          }
#endif
          continue;
        }
        int i3 = rec_3.index;  // station index
        int offset_2 = train_2.time_ranges_[i3].second.date.day_;

        // TODO 检测train1到达早于train2发车
        auto depart_date_2 = std::max(depart_date_1 + train_1.time_ranges_[i].first.date.day_ - offset_2,
                                      train_2.sale_date_range_.first);

        int j2 = depart_date_2 - train_2.sale_date_range_.first;  // date index
        if (train_2.sale_date_range_.second < depart_date_2) {
#ifdef DEBUG_QUERY_TRANSFER
          if (DEBUG_output_flag) {
            for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
            std::cout << "Train " << train_2.train_id_ << "is not suitable. Because date, aka. " << date
                      << " is not in sale_date_range, aka. " << train_2.sale_date_range_.first << " , "
                      << train_2.sale_date_range_.second << "\n";
          }
#endif
          continue;
        }
        int i2 = it->second;
        if (i3 >= i2) {
#ifdef DEBUG_QUERY_TRANSFER
          if (DEBUG_output_flag) {
            for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
            std::cout << "Train " << train_2.train_id_ << "is not suitable. Because it goes the reversed direction\n";
          }
#endif
          continue;
        }
        int min_num_2 = train_2.seat_num_;
        for (int k = i3; k < i2; ++k) {
          min_num_2 = std::min(min_num_2, train_2.left_seat_num_[k][j2]);
        }
        auto depart_date_time_1 = DateTime{date, train_1.time_ranges_[i1].second.time};  // 从station_1出发的时间
        auto arrive_date_time_1 = DateTime{depart_date_1 + train_1.time_ranges_[i].first.date.day_,
                                           train_1.time_ranges_[i].first.time};  // 到达station_3的时间
        auto depart_date_time_2 = DateTime{depart_date_2 + train_2.time_ranges_[i3].second.date.day_,
                                           train_2.time_ranges_[i3].second.time};  // 从station_3出发的时间
        auto arrive_date_time_2 = DateTime{depart_date_2 + train_2.time_ranges_[i2].first.date.day_,
                                           train_2.time_ranges_[i2].first.time};  // 到达station_2的时间
        TransferResult res_1 = {{train_1.train_id_,
                                 {
                                     {station_1, depart_date_time_1},
                                     {station_3, arrive_date_time_1},
                                 },
                                 train_1.prices_[i] - train_1.prices_[i1],
                                 min_num_1},
                                {train_2.train_id_,
                                 {
                                     {station_3, depart_date_time_2},
                                     {station_2, arrive_date_time_2},
                                 },
                                 train_2.prices_[i2] - train_2.prices_[i3],
                                 min_num_2}};
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
#ifdef DEBUG_QUERY_TRANSFER
      if (DEBUG_output_flag) {
        for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
        std::cout << "End for\n";
        --DEBUG_indent;
      }
#endif
    }
#ifdef DEBUG_QUERY_TRANSFER
    if (DEBUG_output_flag) {
      for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
      std::cout << "End for\n";
      --DEBUG_indent;
    }
#endif
  }
#ifdef DEBUG_QUERY_TRANSFER
  if (DEBUG_output_flag) {
    for (int di = 0; di < DEBUG_indent; ++di) std::cout << " ";
    std::cout << "End for\n";
    --DEBUG_indent;
  }
#endif
#ifdef DEBUG_QUERY_TRANSFER
  if (DEBUG_output_flag) {
    std::cout << "Debug end\n";
  }
#endif
  if (success) {
    std::cout << res;
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
  auto train = train_storage_.find(train_id)[0];
  if (!train.is_released_) {
    return false;
  }

#ifdef DEBUG_BUY_TICKET
  bool DEBUG_output_flag = false;
  if (user_name == "Indra" && station_1 == "宁夏石嘴山市" && station_2 == "浙江省慈溪市") {
    DEBUG_output_flag = true;
    std::cout << "Debug begin buy_ticket\n";
  }
#endif
  int i1 = -1, i2 = -1, j;
  int min_num = train.seat_num_;
  Date depart_date;
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
      min_num = std::min(min_num, train.left_seat_num_[i][j]);
    }
  }
#ifdef DEBUG_BUY_TICKET
  if (DEBUG_output_flag) {
    std::cout << "Train condition:\n" << train.to_string(date);
  }
#endif
  if (i1 == -1 || i2 == -1) {
    return false;
  }
  if (min_num >= num) {
    train_storage_.remove(train_id, train);
    for (int i = i1; i < i2; ++i) {
      train.left_seat_num_[i][j] -= num;
    }
    train_storage_.insert(train_id, train);
    std::cout << (train.prices_[i2] - train.prices_[i1]) * num << "\n";
    trade_storage_.insert(user_name, Trade{time_stamp, Status::SUCCESS, train_id,
                                           DateTime{depart_date, {}} + train.time_ranges_[i1].second,
                                           DateTime{depart_date, {}} + train.time_ranges_[i2].first, station_1, i1,
                                           station_2, i2, train.prices_[i2] - train.prices_[i1], num});
    //    if (train_id == "LeavesofGrass" && depart_date == Date{6, 28}) {
    //      std::fstream fs;
    //      fs.open("./tracking", std::ios::out | std::ios::app);
    //      fs << "[" << time_stamp << "] -u " << user_name << " -i " << train_id << " -d " << date << " -n " << num <<
    //      " -f "
    //         << station_1 << " -t " << station_2 << " -q " << (wait ? "true" : "false") << "\n";
    //      fs << train.to_string(date);
    //      fs.close();
    //    }
    return true;
  } else {
    if (wait) {
      auto trade_vec = trade_storage_.find(user_name);
      q_sys_.push({user_name, train_id, i1, i2, j, num, (int)trade_vec.size()});
      trade_storage_.insert(user_name, Trade{time_stamp, Status::PENDING, train_id,
                                             DateTime{depart_date, {}} + train.time_ranges_[i1].second,
                                             DateTime{depart_date, {}} + train.time_ranges_[i2].first, station_1, i1,
                                             station_2, i2, train.prices_[i2] - train.prices_[i1], num});
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
  auto trade_vec = trade_storage_.find(user_name);
  if (n > trade_vec.size()) {
    return false;
  }
  auto &trade = trade_vec[n - 1];
  if (trade.status_ == Status::REFUNDED) {
    return false;
  }
  if (trade.status_ == Status::SUCCESS) {
    auto train = train_storage_.find(trade.train_id_)[0];
    // 还原座位数量
    auto depart_date = trade.leaving_time_.date - train.time_ranges_[trade.station_index_1_].second.date.day_;
    int j = depart_date - train.sale_date_range_.first;
    train_storage_.remove(trade.train_id_, train);
    for (int i = trade.station_index_1_; i < trade.station_index_2_; ++i) {
      train.left_seat_num_[i][j] += trade.num_;
    }
    train_storage_.insert(trade.train_id_, train);
    check_queue(trade.train_id_, trade.station_index_1_, trade.station_index_2_, j);
  }else{

  }
  trade_storage_.remove(user_name, trade);
  trade.status_ = Status::REFUNDED;
  trade_storage_.insert(user_name, trade);
  return true;
}
void TrainSystem::check_queue(const std::string &train_id, int station_index_1, int station_index_2, int date_index) {
  auto it = q_sys_.begin();
  while (it != q_sys_.end()) {
    auto &query = *it;
    if (query.train_id_ != train_id || query.date_index_ != date_index || query.station_index_2_ < station_index_1 ||
        query.station_index_1_ > station_index_2) {
      ++it;
      continue;
    }
    auto train = train_storage_.find(train_id)[0];
    int min_num = train.seat_num_;
    for (int i = query.station_index_1_; i < query.station_index_2_; ++i) {
      min_num = std::min(min_num, train.left_seat_num_[i][date_index]);
    }
    if (min_num < query.num_) {
      ++it;
      continue;
    }
    // 补票成功
    q_sys_.erase(it++);
    train_storage_.remove(train_id, train);
    for (int i = query.station_index_1_; i < query.station_index_2_; ++i) {
      train.left_seat_num_[i][date_index] -= query.num_;
    }
    train_storage_.insert(train_id, train);
    auto trade_vec = trade_storage_.find(query.user_name_);
    auto &trade = trade_vec[trade_vec.size() - query.trade_index_];
    trade_storage_.remove(query.user_name_, trade);
    trade.status_ = Status::SUCCESS;
    trade_storage_.insert(query.user_name_, trade);
  }
}
auto TrainSystem::query_order(const std::string &user_name) -> bool {
  if (!m_sys_->check_is_login(user_name)) {
    return false;
  }
  auto trade_vec = trade_storage_.find(user_name);
  std::cout << trade_vec.size() << "\n";
  for (auto &trade : trade_vec) {
    std::cout << trade;
  }
  return true;
}
void TrainSystem::clear() {
  train_storage_.clear();
  trade_storage_.clear();
  station_storage_.clear();
  q_sys_.reset();
}

}  // namespace CrazyDave