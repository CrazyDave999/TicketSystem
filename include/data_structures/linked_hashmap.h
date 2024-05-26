#ifndef BPT_PRO_LINKED_HASHMAP_HPP
#define BPT_PRO_LINKED_HASHMAP_HPP

#include "../common/utils.hpp"
namespace CrazyDave {
template <class Key, class T, class Hash = std::hash<Key>, class Equal = std::equal_to<Key> >
class linked_hashmap {
 protected:
  long long currentSize;
  int capacity;
  Hash hash;
  Equal equal;

  // 双链表节点
  struct node {
    typedef pair<const Key, T> value_type;
    value_type *val;
    node *next;
    node *prev;

    node *_next;

    explicit node(value_type v) {
      val = new value_type(v);
      next = nullptr;
      prev = nullptr;
      _next = nullptr;
    }

    node(value_type v, node *_n) {
      val = new value_type(v);
      _next = _n;
      next = nullptr;
      prev = nullptr;
    }

    node() {
      val = nullptr;
      next = nullptr;
      prev = nullptr;
      _next = nullptr;
    };

    ~node() { delete val; }
  };

  node *head, *tail;
  node **array;  // 开散列表
  size_t get_hash(const Key &key) const { return hash(key) % capacity; }

 public:
  typedef pair<const Key, T> value_type;
  class const_iterator;
  class iterator {
    friend linked_hashmap<Key, T>;
    friend const_iterator;

   private:
    node *ptr;

   public:
    node *get_ptr() { return ptr; }
    using difference_type = std::ptrdiff_t;
    using value_type = typename linked_hashmap::value_type;
    using pointer = value_type *;
    using reference = value_type &;
    using iterator_category = std::output_iterator_tag;

    explicit iterator(node *ptr = nullptr) : ptr(ptr) {}

    iterator(const iterator &other) : ptr(other.ptr) {}

    iterator &operator=(const iterator &other) {
      if (this == &other) return *this;
      ptr = other.ptr;
      return *this;
    }

    iterator &operator=(const const_iterator &other) { ptr = other.ptr; }

    iterator operator++(int) {
      iterator tmp = *this;
      ptr = ptr->next;
      return tmp;
    }

    iterator &operator++() {
      ptr = ptr->next;
      return *this;
    }

    iterator operator--(int) {
      iterator tmp = *this;
      ptr = ptr->prev;
      return tmp;
    }

    iterator &operator--() {
      ptr = ptr->prev;
      return *this;
    }

    value_type &operator*() const { return *(ptr->val); }

    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }

    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }

