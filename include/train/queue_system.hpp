#ifndef TICKETSYSTEM_QUEUE_SYSTEM_HPP
#define TICKETSYSTEM_QUEUE_SYSTEM_HPP
#include "common/utils.hpp"
#include "data_structures/list.h"
#include "train.hpp"
namespace CrazyDave {
class TrainSystem;
class QueueSystem {
  friend TrainSystem;
  struct Query {
    String<21> user_name_{};
    String<21> train_id_{};
    int station_index_1_;
    int station_index_2_;
    int date_index_;
    int num_{};
    int trade_index_{};
  };

 private:
  File queue_storage{"tmp/queue"};
  list<Query> queue{};

 public:
  QueueSystem();
  ~QueueSystem();

  void reset();
};
}  // namespace CrazyDave
#endif  // TICKETSYSTEM_QUEUE_SYSTEM_HPP
