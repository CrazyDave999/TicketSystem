#pragma once

#include <string>
#include "storage/page/b_plus_tree_page.h"

namespace CrazyDave {

#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>
#define LEAF_PAGE_HEADER_SIZE 16
#define LEAF_PAGE_SIZE ((BUSTUB_PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / sizeof(MappingType)-1)

/**
 * Store indexed key and record id (record id = page id combined with slot id,
 * see `include/common/rid.h` for detailed implementation) together within leaf
 * page. Only support unique key.
 *
 * Leaf page format (keys are stored in order):
 * -----------------------------------------------------------------------
 * | HEADER | KEY(1) + RID(1) | KEY(2) + RID(2) | ... | KEY(n) + RID(n)  |
 * -----------------------------------------------------------------------
 *
 * Header format (size in byte, 16 bytes in total):
 * -----------------------------------------------------------------------
 * | PageType (4) | CurrentSize (4) | MaxSize (4) | NextPageId (4) | ... |
 * -----------------------------------------------------------------------
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
class BPlusTreeLeafPage : public BPlusTreePage {
 public:
  // Delete all constructor / destructor to ensure memory safety
  BPlusTreeLeafPage() = delete;

  BPlusTreeLeafPage(const BPlusTreeLeafPage &other) = delete;

  /**
   * After creating a new leaf page from buffer pool, must call initialize
   * method to set default values
   * @param max_size Max size of the leaf node
   */
  void Init(int max_size = LEAF_PAGE_SIZE){
    SetPageType(IndexPageType::LEAF_PAGE);
    SetSize(0);
    SetNextPageId(INVALID_PAGE_ID);
    SetMaxSize(max_size);
  }

  // Helper methods
  auto GetNextPageId() const -> page_id_t{ return next_page_id_; }

  void SetNextPageId(page_id_t next_page_id){ next_page_id_ = next_page_id; }

  auto KeyAt(int index) const -> KeyType{ return array_[index].first; }

  void SetKeyAt(int index, const KeyType &key) { array_[index].first = key; }

  auto ValueAt(int index) const -> ValueType{ return array_[index].second; }

  void InsertAt(int index, const KeyType &key, const ValueType &value){
    for (int i = GetSize(); i > index; --i) {
      array_[i] = array_[i - 1];
    }
    array_[index].first = key;
    array_[index].second = value;
    IncreaseSize(1);
  }

  void RemoveAt(int index){
    for (int i = index; i < GetSize() - 1; ++i) {
      array_[i] = array_[i + 1];
    }
    IncreaseSize(-1);
  }

  auto PairAt(int index) const -> const MappingType &{ return array_[index]; }

  void InsertAt(int index, const MappingType &_pair) {
    for (int i = GetSize(); i > index; --i) {
      array_[i] = array_[i - 1];
    }
    array_[index] = _pair;
    IncreaseSize(1);
  }

  auto LowerBoundByFirst(const KeyType &key, const KeyComparator &cmp) const -> int{
    int l = 0;
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


  auto UpperBoundByFirst(const KeyType &key, const KeyComparator &cmp) const -> int{
    int l = 0;
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
  page_id_t next_page_id_;
  // Flexible array member for page data.
  MappingType array_[0];
};

}  // namespace CrazyDave
