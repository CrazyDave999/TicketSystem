#include "train/train.hpp"

#include <utility>

CrazyDave::Train::Train(const std::string &train_id, int seat_num, const CrazyDave::vector<std::string> &stations,
                        const CrazyDave::vector<int> &prices, const CrazyDave::Time &start_time,
                        const CrazyDave::vector<int> &travel_times, const CrazyDave::vector<int> &stop_over_times,
                        CrazyDave::DateRange sale_date, const char type)
    : train_id_(train_id),
      station_num_((int)stations.size()),
      seat_num_(seat_num),
      sale_date_(std::move(sale_date)),
      type_(type) {
  Time arrival_time;
  Time leaving_time = start_time;
  for (int i = 0; i < station_num_; ++i) {
    stations_[i] = stations[i];
    prices_[i] = prices[i];
    if (i == 0) {
      arrival_time = Time{-1, -1};
    }
    if (i == station_num_ - 1) {
      leaving_time = Time{-1, -1};
    }
    time_range_[i] = TimeRange{arrival_time, leaving_time};
    if (i < station_num_ - 1) {
      arrival_time = leaving_time + travel_times[i];
    }
    if (i < station_num_ - 2) {
      leaving_time = arrival_time + stop_over_times[i];
    }
  }

}
auto CrazyDave::TrainSystem::add_train(const std::string &train_id, int seat_num,
                                       const CrazyDave::vector<std::string> &stations,
                                       const CrazyDave::vector<int> &prices, const CrazyDave::Time &start_time,
                                       const CrazyDave::vector<int> &travel_times,
                                       const CrazyDave::vector<int> &stop_over_times,
                                       const CrazyDave::DateRange &sale_date, const char type) -> bool {
  Train train;
}
