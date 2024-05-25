/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "storage/page/b_plus_tree_leaf_page.h"

namespace CrazyDave {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

template <typename KeyType, typename ValueType, typename KeyComparator>
class IndexIterator {
 public:
  // you may define your own constructor based on your member variables
  IndexIterator(BufferPoolManager *buffer_pool_manager, page_id_t page_id, int pos = 0)
      : bpm_(buffer_pool_manager), page_id_(page_id), pos_(pos) {
    if (page_id == INVALID_PAGE_ID) {
      is_end_ = true;
    } else {
      guard_ = bpm_->FetchPageRead(page_id);
    }
  }
  ~IndexIterator() = default;  // NOLINT

  auto IsEnd() -> bool{ return is_end_; }

  auto operator*() -> const MappingType &{
    auto *page = guard_.As<B_PLUS_TREE_LEAF_PAGE_TYPE>();
    return page->PairAt(pos_);
  }

  auto operator++() -> IndexIterator &{
    if (is_end_) {
      return *this;
    }
    auto *page = guard_.As<B_PLUS_TREE_LEAF_PAGE_TYPE>();
    ++pos_;
    if (pos_ == page->GetSize()) {
      auto next_page_id = page->GetNextPageId();
      page_id_ = next_page_id;
      guard_.Drop();
      pos_ = 0;
      if (next_page_id == INVALID_PAGE_ID) {
        is_end_ = true;
      } else {
        guard_ = bpm_->FetchPageRead(next_page_id);
      }
    }
    return *this;
  }

  auto operator==(const IndexIterator &itr) const -> bool {
    if (bpm_ != itr.bpm_) {
      return false;
    }
    if (is_end_) {
      return itr.is_end_;
    }
    return page_id_ == itr.page_id_ && pos_ == itr.pos_;
  }

  auto operator!=(const IndexIterator &itr) const -> bool { return !(this->operator==(itr)); }

 private:
  // add your own private member variables here
  BufferPoolManager *bpm_;
  ReadPageGuard guard_;
  page_id_t page_id_;
  int pos_{0};
  bool is_end_{false};
};

}  // namespace CrazyDave
