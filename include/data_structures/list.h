#ifndef BPT_PRO_LIST_H
#define BPT_PRO_LIST_H
#include <cstddef>
namespace CrazyDave {
template <typename T>
class list {
 protected:
  class node {
   public:
    T *data;
    node *next;
    node *prev;
    node() {
      next = prev = nullptr;
      data = nullptr;
    }
    explicit node(T &&D, node *N = nullptr, node *P = nullptr) : next(N), prev(P) { data = new T(std::move(D)); }
    explicit node(const T &D, node *N = nullptr, node *P = nullptr) : next(N), prev(P) { data = new T(std::move(D)); }
    ~node() { delete data; };
  };

 protected:
  node *head, *tail;
  int currentSize;
  node *insert(node *pos, node *cur) {
    cur->prev = pos->prev;
    cur->next = pos;
    pos->prev = pos->prev->next = cur;
    ++currentSize;
    return cur;
  }
  node *erase(node *pos) {
    pos->prev->next = pos->next;
    pos->next->prev = pos->prev;
    --currentSize;
    return pos;
  }

 public:
  class const_iterator;

  class iterator {
    friend list<T>;

   private:
    node *ptr;

   public:
    explicit iterator(node *ptr = nullptr) : ptr(ptr) {}

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

    T &operator*() const { return *(ptr->data); }

    T *operator->() const noexcept { return ptr->data; }

    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }

    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }

    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
  };

  class const_iterator {
    friend list<T>;

   private:
    node *ptr;

   public:
    explicit const_iterator(node *ptr = nullptr) : ptr(ptr) {}
    explicit const_iterator(iterator &other) : ptr(other.ptr) {}
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
    T &operator*() const { return *(ptr->data); }
    T *operator->() const noexcept { return ptr->data; }
    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
  };
  list() {
    head = new node;
    tail = new node;
    head->next = tail;
    tail->prev = head;
    currentSize = 0;
  }
  list(const list &other) {
    head = new node;
    tail = new node;
    head->next = tail;
    tail->prev = head;
    node *p = other.head->next;
    while (p != other.tail) {
      push_back(*(p->data));
      p = p->next;
    }
    currentSize = other.currentSize;
  }
  virtual ~list() {
    clear();
    delete head;
    delete tail;
  }
  list &operator=(const list &other) {
    if (this == &other) return *this;
    clear();
    node *p = other.head->next;
    while (p != other.tail) {
      push_back(*(p->data));
      p = p->next;
    }
    currentSize = other.currentSize;
    return *this;
  }
  T &front() const { return *(head->next->data); }
  T &back() const { return *(tail->prev->data); }
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
  [[nodiscard]] virtual bool empty() const { return currentSize == 0; }
  [[nodiscard]] virtual size_t size() const { return currentSize; }

  virtual void clear() {
    while (!empty()) {
      pop_back();
    }
  }


  virtual iterator erase(iterator pos) {
    pos.ptr->prev->next = pos.ptr->next;
    pos.ptr->next->prev = pos.ptr->prev;
    node *tmp = pos.ptr->next;
    delete pos.ptr;
    pos.ptr = tmp;
    --currentSize;
    return pos;
  }
  void push_back(const T &value) {
    node *tmp = new node(value, tail, tail->prev);
    tail->prev->next = tmp;
    tail->prev = tmp;
    ++currentSize;
  }
  void push_back(T &&value) {
    node *tmp = new node(std::move(value), tail, tail->prev);
    tail->prev->next = tmp;
    tail->prev = tmp;
    ++currentSize;
  }
  void pop_back() {
    node *tmp = tail->prev;
    tmp->prev->next = tail;
    tail->prev = tmp->prev;
    delete tmp;
    --currentSize;
  }
  void push_front(T &value) {
    node *tmp = new node(value, head->next, head);
    head->next->prev = tmp;
    head->next = tmp;
    ++currentSize;
  }
  void pop_front() {
    node *tmp = head->next;
    tmp->next->prev = head;
    head->next = tmp->next;
    delete tmp;
    --currentSize;
  }
};

}  // namespace CrazyDave

#endif  // BPT_PRO_LIST_H
