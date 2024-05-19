#include "train/queue_system.hpp"
namespace CrazyDave {
QueueSystem::QueueSystem() {
  if (queue_storage.get_is_new()) {
    return;
  }
  int size;
  queue_storage.seekg(0);
  queue_storage.read(size);
  for (int i = 0; i < size; ++i) {
    Query query;
    queue_storage.read(query);
    queue.push_back(query);
  }
}
QueueSystem::~QueueSystem() {
  queue_storage.seekp(0);
  queue_storage.write(queue.size());
  for (auto &query : queue) {
    queue_storage.write(query);
  }
}
void QueueSystem::reset() {
  queue.clear();
  queue_storage.clear();
}
void QueueSystem::push(const QueueSystem::Query &query) { queue.push_back(query); }
auto QueueSystem::begin() -> list<Query>::iterator { return queue.begin(); }
auto QueueSystem::end() -> list<Query>::iterator { return queue.end(); }
void QueueSystem::erase(const list<Query>::iterator &it) { queue.erase(it); }

}  // namespace CrazyDave
