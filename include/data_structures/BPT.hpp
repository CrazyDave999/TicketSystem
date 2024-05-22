#ifndef BPT_BPT_HPP
#define BPT_BPT_HPP

#include <cstring>
#include "common/utils.hpp"
#include "data_structures/vector.h"

namespace CrazyDave {
template <class key_t = String<65>, class value_t = int, const int M = 100, const int L = 108>
class BPlusTree {
  struct Pair {
    key_t key{};
    value_t val{};

    Pair &operator=(const Pair &rhs) {
      key = rhs.key;
      val = rhs.val;
      return *this;
    }

    bool operator<(const Pair &rhs) const {
      if (key != rhs.key) return key < rhs.key;
      return val < rhs.val;
    }

    friend bool operator<(const Pair &lhs, const key_t &rhs_key) { return lhs.key < rhs_key; }

    friend bool operator<(const key_t &lhs_key, const Pair rhs) { return lhs_key < rhs.key; }
  };

  struct TreeNode;

  struct WTreeNode {  // Writable struct for a TreeNode.
    int pos{};
    int ch_num = 0;
    int block_pos = 0;
    int size = 0;
    Pair keys[M];
    int ch_pos[M + 1]{};  // The position of children's STreeNode in tree_file.

    WTreeNode() = default;

    explicit WTreeNode(const TreeNode &n) : pos(n.pos), ch_num(n.ch_num), block_pos(n.block_pos), size(n.size) {
      for (int i = 0; i < ch_num; ++i) {
        if (i < ch_num - 1) {
          keys[i] = n.keys[i];
        }
        ch_pos[i] = n.children[i]->pos;
      }
    }
  };

  struct TreeNode {
    TreeNode *fa = nullptr;
    int ch_num = 0;   // If ch_num=0, this node is a leaf.
    int pos{};        // The position of this node in tree_file.
    int block_pos{};  // The position of data in data_file. Only leaf node available.
    int size = 0;     // Size of block. Only leaf node available.
    Pair keys[M];     // At most M - 1 keys, leave one.
    TreeNode *children[M + 1];

    TreeNode(TreeNode *_fa, int _ch_num, int _pos, int _block_pos = 0)
        : fa(_fa), ch_num(_ch_num), pos(_pos), block_pos(_block_pos) {}

    explicit TreeNode(const WTreeNode &wn) : ch_num(wn.ch_num), pos(wn.pos), block_pos(wn.block_pos), size(wn.size) {
      for (int i = 0; i < ch_num - 1; ++i) {
        keys[i] = wn.keys[i];
      }
    }
  };

  struct Storage {
    vector<int> st;
    int size = 0;
    int max_pos = 0;
    int root_pos = 0;
    File *file;

    explicit Storage(File *_file) : file(_file) {}

    int assign() {
      ++size;
      if (st.empty()) {
        return max_pos = size - 1;
      }
      int res = st.back();
      st.pop_back();
      return res;
    }

    void destroy(int pos) {
      --size;
      st.push_back(pos);
    }

    void read() {
      file->seekg(0);
      file->read(size);
      file->read(max_pos);
      file->read(root_pos);
      st.clear();
      for (int i = 0; i < max_pos - size + 1; ++i) {
        int pos;
        file->read(pos);
        st.push_back(pos);
      }
    }

    void write() {
      file->seekp(0);
      file->write(size);
      file->write(max_pos);
      file->write(root_pos);
      for (int i = 0; i < max_pos - size + 1; ++i) {
        file->write(st[i]);
      }
    }
    void reset() {
      size = 0;
      max_pos = 0;
      root_pos = 0;
      st.clear();
    }
  };
  File node_file;          // Successively storage of WTreeNode.
  File node_storage_file;  // Information of node_storage.
  File block_file;
  File block_storage_file;
  TreeNode *root;
  using Block = Pair[L + 1];
  Block cache1;
  Block cache2;
  Storage node_storage{&node_storage_file};
  Storage block_storage{&block_storage_file};

