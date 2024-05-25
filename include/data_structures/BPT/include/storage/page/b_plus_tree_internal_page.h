#pragma once
#include <string>
#include "storage/page/b_plus_tree_page.h"

namespace CrazyDave {

#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>
#define INTERNAL_PAGE_HEADER_SIZE 12
#define INTERNAL_PAGE_SIZE ((BUSTUB_PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(MappingType)) - 1)
/**
 * Store n indexed keys and n+1 child pointers (page_id) within internal page.
 * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
 * K(i) <= K < K(i+1).
 * NOTE: since the number of keys does not equal to number of child pointers,
 * the first key always remains invalid. That is to say, any search/lookup
 * should ignore the first key.
 *
 * Internal page format (keys are stored in increasing order):
 *  --------------------------------------------------------------------------
 * | HEADER | KEY(1)+PAGE_ID(1) | KEY(2)+PAGE_ID(2) | ... | KEY(n)+PAGE_ID(n) |
 *  --------------------------------------------------------------------------
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
class BPlusTreeInternalPage : public BPlusTreePage {
 public:
  // Deleted to disallow initialization
  BPlusTreeInternalPage() = delete;

  BPlusTreeInternalPage(const BPlusTreeInternalPage &other) = delete;

  /**
   * Writes the necessary header information to a newly created page, must be called after
   * the creation of a new page to make a valid BPlusTreeInternalPage
   * @param max_size Maximal size of the page
   */
  void Init(int max_size = INTERNAL_PAGE_SIZE - 1) {
    SetPageType(IndexPageType::INTERNAL_PAGE);
    SetSize(0);
    SetMaxSize(max_size);
  }

  /**
   * @param index The index of the key to get. Index must be non-zero.
   * @return Key at index
   */
  auto KeyAt(int index) const -> KeyType { return array_[index].first; }

  /**
   *
   * @param index The index of the key to set. Index must be non-zero.
   * @param key The new value for key
   */
  void SetKeyAt(int index, const KeyType &key) { array_[index].first = key; }

  /**
   *
   * @param value the value to search for
   */
  auto ValueIndex(const ValueType &value) const -> int {
    for (int i = 0; i < GetSize(); ++i) {
      if (array_[i].second == value) {
        return i;
      }
    }
    return -1;
  }

  /**
   *
   * @param index the index
   * @return the value at the index
   */
  auto ValueAt(int index) const -> ValueType { return array_[index].second; }

  void InsertAt(int index, const KeyType &key, const ValueType &value) {
    for (int i = GetSize(); i > index; --i) {
      array_[i] = array_[i - 1];
    }
    array_[index].first = key;
    array_[index].second = value;
    IncreaseSize(1);
  }

  void RemoveAt(int index) {
    for (int i = index; i < GetSize() - 1; ++i) {
      array_[i] = array_[i + 1];
    }
    IncreaseSize(-1);
  }

  auto PairAt(int index) const -> const MappingType & { return array_[index]; }

  void InsertAt(int index, const MappingType &pair) {
    for (int i = GetSize(); i > index; --i) {
      array_[i] = array_[i - 1];
    }
    array_[index] = pair;
    IncreaseSize(1);
  }

  auto LowerBoundByFirst(const KeyType &key, const KeyComparator &cmp) const -> int {
    int l = 1;
    int r = GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (cmp(array_[mid].first.first, key.first) == -1) {
        l = mid + 1;
      } else {
        r = mid;
      }
    }
    return l;
  }

  auto UpperBoundByFirst(const KeyType &key, const KeyComparator &cmp) const -> int {
    int l = 1;
    int r = GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (cmp(key.first, array_[mid].first.first) == -1) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    return l;
  }

 private:
  // Flexible array member for page data.
  MappingType array_[0];
};
}  // namespace CrazyDave
