#ifndef BPT_PRO_MAP_H
#define BPT_PRO_MAP_H

#include <cstddef>
#include <functional>
#include "common/utils.h"

namespace CrazyDave {
template <class Key, class T, class Compare = std::less<Key>>
class map {
 public:
  using value_type = pair<const Key, T>;
  enum Color { RED, BLACK };

 private:
  struct TreeNode {
    value_type *data = nullptr;
    TreeNode *left = nullptr;
    TreeNode *right = nullptr;
    TreeNode *fa = nullptr;
    Color color = RED;

    TreeNode() = default;

    TreeNode(const value_type &_data, TreeNode *_left, TreeNode *_right, Color _color = RED)
        : left(_left), right(_right), color(_color) {
      data = new value_type{_data};
    }

    ~TreeNode() { delete data; }
  };

  TreeNode *root = nullptr;
  TreeNode *head = nullptr;
  TreeNode *tail = nullptr;
  TreeNode *pe = nullptr;  // for past-the-end iterator
  size_t num = 0;
  Compare cmp;

  void swap_tree_node(TreeNode *n1, TreeNode *n2) {
    if (root == n1) {
      root = n2;
    } else if (root == n2) {
      root = n1;
    }
    if (head == n1) {
      head = n2;
    } else if (head == n2) {
      head = n1;
    }
    if (tail == n1) {
      tail = n2;
    } else if (tail == n2) {
      tail = n1;
    }
    TreeNode *f1 = n1->fa, *f2 = n2->fa, *l1 = n1->left, *l2 = n2->left, *r1 = n1->right, *r2 = n2->right;
    if (f1) {
      TreeNode *&_n1 = n1 == f1->left ? f1->left : f1->right;
      _n1 = n2;
    }
    if (f2) {
      TreeNode *&_n2 = n2 == f2->left ? f2->left : f2->right;
      _n2 = n1;
    }
    if (l1) {
      l1->fa = n2;
    }
    if (r1) {
      r1->fa = n2;
    }
    if (l2) {
      l2->fa = n1;
    }
    if (r2) {
      r2->fa = n1;
    }
    std::swap(n1->fa, n2->fa);
    std::swap(n1->left, n2->left);
    std::swap(n1->right, n2->right);
  }

  void LL(TreeNode *gp) {
    TreeNode *pa = gp->left;
    gp->left = pa->right;
    if (pa->right) {
      pa->right->fa = gp;
    }
    pa->right = gp;
    pa->fa = gp->fa;
    if (gp->fa) {
      if (gp == gp->fa->left) {
        gp->fa->left = pa;
      } else {
        gp->fa->right = pa;
      }
    }
    gp->fa = pa;
    if (root == gp) {
      root = pa;
    }
  }

  void RR(TreeNode *gp) {
    TreeNode *pa = gp->right;
    gp->right = pa->left;
    if (pa->left) {
      pa->left->fa = gp;
    }
    pa->left = gp;
    pa->fa = gp->fa;
    if (gp->fa) {
      if (gp == gp->fa->left) {
        gp->fa->left = pa;
      } else {
        gp->fa->right = pa;
      }
    }
    gp->fa = pa;
    if (root == gp) {
      root = pa;
    }
  }

  void LR(TreeNode *gp) {
    TreeNode *pa = gp->left;
    RR(pa);
    LL(gp);
  }

  void RL(TreeNode *gp) {
    TreeNode *pa = gp->right;
    LL(pa);
    RR(gp);
  }

  void successive_red_adjust(TreeNode *n) {
    if (n == root) {
      // The case when root is black and its two sons are both red.
      // In insert function root change to red and its two sons change to black.
      n->color = BLACK;
      return;
    }
    TreeNode *pa = n->fa;
    TreeNode *gp = pa->fa;
    TreeNode *si = pa == gp->left ? gp->right : gp->left;  // sibling of pa
    if (si == nullptr || si->color == BLACK) {
      if (pa == gp->left) {
        if (n == pa->left) {
          std::swap(gp->color, pa->color);
          LL(gp);
        } else {
          std::swap(gp->color, n->color);
          LR(gp);
        }
      } else {
        if (n == pa->left) {
          std::swap(gp->color, n->color);
          RL(gp);
        } else {
          std::swap(gp->color, pa->color);
          RR(gp);
        }
      }
    } else {
      gp->color = RED;
      pa->color = BLACK;
      si->color = BLACK;
      if (gp != root && gp->fa->color == RED) {
        successive_red_adjust(gp);
      }
    }
    root->color = BLACK;
  };