  void read_node(TreeNode *&n) {  // Use dfs to read the structure of tree in tree_file.
    WTreeNode wn;
    node_file.read(wn);
    n = new TreeNode{wn};

    for (int i = 0; i < n->ch_num; ++i) {
      node_file.seekg(wn.ch_pos[i] * sizeof(WTreeNode));
      read_node(n->children[i]);
      n->children[i]->fa = n;
    }
  }

  void write_node(TreeNode *n) {
    WTreeNode wn{*n};
    node_file.write(wn);
    for (int i = 0; i < n->ch_num; ++i) {
      node_file.seekp(wn.ch_pos[i] * sizeof(WTreeNode));
      write_node(n->children[i]);
    }
    delete n;
  }
  void destroy_node(TreeNode *n) {
    for (int i = 0; i < n->ch_num; ++i) {
      destroy_node(n->children[i]);
    }
    delete n;
  }

  void read_block(TreeNode *n, Block &bk) {
    block_file.seekg(n->block_pos * sizeof(Block));
    block_file.read(bk, n->size * sizeof(Pair));
  }

  void write_block(TreeNode *n, Block &bk) {
    block_file.seekp(n->block_pos * sizeof(Block));
    block_file.write(bk, n->size * sizeof(Pair));
  }

  void split_node(TreeNode *n) {
    auto *tmp = new TreeNode{n->fa, M - M / 2, node_storage.assign()};

    for (int i = M / 2 + 1; i <= M; ++i) {
      if (i <= M - 1) {
        tmp->keys[i - M / 2 - 1] = n->keys[i];
      }
      tmp->children[i - M / 2 - 1] = n->children[i];
      n->children[i] = nullptr;
      tmp->children[i - M / 2 - 1]->fa = tmp;
    }
    n->ch_num = M / 2 + 1;
    // Lift keys[M / 2] to n->fa.
    if (n == root) {
      root = tmp->fa = n->fa = new TreeNode{nullptr, 1, node_storage.assign()};
      root->children[0] = n;
      node_storage.root_pos = root->pos;
    }
    auto *fa = n->fa;
    Pair &pr = n->keys[M / 2];
    int pos = increase_insert(fa->keys, fa->ch_num - 1, pr);
    insert_at(fa->children, fa->ch_num, pos + 1, tmp);
    ++fa->ch_num;
    if (fa->ch_num > M) {
      split_node(fa);
    }
  }

  void split_block(TreeNode *n, Block &bk) {
    auto *tmp = new TreeNode{n->fa, 0, node_storage.assign(), block_storage.assign()};
    Block &nx = cache2;
    for (int i = L - L / 2; i <= L; ++i) {
      nx[i - L + L / 2] = bk[i];
    }
    n->size = L - L / 2;
    tmp->size = L / 2 + 1;
    write_block(tmp, nx);
    if (n == root) {
      root = tmp->fa = n->fa = new TreeNode{nullptr, 1, node_storage.assign()};
      root->children[0] = n;
      node_storage.root_pos = root->pos;
    }
    auto *fa = n->fa;
    int pos = increase_insert(fa->keys, fa->ch_num - 1, nx[0]);
    insert_at(fa->children, fa->ch_num, pos + 1, tmp);
    ++fa->ch_num;
    if (fa->ch_num > M) {
      split_node(fa);
    }
  }

