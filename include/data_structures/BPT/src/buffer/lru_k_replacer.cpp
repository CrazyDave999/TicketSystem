#include "buffer/lru_k_replacer.h"

namespace CrazyDave {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {}

auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  // latch_.lock();
  size_t max_diff = 0;
  auto victim_it = node_store_.end();
  for (auto it = node_store_.begin(); it != node_store_.end(); ++it) {
    auto &node = it->second;
    if (!node.is_evictable_) {
      continue;
    }
    if (max_diff != inf_) {
      if (node.history_.size() < k_) {
        max_diff = inf_;
        victim_it = it;
      } else if (current_timestamp_ - node.history_.front() > max_diff) {
        max_diff = current_timestamp_ - node.history_.front();
        victim_it = it;
      }
    } else if (node.history_.size() < k_) {
      if (node.history_.front() < victim_it->second.history_.front()) {
        victim_it = it;
      }
    }
  }
  if (victim_it == node_store_.end()) {
    // latch_.unlock();
    return false;
  }
  *frame_id = victim_it->second.fid_;
  --curr_size_;
  node_store_.erase(victim_it);
  // latch_.unlock();
  return true;
}

void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  // latch_.lock();
  auto &node = node_store_[frame_id];
  if (node.history_.empty()) {
    node.fid_ = frame_id;
    node.k_ = k_;
  }
  node.history_.push_back(current_timestamp_);
  if (node.history_.size() > k_) {
    node.history_.pop_front();
  }
  ++current_timestamp_;
  // latch_.unlock();
}

void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  // latch_.lock();
  auto it = node_store_.find(frame_id);
  if (it->second.is_evictable_ ^ set_evictable) {
    if (set_evictable) {
      ++curr_size_;
    } else {
      --curr_size_;
    }
  }
  it->second.is_evictable_ = set_evictable;
  // latch_.unlock();
}

void LRUKReplacer::Remove(frame_id_t frame_id) {
  // latch_.lock();
  auto it = node_store_.find(frame_id);
  if (it == node_store_.end()) {
    // latch_.unlock();
    return;
  }
  node_store_.erase(it);
  --curr_size_;
  // latch_.unlock();
}

auto LRUKReplacer::Size() -> size_t {
  // latch_.lock();
  auto res = curr_size_;
  // latch_.unlock();
  return res;
}

}  // namespace CrazyDave
