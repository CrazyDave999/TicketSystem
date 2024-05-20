#ifndef TICKETSYSTEM_QUEUE_SYSTEM_HPP
#define TICKETSYSTEM_QUEUE_SYSTEM_HPP
#include "common/utils.hpp"
#include "data_structures/list.h"
namespace CrazyDave {
class QueueSystem {
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
#ifdef DEBUG_FILE_IN_TMP
  File queue_storage{"tmp/qu"};
#else
  File queue_storage{"qu"};
#endif
  list<Query> queue{};

 public:
  QueueSystem();
  ~QueueSystem();

  void push(const Query &query);
  auto begin() -> list<Query>::iterator;
  auto end() -> list<Query>::iterator;
  void erase(const list<Query>::iterator &it);
  void reset();
};
}  // namespace CrazyDave
#endif  // TICKETSYSTEM_QUEUE_SYSTEM_HPP