  void imbalance_adjust(TreeNode *n) {
    // For fixing imbalanced node created by removing a black leaf node and its sibling is also black leaf.
    TreeNode *pa = n->fa;
    TreeNode *si = n == pa->left ? pa->right : pa->left;
    if (pa->color == RED) {
      if (n == pa->left) {
        if (si->left->color == BLACK) {
          RR(pa);
        } else if (si->right->color == BLACK) {
          pa->color = BLACK;
          si->color = RED;
          successive_red_adjust(si->left);
        } else {
          pa->color = BLACK;
          si->color = RED;
          si->right->color = BLACK;
          RR(pa);
        }
      } else {
        if (si->right->color == BLACK) {
          LL(pa);
        } else if (si->left->color == BLACK) {
          pa->color = BLACK;
          si->color = RED;
          successive_red_adjust(si->right);
        } else {
          pa->color = BLACK;
          si->color = RED;
          si->left->color = BLACK;
          LL(pa);
        }
      }
    } else {  // pa->color == BLACK
      if (si->color == BLACK) {
        if (si->left->color == BLACK && si->right->color == BLACK) {
          si->color = RED;
          if (pa != root) {
            imbalance_adjust(pa);
          }
        } else {
          if (n == pa->left) {
            if (si->right->color == RED) {
              si->right->color = BLACK;
              RR(pa);
            } else if (si->left->color == RED) {
              si->left->color = BLACK;
              RL(pa);
            }
          } else {
            if (si->left->color == RED) {
              si->left->color = BLACK;
              LL(pa);
            } else if (si->right->color == RED) {
              si->right->color = BLACK;
              LR(pa);
            }
          }
        }
      } else {  // pa->color == BLACK, si->color == RED
        pa->color = RED;
        si->color = BLACK;
        if (n == pa->left) {
          RR(pa);
        } else {
          LL(pa);
        }
        imbalance_adjust(n);
      }
    }
  }

  void copy(TreeNode *&n, TreeNode *_n, const map &_mp) {
    n = new TreeNode{*(_n->data), nullptr, nullptr, _n->color};
    if (_n == _mp.head) {
      head = n;
    }
    if (_n == _mp.tail) {
      tail = n;
    }
    if (_n->left) {
      copy(n->left, _n->left, _mp);
      n->left->fa = n;
    }
    if (_n->right) {
      copy(n->right, _n->right, _mp);
      n->right->fa = n;
    }
  }

  void destroy(TreeNode *n) {
    if (n->left) {
      destroy(n->left);
    }
    if (n->right) {
      destroy(n->right);
    }
    delete n;
  }

 public:
  class const_iterator;

  class iterator {
    friend map;

   private:
    TreeNode *n = nullptr;
    const map *mp = nullptr;

   public:
    iterator() = default;

    iterator(TreeNode *_n, const map *_mp) : n(_n), mp(_mp) {}

    iterator(const iterator &other) : n(other.n), mp(other.mp) {}

    iterator operator++(int) {
      iterator it{*this};
      if (n == mp->tail) {
        n = mp->pe;
        return it;
      }
      if (n->right) {
        n = n->right;
        while (n->left) {
          n = n->left;
        }
      } else {
        while (n != n->fa->left) {
          n = n->fa;
        }
        n = n->fa;
      }
      return it;
    }

    iterator &operator++() {
      if (n == mp->tail) {
        n = mp->pe;
        return *this;
      }
      if (n->right) {
        n = n->right;
        while (n->left) {
          n = n->left;
        }
      } else {
        while (n != n->fa->left) {
          n = n->fa;
        }
        n = n->fa;
      }
      return *this;
    }

    iterator operator--(int) {
      iterator it{*this};
      if (n == mp->pe) {
        n = mp->tail;
        return it;
      }
      if (n->left) {
        n = n->left;
        while (n->right) {
          n = n->right;
        }
      } else {
        while (n != n->fa->right) {
          n = n->fa;
        }
        n = n->fa;
      }
      return it;
    }

    iterator &operator--() {
      if (n == mp->pe) {
        n = mp->tail;
        return *this;
      }
      if (n->left) {
        n = n->left;
        while (n->right) {
          n = n->right;
        }
      } else {
        while (n != n->fa->right) {
          n = n->fa;
        }
        n = n->fa;
      }
      return *this;
    }

    value_type &operator*() const { return *(n->data); }

