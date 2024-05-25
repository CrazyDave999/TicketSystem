#ifndef BPT_PRO_CONCURRENT_LIST_H
#define BPT_PRO_CONCURRENT_LIST_H
#include <atomic>

/**
 * A bounded, lock-free, concurrent queue for garbage collection.
 * Based on ring buffer method and CAS, multiple producer and multiple consumer (aka. MPMC) is available.
 */

namespace CrazyDave {
template <class T, const int capacity>
class ConcurrentQueue {
 private:
  T buffer_[capacity];
  std::atomic<size_t> head_{0};
  std::atomic<size_t> tail_{0};

 public:
  ConcurrentQueue() = default;
  ~ConcurrentQueue() = default;

  auto push(const T &value) -> bool {
    size_t current_tail = tail_.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % capacity;
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false;
    }
    while (
        !tail_.compare_exchange_weak(current_tail, next_tail, std::memory_order_acq_rel, std::memory_order_relaxed)) {
      next_tail = (current_tail + 1) % capacity;
      if (next_tail == head_.load(std::memory_order_acquire)) {
        return false;
      }
    }
    buffer_[current_tail] = value;
    return true;
  }

  auto pop(T &value) -> bool {
    size_t current_head = head_.load(std::memory_order_relaxed);
    if (current_head == tail_.load(std::memory_order_acquire)) {
      return false;
    }
    size_t next_head = (current_head + 1) % capacity;
    while (
        !head_.compare_exchange_weak(current_head, next_head, std::memory_order_acq_rel, std::memory_order_relaxed)) {
      current_head = head_.load(std::memory_order_relaxed);
      if (current_head == tail_.load(std::memory_order_acquire)) {
        return false;
      }
      next_head = (current_head + 1) % capacity;
    }
    value = buffer_[current_head];
    return true;
  }

  /**
   * Helper functions for writing data back to disk.
   */
  auto get_head() -> int { return head_.load(); }

  auto get_tail() -> int { return tail_.load(); }

  auto get_value(int index) -> const T & { return buffer_[index]; };
};
}  // namespace CrazyDave
#endif  // BPT_PRO_CONCURRENT_LIST_H