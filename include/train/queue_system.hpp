#ifndef TICKETSYSTEM_QUEUE_SYSTEM_HPP
#define TICKETSYSTEM_QUEUE_SYSTEM_HPP

#include "common/utils.hpp"
#include "data_structures/list.h"
namespace CrazyDave {
class QueueSystem {
  struct Query {
    size_t user_hs_{};
    size_t train_hs_{};
    short station_index_1_{};
    short station_index_2_{};
    short date_index_{};
    int num_{};
    int trade_index_{};
    Query() = default;
    Query(size_t user_hs, size_t train_hs, short station_index_1, short station_index_2,
          short date_index, int num, int trade_index)
        : user_hs_(user_hs),
          train_hs_{train_hs},
          station_index_1_{station_index_1},
          station_index_2_{station_index_2},
          date_index_{date_index},
          num_{num},
          trade_index_{trade_index} {}
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
