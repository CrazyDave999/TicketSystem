#pragma once
#include <iostream>
#include <optional>
#include <string>

#include "common/config.h"
#include "common/utils.h"
#include "data_structures/list.h"
#include "data_structures/vector.h"
#include "storage/index/index_iterator.h"
#include "storage/page/b_plus_tree_header_page.h"
#include "storage/page/b_plus_tree_internal_page.h"
#include "storage/page/b_plus_tree_leaf_page.h"
#include "storage/page/page_guard.h"

namespace CrazyDave {

/**
 * @brief Definition of the Context class.
 *
 * Hint: This class is designed to help you keep track of the pages
 * that you're modifying or accessing.
 */
class Context {
 public:
  // When you insert into / remove from the B+ tree, store the write guard of header page here.
  // Remember to drop the header page guard and set it to nullopt when you want to unlock all.
  std::optional<WritePageGuard> header_write_guard_{std::nullopt};

  std::optional<ReadPageGuard> header_read_guard_{std::nullopt};

  // Save the root page id here so that it's easier to know if the current page is the root page.
  page_id_t root_page_id_{INVALID_PAGE_ID};

  // Store the write guards of the pages that you're modifying here.
  list<WritePageGuard> write_set_;

  // You may want to use this when getting value, but not necessary.
  list<ReadPageGuard> read_set_;

  // Record the index of key in the path.
  list<int> index_set_;

  [[nodiscard]] auto IsRootPage(page_id_t page_id) const -> bool { return page_id == root_page_id_; }
};

#define BPLUSTREE_TYPE BPlusTree<KeyType, ValueType, KeyComparator>

// Main class providing the API for the Interactive B+ Tree.
template <typename KeyFirst, typename KeySecond, typename ValueType, typename KeyComparator>
class BPlusTree {
  using KeyType = pair<KeyFirst, KeySecond>;
  using InternalPage = BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator>;
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;
  enum class Protocol { Optimistic, Pessimistic };

 public:
  explicit BPlusTree(std::string name, page_id_t header_page_id, size_t pool_size, size_t replacer_k,
                     int leaf_max_size = LEAF_PAGE_SIZE, int internal_max_size = INTERNAL_PAGE_SIZE)
      : index_name_(std::move(name)),
        leaf_max_size_(leaf_max_size),
        internal_max_size_(internal_max_size),
        header_page_id_(header_page_id) {
    //  std::cout << "Hello from asshole debugger CrazyDave.\nConstructing BPlusTree.\nleaf_max_size: " <<
    //  leaf_max_size_
    //            << ", internal_max_size: " << internal_max_size_ << "\n";  // debug
    bpm_ = new BufferPoolManager{index_name_, pool_size, replacer_k};
    if (bpm_->IsNew()) {
      WritePageGuard guard = bpm_->FetchPageWrite(header_page_id_);
      auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
      root_page->root_page_id_ = INVALID_PAGE_ID;
    }
  }
  ~BPlusTree() { delete bpm_; }

  // Returns true if this B+ tree has no keys and values.
  [[nodiscard]] auto IsEmpty() const -> bool {
    auto guard = bpm_->FetchPageRead(header_page_id_);
    auto root_page = guard.As<BPlusTreeHeaderPage>();
    return root_page->root_page_id_ == INVALID_PAGE_ID;
  }

  void insert(const KeyFirst &key, const KeySecond &value) { insert({key, value}, {}); }

  void remove(const KeyFirst &key, const KeySecond &value) { remove({key, value}); }

  // Return the value associated with a given key
  void find(const KeyFirst &key, vector<KeySecond> &result) {
    auto header_page_guard = bpm_->FetchPageRead(header_page_id_);
    auto header_page = header_page_guard.As<BPlusTreeHeaderPage>();
    if (header_page->root_page_id_ == INVALID_PAGE_ID) {
      header_page_guard.Drop();
      return;
    }
    auto guard = bpm_->FetchPageRead(header_page->root_page_id_);
    header_page_guard.Drop();
    find({key, {}}, result, guard);
  }