  void check_merge_node(TreeNode *n) {
    auto *fa = n->fa;
    int pos = upper_bound(fa->keys, fa->ch_num - 1, n->keys[0]);
    if (pos > 0) {
      TreeNode *left = fa->children[pos - 1];
      if (left->ch_num > M - M / 2) {
        Pair &pr = fa->keys[pos - 1];
        TreeNode *adopt = left->children[left->ch_num - 1];
        insert_at(n->keys, n->ch_num - 1, 0, pr);
        insert_at(n->children, n->ch_num, 0, adopt);
        adopt->fa = n;

        pr = left->keys[left->ch_num - 2];

        --left->ch_num;
        ++n->ch_num;
      } else {
        merge_node(left, n);
      }
    } else {
      TreeNode *right = fa->children[pos + 1];
      if (right->ch_num > M - M / 2) {
        Pair &pr = fa->keys[pos];
        TreeNode *adopt = right->children[0];
        insert_at(n->keys, n->ch_num - 1, n->ch_num - 1, pr);
        insert_at(n->children, n->ch_num, n->ch_num, adopt);
        adopt->fa = n;

        pr = right->keys[0];

        remove_at(right->keys, right->ch_num - 1, 0);
        remove_at(right->children, right->ch_num, 0);

        --right->ch_num;
        ++n->ch_num;
      } else {
        merge_node(n, right);
      }
    }
  }

  void check_merge_block(TreeNode *n, Block &bk) {
    auto *fa = n->fa;
    int pos = upper_bound(fa->keys, fa->ch_num - 1, bk[0]);
    if (pos > 0) {
      TreeNode *left = fa->children[pos - 1];
      Block &bkl = cache2;

      read_block(left, bkl);

      if (left->size > L - L / 2) {
        Pair &pr = bkl[left->size - 1];
        insert_at(bk, n->size, 0, pr);

        fa->keys[pos - 1] = pr;

        --left->size;
        ++n->size;

        write_block(left, bkl);
        write_block(n, bk);
      } else {
        merge_block(left, bkl, n, bk);
      }
    } else {
      TreeNode *right = fa->children[pos + 1];
      Block &bkr = cache2;

      read_block(right, bkr);

      if (right->size > L - L / 2) {
        Pair &pr = bkr[0];
        insert_at(bk, n->size, n->size, pr);
        remove_at(bkr, right->size, 0);

        fa->keys[pos] = bkr[0];

        --right->size;
        ++n->size;

        write_block(right, bkr);
        write_block(n, bk);
      } else {
        merge_block(n, bk, right, bkr);
      }
    }
  }

  void merge_node(TreeNode *n1, TreeNode *n2) {  // n1 is on the left of n2.
    auto *fa = n1->fa;
    Pair &pr = n1->keys[0];
    int pos = upper_bound(fa->keys, fa->ch_num - 1, pr);
    for (int i = 0; i <= n2->ch_num - 1; ++i) {
      n1->children[n1->ch_num + i] = n2->children[i];
      n1->children[n1->ch_num + i]->fa = n1;
    }
    n1->keys[n1->ch_num - 1] = fa->keys[pos];
    for (int i = 0; i <= n2->ch_num - 2; ++i) {
      n1->keys[n1->ch_num + i] = n2->keys[i];
    }

    n1->ch_num += n2->ch_num;  // 2(M - M / 2) - 1 <= M

    remove_at(fa->keys, fa->ch_num - 1, pos);
    remove_at(fa->children, fa->ch_num, pos + 1);

    node_storage.destroy(n2->pos);
    delete n2;
    --fa->ch_num;
    if (fa == root) {
      if (fa->ch_num == 1) {
        node_storage.destroy(root->pos);
        delete root;
        root = n1;
        node_storage.root_pos = root->pos;
      }
    } else if (fa->ch_num < M - M / 2) {
      check_merge_node(fa);
    }
  }