    bool operator==(const iterator &rhs) const { return n == rhs.n; }

    bool operator==(const const_iterator &rhs) const { return n == rhs.n; }

    bool operator!=(const iterator &rhs) const { return n != rhs.n; }

    bool operator!=(const const_iterator &rhs) const { return n != rhs.n; }

    value_type *operator->() const noexcept { return n->data; }
  };

  class const_iterator {
    friend map;

   private:
    TreeNode *n = nullptr;
    const map *mp = nullptr;

   public:
    const_iterator() = default;

    const_iterator(TreeNode *_n, const map *_mp) : n(_n), mp(_mp) {}

    const_iterator(const const_iterator &other) : n(other.n), mp(other.mp) {}

    const_iterator(const iterator &other) : n(other.n), mp(other.mp) {}

    const_iterator operator++(int) {
      const_iterator it{*this};
      if (n == mp->tail) {
        n = mp->pe;
        return it;
      }
      if (n->right) {
        n = n->right;
        while (n->left) {
          n = n->left;
        }
      } else {
        while (n != n->fa->left) {
          n = n->fa;
        }
        n = n->fa;
      }
      return it;
    }

    const_iterator &operator++() {
      if (n == mp->tail) {
        n = mp->pe;
        return *this;
      }
      if (n->right) {
        n = n->right;
        while (n->left) {
          n = n->left;
        }
      } else {
        while (n != n->fa->left) {
          n = n->fa;
        }
        n = n->fa;
      }
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator it{*this};
      if (n == mp->pe) {
        n = mp->tail;
        return it;
      }
      if (n->left) {
        n = n->left;
        while (n->right) {
          n = n->right;
        }
      } else {
        while (n != n->fa->right) {
          n = n->fa;
        }
        n = n->fa;
      }
      return it;
    }

    const_iterator &operator--() {
      if (n == mp->pe) {
        n = mp->tail;
        return *this;
      }
      if (n->left) {
        n = n->left;
        while (n->right) {
          n = n->right;
        }
      } else {
        while (n != n->fa->right) {
          n = n->fa;
        }
        n = n->fa;
      }
      return *this;
    }

    value_type &operator*() const { return *(n->data); }

    bool operator==(const iterator &rhs) const { return n == rhs.n; }

    bool operator==(const const_iterator &rhs) const { return n == rhs.n; }

    bool operator!=(const iterator &rhs) const { return n != rhs.n; }

    bool operator!=(const const_iterator &rhs) const { return n != rhs.n; }

    value_type *operator->() const noexcept { return n->data; }
  };

  map() { pe = new TreeNode; }

  map(const map &other) {
    pe = new TreeNode;
    num = other.num;
    if (!other.empty()) {
      copy(root, other.root, other);
    }
  }

  map &operator=(const map &other) {
    if (this == &other) {
      return *this;
    }
    clear();
    delete pe;
    pe = new TreeNode;
    num = other.num;
    if (!other.empty()) {
      copy(root, other.root, other);
    }
    return *this;
  }

  ~map() {
    clear();
    delete pe;
  }

  T &at(const Key &key) {
    iterator it = find(key);
    return it->second;
  }

  const T &at(const Key &key) const {
    const_iterator it = find(key);
    return it->second;
  }

  T &operator[](const Key &key) {
    iterator it = find(key);
    if (it == end()) {
      return insert({key, T()}).first->second;
    }
    return it->second;
  }

  const T &operator[](const Key &key) const {
    const_iterator it = find(key);
    return it->second;
  }

  iterator begin() {
    if (empty()) {
      return {pe, this};
    }
    return {head, this};
  }

  const_iterator cbegin() const {
    if (empty()) {
      return {pe, this};
    }
    return {head, this};
  }

  iterator end() const { return {pe, this}; }

  const_iterator cend() const { return {pe, this}; }

  bool empty() const { return num == 0; }

  size_t size() const { return num; }

  void clear() {
    if (root) {
      destroy(root);
    }
    root = head = tail = nullptr;
    num = 0;
  }

