#ifndef TICKETSYSTEM_QUEUE_SYSTEM_HPP
#define TICKETSYSTEM_QUEUE_SYSTEM_HPP
#include "common/utils.hpp"
#include "data_structures/list.h"
namespace CrazyDave {
class QueueSystem {
  struct Query {
    String<20> user_name_{};
    String<20> train_id_{};
    int station_index_1_{};
    int station_index_2_{};
    int date_index_{};
    int num_{};
    int trade_index_{};
    Query() = default;
    Query(const std::string &user_name, const std::string &train_id, int station_index_1, int station_index_2,
          int date_index, int num, int trade_index)
        : user_name_{user_name},
          train_id_{train_id},
          station_index_1_{station_index_1},
          station_index_2_{station_index_2},
          date_index_{date_index},
          num_{num},
          trade_index_{trade_index} {}
    friend std::ostream &operator<<(std::ostream &os, const Query &query) {
      os << query.user_name_ << ' ' << query.train_id_ << ' ' << query.station_index_1_ << ' '
         << query.station_index_2_ << ' ' << query.date_index_ << ' ' << query.num_ << ' ' << query.trade_index_ <<"\n";
      return os;
    }
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