  void merge_block(TreeNode *n1, Block &bk1, TreeNode *n2, Block &bk2) {
    for (int i = 0; i <= n2->size - 1; ++i) {
      bk1[n1->size + i] = bk2[i];
    }
    n1->size += n2->size;
    Pair &pr = bk1[n1->size - 1];
    auto *fa = n1->fa;
    int pos = upper_bound(fa->keys, fa->ch_num - 1, pr);
    remove_at(fa->keys, fa->ch_num - 1, pos - 1);
    remove_at(fa->children, fa->ch_num, pos);

    node_storage.destroy(n2->pos);
    block_storage.destroy(n2->block_pos);
    delete n2;

    write_block(n1, bk1);
    --fa->ch_num;
    if (fa == root) {
      if (fa->ch_num == 1) {
        node_storage.destroy(root->pos);
        delete root;
        root = n1;
        node_storage.root_pos = root->pos;
      }
    } else if (fa->ch_num < M - M / 2) {
      check_merge_node(fa);
    }
  }

  void insert(TreeNode *n, const Pair &pr) {
    if (n->ch_num > 0) {
      // Not leaf node.
      int pos = upper_bound(n->keys, n->ch_num - 1, pr);
      insert(n->children[pos], pr);
      return;
    }
    // Leaf node.
    Block &bk = cache1;
    read_block(n, bk);
    increase_insert(bk, n->size, pr);
    ++n->size;
    write_block(n, bk);
    if (n->size > L) {
      split_block(n, bk);
    }
  }

  void remove(TreeNode *n, const Pair &pr) {
    if (n->ch_num > 0) {
      int pos = upper_bound(n->keys, n->ch_num - 1, pr);
      remove(n->children[pos], pr);
      return;
    }
    Block &bk = cache1;
    read_block(n, bk);
    if (increase_remove(bk, n->size, pr)) {
      --n->size;
      if (n == root || n->size >= L - L / 2) {
        write_block(n, bk);
      } else {
        // Adopt pair or merge block.
        check_merge_block(n, bk);
      }
    }
  }

  void find(TreeNode *n, const key_t &key, vector<value_t> &res) {
    if (n->ch_num == 0) {
      Block &bk = cache1;
      read_block(n, bk);
      int l = lower_bound(bk, n->size, key);
      int r = upper_bound(bk, n->size, key);
      for (int i = l; i < r; ++i) {
        res.push_back(bk[i].val);
      }
      return;
    }
    int l = lower_bound(n->keys, n->ch_num - 1, key);
    int r = upper_bound(n->keys, n->ch_num - 1, key);
    for (int i = l; i <= r; ++i) {
      find(n->children[i], key, res);
    }
  }

 public:
  BPlusTree(const char *node_str1, const char *node_str2, const char *block_str1, const char *block_str2)
      : node_file(node_str1), node_storage_file(node_str2), block_file(block_str1), block_storage_file(block_str2) {
    node_file.open();
    node_storage_file.open();
    block_file.open();
    block_storage_file.open();
    if (node_file.get_is_new()) {
      root = new TreeNode{nullptr, 0, node_storage.assign(), block_storage.assign()};
    } else {
      node_storage.read();
      block_storage.read();
      node_file.seekg(node_storage.root_pos * sizeof(WTreeNode));
      read_node(root);
    }
  }

  ~BPlusTree() {
    node_storage.write();
    block_storage.write();
    node_file.seekp(node_storage.root_pos * sizeof(WTreeNode));
    write_node(root);
    node_file.close();
    node_storage_file.close();
    block_file.close();
    block_storage_file.close();
  }

  void insert(const key_t &key, const value_t &val) {
    Pair pr{key, val};
    insert(root, pr);
  }

  void remove(const key_t &key, const value_t &val) {
    Pair pr{key, val};
    remove(root, pr);
  }

  vector<value_t> find(const key_t &key) {
    vector<value_t> res;
    find(root, key, res);
    return std::move(res);
  }
  void clear() {
    node_file.clear();
    node_storage_file.clear();
    block_file.clear();
    block_storage_file.clear();
    destroy_node(root);
    node_storage.reset();
    block_storage.reset();
    root = new TreeNode{nullptr, 0, node_storage.assign(), block_storage.assign()};
  }
};
}  // namespace CrazyDave

#endif  // BPT_BPT_HPP