  // Return the page id of the root node
  auto GetRootPageId() -> page_id_t {
    auto guard = bpm_->FetchPageRead(header_page_id_);
    auto header_page = guard.As<BPlusTreeHeaderPage>();
    return header_page->root_page_id_;
  }

  // Index iterator
  auto Begin() -> INDEXITERATOR_TYPE {
    auto header_page = bpm_->FetchPageRead(header_page_id_).As<BPlusTreeHeaderPage>();
    if (header_page->root_page_id_ == INVALID_PAGE_ID) {
      return End();
    }

    auto guard = bpm_->FetchPageRead(header_page->root_page_id_);
    auto bpt_page = guard.As<BPlusTreePage>();
    page_id_t page_id = header_page->root_page_id_;
    while (!bpt_page->IsLeafPage()) {
      auto internal_page = reinterpret_cast<const InternalPage *>(bpt_page);
      page_id = internal_page->ValueAt(0);
      guard = bpm_->FetchPageRead(page_id);
      bpt_page = guard.As<BPlusTreePage>();
    }
    return {bpm_, page_id};
  }

  auto End() -> INDEXITERATOR_TYPE { return {bpm_, INVALID_PAGE_ID}; }

  auto Begin(const KeyType &key) -> INDEXITERATOR_TYPE {
    auto header_page = bpm_->FetchPageRead(header_page_id_).As<BPlusTreeHeaderPage>();
    if (header_page->root_page_id_ == INVALID_PAGE_ID) {
      return End();
    }

    auto guard = bpm_->FetchPageRead(header_page->root_page_id_);
    auto bpt_page = guard.As<BPlusTreePage>();
    page_id_t page_id = header_page->root_page_id_;
    while (!bpt_page->IsLeafPage()) {
      auto internal_page = reinterpret_cast<const InternalPage *>(bpt_page);
      auto l = UpperBound(internal_page, key) - 1;
      page_id = internal_page->ValueAt(l);
      guard = bpm_->FetchPageRead(page_id);
      bpt_page = guard.As<BPlusTreePage>();
    }
    auto leaf_page = reinterpret_cast<const LeafPage *>(bpt_page);
    auto l = BinarySearch(leaf_page, key);
    if (l != -1) {
      return {bpm_, page_id, l};
    }
    return End();
  }

 private:
  auto LowerBound(const LeafPage *page, const KeyType &key) const -> int {
    int l = 0;
    int r = page->GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (comparator_(page->KeyAt(mid), key) == -1) {
        l = mid + 1;
      } else {
        r = mid;
      }
    }
    return l;
  }