  pair<iterator, bool> insert(const value_type &val) {
    if (root == nullptr) {
      head = tail = root = new TreeNode{val, nullptr, nullptr, BLACK};
      ++num;
      iterator it{root, this};
      return {it, true};
    }
    TreeNode *n, *pa;
    n = pa = root;
    while (true) {  // Top-down method
      if (n == nullptr) {
        n = new TreeNode{val, nullptr, nullptr, RED};
        if (cmp(n->data->first, pa->data->first)) {
          pa->left = n;
          if (head == pa) {
            head = n;
          }
        } else {
          pa->right = n;
          if (tail == pa) {
            tail = n;
          }
        }
        n->fa = pa;
        if (pa->color == RED) {
          successive_red_adjust(n);
        }

        ++num;
        iterator it{n, this};
        return {it, true};
      } else {
        if (n->left && n->left->color == RED && n->right && n->right->color == RED) {
          n->color = RED;
          n->left->color = n->right->color = BLACK;
          if (pa->color == RED) {
            successive_red_adjust(n);
          }
        }
      }
      pa = n;
      if (cmp(val.first, n->data->first)) {
        n = n->left;
      } else if (cmp(n->data->first, val.first)) {
        n = n->right;
      } else {  // n->data->first equals to val.first
        iterator it{n, this};
        return {it, false};
      }
    }
  }

  void erase(iterator pos) {
    TreeNode *n = pos.n;

    if (n->left && n->right) {
      iterator it = pos++;
      TreeNode *sb = pos.n;
      swap_tree_node(n, sb);
      std::swap(n->color, sb->color);
      erase(it);
      return;
    }
    if (n->left) {
      TreeNode *sb = n->left;
      swap_tree_node(n, sb);
      std::swap(n->color, sb->color);
    }
    if (n->right) {
      TreeNode *sb = n->right;
      swap_tree_node(n, sb);
      std::swap(n->color, sb->color);
    }
    // Leaf node.
    --num;
    if (n == root) {
      delete root;
      root = head = tail = nullptr;
      return;
    }
    TreeNode *&pa = n->fa;
    if (n->color == BLACK) {
      TreeNode *si = n == pa->left ? pa->right : pa->left;
      if (si->color == BLACK) {
        if (si->left && si->right) {  // two si sons red.
          std::swap(pa->color, si->color);
          if (n == pa->left) {
            si->right->color = BLACK;
            RR(pa);
          } else {
            si->left->color = BLACK;
            LL(pa);
          }
        } else {
          if (n == pa->left) {
            if (si->right) {
              std::swap(pa->color, si->color);
              si->right->color = BLACK;
              RR(pa);
            } else if (si->left) {
              si->left->color = pa->color;
              pa->color = BLACK;
              RL(pa);
            } else {
              si->color = RED;
              if (pa->color == RED) {
                pa->color = BLACK;
              } else {
                if (pa != root) {
                  imbalance_adjust(pa);
                }
              }
            }
          } else {
            if (si->left) {
              std::swap(pa->color, si->color);
              si->left->color = BLACK;
              LL(pa);
            } else if (si->right) {
              si->right->color = pa->color;
              pa->color = BLACK;
              LR(pa);
            } else {
              si->color = RED;
              if (pa->color == RED) {
                pa->color = BLACK;
              } else {
                if (pa != root) {
                  imbalance_adjust(pa);
                }
              }
            }
          }
        }
      } else {  // n->color == BLACK, si->color == RED
        pa->color = RED;
        si->color = BLACK;
        if (n == pa->left) {
          RR(pa);
          RR(pa);
          if (pa->right) {
            successive_red_adjust(pa->right);
          }
        } else {
          LL(pa);
          LL(pa);
          if (pa->left) {
            successive_red_adjust(pa->left);
          }
        }
      }
    }
    if (n == pa->left) {
      if (head == n) {
        head = pa;
      }
      pa->left = nullptr;
    } else {
      if (tail == n) {
        tail = pa;
      }
      pa->right = nullptr;
    }
    delete n;
  }

  size_t count(const Key &key) const { return find(key) != end(); }

  iterator find(const Key &key) {
    if (empty()) {
      return end();
    }
    TreeNode *n = root;
    while (true) {
      if (n == nullptr) {
        return end();
      }
      if (cmp(key, n->data->first)) {
        n = n->left;
      } else if (cmp(n->data->first, key)) {
        n = n->right;
      } else {
        return {n, this};
      }
    }
  }

  const_iterator find(const Key &key) const {
    if (empty()) {
      return cend();
    }
    TreeNode *n = root;
    while (true) {
      if (n == nullptr) {
        return cend();
      }
      if (cmp(key, n->data->first)) {
        n = n->left;
      } else if (cmp(n->data->first, key)) {
        n = n->right;
      } else {
        return {n, this};
      }
    }
  }
};
}  // namespace CrazyDave
#endif  // BPT_PRO_MAP_H