    value_type *operator->() const noexcept { return ptr->val; }
  };

  class const_iterator {
    friend linked_hashmap<Key, T>;
    friend iterator;

   private:
    node *ptr;

   public:
    node *get_ptr() { return ptr; }

    explicit const_iterator(node *ptr = nullptr) : ptr(ptr) {}

    const_iterator(const const_iterator &other) : ptr(other.ptr) {}

    const_iterator &operator=(const iterator &other) {
      ptr = other.ptr;
      return *this;
    }

    const_iterator &operator=(const const_iterator &other) {
      if (this == &other) return *this;
      ptr = other.ptr;
      return *this;
    }

    explicit const_iterator(const iterator &other) : ptr(other.ptr) {}

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ptr = ptr->next;
      return tmp;
    }

    const_iterator &operator++() {
      ptr = ptr->next;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator tmp = *this;
      ptr = ptr->prev;
      return tmp;
    }

    const_iterator &operator--() {
      ptr = ptr->prev;
      return *this;
    }

    const value_type &operator*() const { return *(ptr->val); }

    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }

    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }

    const value_type *operator->() const noexcept { return ptr->val; }
  };

  linked_hashmap() {
    head = new node;
    tail = new node;
    head->next = tail;
    tail->prev = head;
    currentSize = 0;
    capacity = 1e4+ 17;
    array = new node *[capacity];
    for (int i = 0; i < capacity; ++i) {
      array[i] = nullptr;
    }
  }

  linked_hashmap(const linked_hashmap &other) {
    head = new node;
    tail = new node;
    head->next = tail;
    tail->prev = head;
    capacity = other.capacity;
    currentSize = other.currentSize;
    array = new node *[capacity];
    for (int i = 0; i < capacity; ++i) {
      array[i] = nullptr;
    }
    node *p;
    node *q = other.head->next;
    size_t hs;
    while (q != other.tail) {
      p = new node(*(q->val));
      p->next = tail;
      p->prev = tail->prev;
      tail->prev = tail->prev->next = p;
      hs = get_hash(p->val->first);
      if (!array[hs]) {
        array[hs] = p;
      } else {
        p->_next = array[hs];
        array[hs] = p;
      }

      q = q->next;
    }
  }

  linked_hashmap &operator=(const linked_hashmap &other) {
    if (this == &other) return *this;
    clear();
    delete[] array;
    capacity = other.capacity;
    currentSize = other.currentSize;
    array = new node *[capacity];
    for (int i = 0; i < capacity; ++i) {
      array[i] = nullptr;
    }
    node *p;
    node *q = other.head->next;
    size_t hs;
    while (q != other.tail) {
      p = new node(*(q->val));
      p->next = tail;
      p->prev = tail->prev;
      tail->prev = tail->prev->next = p;
      hs = get_hash(p->val->first);
      if (!array[hs]) {
        array[hs] = p;
      } else {
        p->_next = array[hs];
        array[hs] = p;
      }

      q = q->next;
    }
    return *this;
  }

  ~linked_hashmap() {
    node *p = head->next, *tmp;
    while (p != tail) {
      tmp = p->next;
      delete p;
      p = tmp;
    }
    delete head;
    delete tail;
    delete[] array;
  }

  T &at(const Key &key) {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;

    return array[hs]->val->second;
  }

  const T &at(const Key &key) const {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;

    return array[hs]->val->second;
  }

  T &operator[](const Key &key) {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;
    if (!p) {
      value_type v{key, {}};
      auto pr = insert(v);
      return pr.first.get_ptr()->val->second;
    }
    return p->val->second;
  }

  const T &operator[](const Key &key) const {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;
    return p->val->second;
  }

  iterator begin() {
    iterator itr(head->next);
    return itr;
  }

  const_iterator cbegin() const {
    const_iterator itr(head->next);
    return itr;
  }

  iterator end() {
    iterator itr(tail);
    return itr;
  }

  const_iterator cend() const {
    const_iterator itr(tail);
    return itr;
  }

  [[nodiscard]] bool empty() const { return currentSize == 0; }

  [[nodiscard]] size_t size() const { return (size_t)currentSize; }

  void clear() {
    node *p = head->next;
    while (p != tail) {
      node *tmp = p->next;
      delete p;
      p = tmp;
    }
    for (int i = 0; i < capacity; ++i) {
      array[i] = nullptr;
    }
    head->next = tail;
    tail->prev = head;
    currentSize = 0;
  }

  pair<iterator, bool> insert(const value_type &value) {
    size_t hs = get_hash(value.first);
    node *p = array[hs];

    while (p && !equal(p->val->first, value.first)) p = p->_next;

    if (p) {
      pair<iterator, bool> pr(iterator(p), false);
      return pr;
    } else {
      p = array[hs] = new node(value, array[hs]);  // 插在头部
      p->next = tail;
      p->prev = tail->prev;
      tail->prev = tail->prev->next = p;
      ++currentSize;
      pair<iterator, bool> pr(iterator(p), true);
      return pr;
    }
  }

  void erase(iterator pos) {
    node *p = pos.get_ptr();
    size_t hs = get_hash(p->val->first);
    node *q = array[hs];
    if (q == p) {
      p->prev->next = p->next;
      p->next->prev = p->prev;
      array[hs] = q->_next;
      delete q;
      --currentSize;
      return;
    }
    while (q->_next && q->_next != p) q = q->_next;
    if (q->_next) {
      p->prev->next = p->next;
      p->next->prev = p->prev;
      q->_next = p->_next;
      --currentSize;
      delete p;
    }
  }

  size_t count(const Key &key) const {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;
    if (p)
      return 1;
    else
      return 0;
  }

  iterator find(const Key &key) {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;

    if (!p)
      return end();
    else
      return iterator(p);
  }

  const_iterator find(const Key &key) const {
    size_t hs = get_hash(key);
    node *p = array[hs];
    while (p && !equal(p->val->first, key)) p = p->_next;

    if (!p)
      return cend();
    else
      return const_iterator(p);
  }
};

}  // namespace CrazyDave
#endif  // BPT_PRO_LINKED_HASHMAP_HPP