  auto LowerBound(const InternalPage *page, const KeyType &key) const -> int {
    int l = 0;
    int r = page->GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (comparator_(page->KeyAt(mid), key) == -1) {
        l = mid + 1;
      } else {
        r = mid;
      }
    }
    return l;
  }

  // binary search，找不到返回-1
  auto BinarySearch(const LeafPage *page, const KeyType &key) const -> int {
    auto l = LowerBound(page, key);
    if (l == page->GetSize() || comparator_(key, page->KeyAt(l)) != 0) {
      return -1;
    }
    return l;
  }

  auto UpperBound(const LeafPage *page, const KeyType &key) const -> int {
    int l = 0;
    int r = page->GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (comparator_(key, page->KeyAt(mid)) == -1) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    return l;
  }
  // upper bound. 返回第一个大于key的index
  auto UpperBound(const InternalPage *page, const KeyType &key) const -> int {
    int l = 1;
    int r = page->GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (comparator_(key, page->KeyAt(mid)) == -1) {
        r = mid;
      } else {
        l = mid + 1;
      }
    }
    return l;
  }

  auto InsertKeyValue(LeafPage *page, const KeyType &key, const ValueType &value) const -> bool {
    int l = 0;
    int r = page->GetSize();
    while (l < r) {
      int mid = (l + r) >> 1;
      if (comparator_(page->KeyAt(mid), key) == -1) {
        l = mid + 1;
      } else {
        r = mid;
      }
    }
    if (l == page->GetSize() || comparator_(key, page->KeyAt(l)) != 0) {
      page->InsertAt(l, key, value);
      return true;
    }
    return false;
  }

  auto SplitLeafPage(LeafPage *page, page_id_t *n_page_id, Context &ctx) -> LeafPage * {
    auto n_page_guard = bpm_->NewPageGuarded(n_page_id);
    auto *n_page = n_page_guard.AsMut<LeafPage>();

    n_page->Init(leaf_max_size_);
    auto size = page->GetSize();
    for (int i = size >> 1; i < size; ++i) {
      n_page->InsertAt(n_page->GetSize(), page->PairAt(i));
    }
    page->SetSize(size >> 1);
    n_page->SetNextPageId(page->GetNextPageId());
    page->SetNextPageId(*n_page_id);
    if (ctx.IsRootPage(ctx.write_set_.back().PageId())) {  // 根是叶子，新根
      page_id_t n_root_page_id;
      auto n_root_guard = bpm_->NewPageGuarded(&n_root_page_id);
      auto *n_root_page = n_root_guard.AsMut<InternalPage>();
      n_root_page->Init(internal_max_size_);
      if (ctx.header_write_guard_.has_value()) {
        ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = n_root_page_id;
      }
      n_root_page->InsertAt(0, KeyType(), ctx.root_page_id_);
      InsertKeyValue(n_root_page, n_page->KeyAt(0), *n_page_id);
      ctx.write_set_.pop_back();
      return n_page;
    }
    ctx.write_set_.pop_back();
    auto *p_page = ctx.write_set_.back().template AsMut<InternalPage>();

    InsertKeyValue(p_page, n_page->KeyAt(0), *n_page_id);

    return n_page;
  }

  void InsertKeyValue(InternalPage *page, const KeyType &key, const page_id_t &value) {
    auto l = UpperBound(page, key);
    page->InsertAt(l, key, value);
  }

  auto SplitInternalPage(InternalPage *page, page_id_t *n_page_id, Context &ctx) -> InternalPage * {
    auto n_page_guard = bpm_->NewPageGuarded(n_page_id);
    auto *n_page = n_page_guard.AsMut<InternalPage>();
    n_page->Init(internal_max_size_);
    auto size = page->GetSize();
    for (int i = size >> 1; i < size; ++i) {
      n_page->InsertAt(n_page->GetSize(), page->PairAt(i));
    }
    page->SetSize(size >> 1);
    if (ctx.IsRootPage(ctx.write_set_.back().PageId())) {  // 新根
      page_id_t n_root_page_id;
      auto n_root_guard = bpm_->NewPageGuarded(&n_root_page_id);
      auto *n_root_page = n_root_guard.AsMut<InternalPage>();
      n_root_page->Init(internal_max_size_);
      if (ctx.header_write_guard_.has_value()) {
        ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = n_root_page_id;
      }
      n_root_page->InsertAt(0, KeyType(), ctx.root_page_id_);
      InsertKeyValue(n_root_page, n_page->KeyAt(0), *n_page_id);
      ctx.write_set_.pop_back();
      return n_page;
    }
    ctx.write_set_.pop_back();
    auto *p_page = ctx.write_set_.back().template AsMut<InternalPage>();
    InsertKeyValue(p_page, n_page->KeyAt(0), *n_page_id);
    return n_page;
  }

  void RemoveKeyValue(LeafPage *page, const KeyType &key) {
    int l = BinarySearch(page, key);
    if (l != -1) {
      page->RemoveAt(l);
    }
  }

  auto TryAdoptFromNeighbor(LeafPage *page, Context &ctx) -> bool {
    //  std::cout << "Trying to adopt a child from neighbor. Type: leaf_page\n Before: " << page->ToString()
    //            << "\n";  // debug
    //  auto *p_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
    auto it = ctx.write_set_.end();
    --it, --it;
    auto *p_page = it->AsMut<InternalPage>();
    int l = ctx.index_set_.back();
    if (l < p_page->GetSize() - 1) {
      auto r_page_id = p_page->ValueAt(l + 1);
      auto r_page_guard = bpm_->FetchPageWrite(r_page_id);
      auto *r_page = r_page_guard.template AsMut<LeafPage>();
      if (r_page->GetSize() > r_page->GetMinSize()) {
        page->InsertAt(page->GetSize(), r_page->PairAt(0));
        r_page->RemoveAt(0);
        p_page->SetKeyAt(l + 1, r_page->KeyAt(0));
        //      std::cout << "Successfully adopted " << key << ", " << value
        //                << "from right neighbor.\n After: " << page->ToString() << "\n";  // debug
        ctx.write_set_.pop_back();
        ctx.index_set_.pop_back();
        return true;
      }
    }
    if (l > 0) {
      auto l_page_id = p_page->ValueAt(l - 1);
      auto l_page_guard = bpm_->FetchPageWrite(l_page_id);
      auto *l_page = l_page_guard.template AsMut<LeafPage>();
      if (l_page->GetSize() > l_page->GetMinSize()) {
        page->InsertAt(0, l_page->PairAt(l_page->GetSize() - 1));
        l_page->RemoveAt(l_page->GetSize() - 1);
        p_page->SetKeyAt(l, page->KeyAt(0));
        //      std::cout << "Successfully adopted " << key << ", " << value
        //                << "from left neighbor.\n After: " << page->ToString() << "\n";  // debug
        ctx.write_set_.pop_back();
        ctx.index_set_.pop_back();
        return true;
      }
    }
    //  std::cout << "Adoption failed. Consider merging leaf page.\n";  // debug
    return false;
  }

  void MergeLeafPage(LeafPage *page, Context &ctx) {
    // 必须先 TryAdoptFromNeighbor，再考虑 MergeLeafPage。领养失败则必定能合并
    //  std::cout << "Merging a page. Type: leaf_page.\n Before: " << page->ToString() << "\n";  // debug
    //  auto *p_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
    auto it = ctx.write_set_.end();
    --it, --it;
    auto *p_page = it->AsMut<InternalPage>();
    int l = ctx.index_set_.back();
    if (l < p_page->GetSize() - 1) {
      auto r_page_id = p_page->ValueAt(l + 1);
      auto r_page_guard = bpm_->FetchPageWrite(r_page_id);
      auto *r_page = r_page_guard.template AsMut<LeafPage>();
      //    std::cout << "Merging r_page: " << r_page->ToString() << " to page: " << page->ToString() << "\n";  // debug
      for (int i = 0; i < r_page->GetSize(); ++i) {
        page->InsertAt(page->GetSize(), r_page->PairAt(i));
      }
      r_page->SetSize(0);
      page->SetNextPageId(r_page->GetNextPageId());
      p_page->RemoveAt(l + 1);
      bpm_->DeletePage(r_page_id);
      ctx.write_set_.pop_back();
      ctx.index_set_.pop_back();
      //    std::cout << "Successfully merged. After merging, page: " << page->ToString() << "\n";  // debug
      return;
    }
    auto l_page_id = p_page->ValueAt(l - 1);
    auto l_page_guard = bpm_->FetchPageWrite(l_page_id);
    auto *l_page = l_page_guard.template AsMut<LeafPage>();
    //  std::cout << "Merging page: " << page->ToString() << " to l_page: " << l_page->ToString() << "\n";  // debug
    for (int i = 0; i < page->GetSize(); ++i) {
      l_page->InsertAt(l_page->GetSize(), page->PairAt(i));
    }
    page->SetSize(0);
    l_page->SetNextPageId(page->GetNextPageId());
    p_page->RemoveAt(l);
    bpm_->DeletePage(p_page->ValueAt(l));
    ctx.write_set_.pop_back();
    ctx.index_set_.pop_back();
    //  std::cout << "Successfully merged. After merging, l_page: " << l_page->ToString() << "\n";  // debug
  }

  auto TryAdoptFromNeighbor(InternalPage *page, Context &ctx) -> bool {
    //  std::cout << "Trying to adopt a child from neighbor. Type: leaf_page\n Before: " << page->ToString()
    //            << "\n";  // debug
    //  auto *p_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
    auto it = ctx.write_set_.end();
    --it, --it;
    auto *p_page = it->AsMut<InternalPage>();
    int l = ctx.index_set_.back();
    if (l < p_page->GetSize() - 1) {
      auto r_page_id = p_page->ValueAt(l + 1);
      auto r_page_guard = bpm_->FetchPageWrite(r_page_id);
      auto *r_page = r_page_guard.template AsMut<InternalPage>();
      if (r_page->GetSize() > r_page->GetMinSize()) {
        page->InsertAt(page->GetSize(), r_page->PairAt(0));
        r_page->RemoveAt(0);
        p_page->SetKeyAt(l + 1, r_page->KeyAt(0));
        //      std::cout << "Successfully adopted " << key << ", " << value
        //                << "from right neighbor.\n After: " << page->ToString() << "\n";  // debug
        ctx.write_set_.pop_back();
        ctx.index_set_.pop_back();
        return true;
      }
    }
    if (l > 0) {
      auto l_page_id = p_page->ValueAt(l - 1);
      auto l_page_guard = bpm_->FetchPageWrite(l_page_id);
      auto *l_page = l_page_guard.template AsMut<InternalPage>();
      if (l_page->GetSize() > l_page->GetMinSize()) {
        page->InsertAt(0, l_page->PairAt(l_page->GetSize() - 1));
        l_page->RemoveAt(l_page->GetSize() - 1);
        p_page->SetKeyAt(l, page->KeyAt(0));
        //      std::cout << "Successfully adopted " << key << ", " << value
        //                << "from left neighbor.\n After: " << page->ToString() << "\n";  // debug
        ctx.write_set_.pop_back();
        ctx.index_set_.pop_back();
        return true;
      }
    }
    //  std::cout << "Adoption failed. Consider merging leaf page.\n";  // debug
    return false;
  }

  void MergeInternalPage(InternalPage *page, Context &ctx) {
    // 必须先 TryAdoptFromNeighbor，再考虑 MergeLeafPage。领养失败则必定能合并
    //  std::cout << "Merging a page. Type: leaf_page.\n Before: " << page->ToString() << "\n";  // debug
    //  auto *p_page = ctx.write_set_[ctx.write_set_.size() - 2].AsMut<InternalPage>();
    auto it = ctx.write_set_.end();
    --it, --it;
    auto *p_page = it->AsMut<InternalPage>();
    int l = ctx.index_set_.back();
    if (l < p_page->GetSize() - 1) {
      auto r_page_id = p_page->ValueAt(l + 1);
      auto r_page_guard = bpm_->FetchPageWrite(r_page_id);
      auto *r_page = r_page_guard.template AsMut<InternalPage>();
      //    std::cout << "Merging r_page: " << r_page->ToString() << " to page: " << page->ToString() << "\n";  // debug
      for (int i = 0; i < r_page->GetSize(); ++i) {
        page->InsertAt(page->GetSize(), r_page->PairAt(i));
      }
      r_page->SetSize(0);
      p_page->RemoveAt(l + 1);
      bpm_->DeletePage(r_page_id);
      ctx.write_set_.pop_back();
      ctx.index_set_.pop_back();
      //    std::cout << "Successfully merged. After merging, page: " << page->ToString() << "\n";  // debug
      return;
    }
    auto l_page_id = p_page->ValueAt(l - 1);
    auto l_page_guard = bpm_->FetchPageWrite(l_page_id);
    auto *l_page = l_page_guard.template AsMut<InternalPage>();
    //  std::cout << "Merging page: " << page->ToString() << " to l_page: " << l_page->ToString() << "\n";  // debug
    for (int i = 0; i < page->GetSize(); ++i) {
      l_page->InsertAt(l_page->GetSize(), page->PairAt(i));
    }
    page->SetSize(0);
    p_page->RemoveAt(l);
    bpm_->DeletePage(p_page->ValueAt(l));
    ctx.write_set_.pop_back();
    ctx.index_set_.pop_back();
    //  std::cout << "Successfully merged. After merging, l_page: " << l_page->ToString() << "\n";  // debug
  }

  /**
   * @return whether insert successfully and if false, whether it is because leaf node unsafe.
   */
  auto insert(const KeyType &key, const ValueType &value) -> pair<bool, bool> {
    Context ctx;
    ctx.header_write_guard_ = bpm_->FetchPageWrite(header_page_id_);
    ctx.root_page_id_ = ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_;
    if (ctx.root_page_id_ == INVALID_PAGE_ID) {
      page_id_t n_root_page_id;
      auto n_root_guard = bpm_->NewPageGuarded(&n_root_page_id);
      auto *n_root_page = n_root_guard.AsMut<LeafPage>();
      n_root_page->Init(leaf_max_size_);
      auto *header_page = ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>();
      header_page->root_page_id_ = n_root_page_id;
      n_root_page->InsertAt(0, key, value);
      return {true, true};
    }

    ctx.write_set_.push_back(bpm_->FetchPageWrite(ctx.root_page_id_));
    auto bpt_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
    while (!bpt_page->IsLeafPage()) {
      if (bpt_page->GetSize() < bpt_page->GetMaxSize()) {  // safe
        while (ctx.write_set_.size() > 1) {
          ctx.write_set_.pop_front();
        }
      }
      auto *internal_page = reinterpret_cast<InternalPage *>(bpt_page);

      auto l = UpperBound(internal_page, key) - 1;
      ctx.write_set_.push_back(bpm_->FetchPageWrite(internal_page->ValueAt(l)));
      bpt_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
    }
    auto *leaf_page = reinterpret_cast<LeafPage *>(bpt_page);

    if (InsertKeyValue(leaf_page, key, value)) {
      if (leaf_page->GetSize() == leaf_page->GetMaxSize()) {
        page_id_t n_page_id;
        SplitLeafPage(leaf_page, &n_page_id, ctx);
        InternalPage *internal_page;
        while (ctx.write_set_.size() > 1) {
          internal_page = ctx.write_set_.back().AsMut<InternalPage>();
          SplitInternalPage(internal_page, &n_page_id, ctx);
        }
        // 三种可能
        // 1. 根是leaf，在Split操作中已经完成根的更新，ctx.write_set_为空
        // 2. 在根以下没有遇到过safe node，ctx.write_set_中有一个元素，是根的写锁，根有可能需要分裂
        // 3. 在根以下遇到过safe node，ctx.write_set_中有一个元素，是safe node
        if (!ctx.write_set_.empty()) {
          internal_page = ctx.write_set_.back().template AsMut<InternalPage>();
          if (internal_page->GetSize() > internal_page->GetMaxSize()) {
            SplitInternalPage(internal_page, &n_page_id, ctx);
          }
        }
      }
      return {true, false};
    }
    return {false, false};
  }

  /**
   * @return whether insert successfully and if false, whether it is because leaf node unsafe.
   */
  auto remove(const KeyType &key) -> pair<bool, bool> {
    Context ctx;
    // 用栈模拟递归
    ctx.header_write_guard_ = bpm_->FetchPageWrite(header_page_id_);
    ctx.root_page_id_ = ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_;
    if (ctx.root_page_id_ == INVALID_PAGE_ID) {  // 空树
      return {true, false};
    }

    ctx.write_set_.push_back(bpm_->FetchPageWrite(ctx.root_page_id_));
    auto bpt_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
    while (!bpt_page->IsLeafPage()) {
      if (bpt_page->GetSize() > bpt_page->GetMinSize()) {  // safe
        while (ctx.write_set_.size() > 1) {
          ctx.write_set_.pop_front();
          ctx.index_set_.pop_front();
        }
      }
      auto *internal_page = reinterpret_cast<InternalPage *>(bpt_page);
      auto l = UpperBound(internal_page, key) - 1;
      ctx.write_set_.push_back(bpm_->FetchPageWrite(internal_page->ValueAt(l)));
      ctx.index_set_.push_back(l);
      bpt_page = ctx.write_set_.back().AsMut<BPlusTreePage>();
    }
    auto leaf_page = reinterpret_cast<LeafPage *>(bpt_page);
    RemoveKeyValue(leaf_page, key);

    if (leaf_page->GetSize() >= leaf_page->GetMinSize()) {
      return {true, false};
    }
    if (ctx.IsRootPage(ctx.write_set_.back().PageId())) {  // 根就是叶子
      if (leaf_page->GetSize() == 0) {
        ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = INVALID_PAGE_ID;
        bpm_->DeletePage(ctx.root_page_id_);
      }
      return {true, false};
    }
    if (TryAdoptFromNeighbor(leaf_page, ctx)) {
      return {true, false};
    }
    MergeLeafPage(leaf_page, ctx);
    auto *page = ctx.write_set_.back().AsMut<InternalPage>();
    while (ctx.write_set_.size() > 1) {
      if (TryAdoptFromNeighbor(page, ctx)) {
        return {true, false};
      }
      MergeInternalPage(page, ctx);
      page = ctx.write_set_.back().AsMut<InternalPage>();
    }
    // 两种可能：
    // 1. ctx.write_set_中仅剩根的写锁，这时有可能根仅剩一个儿子，需要换根
    // 2. ctx.write_set_中仅剩安全节点的写锁，什么都不用做
    if (page->GetSize() == 1) {
      ctx.header_write_guard_->AsMut<BPlusTreeHeaderPage>()->root_page_id_ = page->ValueAt(0);
      bpm_->DeletePage(ctx.root_page_id_);
    }
    return {true, false};
  }

  void find(const KeyType &key, vector<KeySecond> &result, ReadPageGuard &guard) {
    auto *page = guard.template As<BPlusTreePage>();
    if (page->IsLeafPage()) {
      auto leaf_page = reinterpret_cast<const LeafPage *>(page);
      int l = leaf_page->LowerBoundByFirst(key, comparator_);
      int r = leaf_page->UpperBoundByFirst(key, comparator_);
      for (int i = l; i < r; ++i) {
        result.push_back(leaf_page->KeyAt(i).second);
      }
      guard.Drop();
      return;
    }
    auto internal_page = reinterpret_cast<const InternalPage *>(page);
    int l = internal_page->LowerBoundByFirst(key, comparator_) - 1;
    int r = internal_page->UpperBoundByFirst(key, comparator_) - 1;

    page_id_t son_page_id[INTERNAL_PAGE_SIZE + 2];
    for (int i = l; i <= r; ++i) {
      son_page_id[i - l] = internal_page->ValueAt(i);
    }
    guard.Drop();
    for (int i = l; i <= r; ++i) {
      auto n_guard = bpm_->FetchPageRead(son_page_id[i - l]);
      find(key, result, n_guard);
    }
  }

  // member variable
  std::string index_name_;
  BufferPoolManager *bpm_;
  KeyComparator comparator_;
  int leaf_max_size_;
  int internal_max_size_;
  page_id_t header_page_id_;
};

template <class KeyType, class ValueType>
using BPT = BPlusTree<KeyType, ValueType, char, Comparator<KeyType, ValueType, char>>;

}  // namespace CrazyDave
