#ifndef BPT_PRO_UTILS_H
#define BPT_PRO_UTILS_H
#include <cstring>
#include <string>
namespace CrazyDave {

template <class KeyFirst, class KeySecond, class ValueType>
class Comparator {
  using KeyType = pair<KeyFirst, KeySecond>;

 public:
  auto operator()(const KeyType &k1, const KeyType &k2) const -> int {
    if (k1 < k2) {
      return -1;
    }
    if (k2 < k1) {
      return 1;
    }
    return 0;
  }
  auto operator()(const KeyFirst &k1, const KeyFirst &k2) const -> int {
    if (k1 < k2) {
      return -1;
    }
    if (k2 < k1) {
      return 1;
    }
    return 0;
  }
  auto operator()(const ValueType &s1, const ValueType &s2) const -> int {
    if (s1 < s2) {
      return -1;
    }
    if (s2 < s1) {
      return 1;
    }
    return 0;
  }
};

}  // namespace CrazyDave
#endif  // BPT_PRO_UTILS_H
