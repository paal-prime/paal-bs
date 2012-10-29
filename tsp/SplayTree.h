#ifndef TSP_SPLAYTREE_H_
#define TSP_SPLAYTREE_H_

#include <boost/utility.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <algorithm>

namespace tsp {

  template<typename N> inline size_t node_size(N node) {
    return (node == NULL) ? 0 : node->size();
  }

  template<typename V> class Node {
    public:
      typedef V value_type;
      typedef Node<value_type> node_type;

      explicit Node(const value_type &val) : val_(val) {
      }
      node_type *parent() {
        return parent_;
      }
      void make_root() {
        parent_ = NULL;
      }
      node_type *left() {
        normalize();
        return left_;
      }
      void set_left(node_type *node) {
        normalize();
        set_left_internal(node);
        update_size();
      }
      void set_left_internal(node_type *node) {
        left_ = node;
        if (node != NULL) {
          node->parent_ = this;
        }
      }
      node_type *right() {
        normalize();
        return right_;
      }
      void set_right(node_type *node) {
        normalize();
        set_right_internal(node);
        update_size();
      }
      void set_right_internal(node_type *node) {
        right_ = node;
        if (node != NULL) {
          node->parent_ = this;
        }
      }
      void update_size() {
        size_ = 1 + ((left_ != NULL) ? left_->size_ : 0)
                + ((right_ != NULL) ? right_->size_ : 0);
      }

      node_type *next() {
        normalize_root_path();
        node_type *node = right();
        if (node != NULL) {
          return node->subtree_min();
        } else {
          node_type *prev = NULL;
          node = this;
          for (;;) {
            prev = node;
            node = node->parent();
            if (node == NULL) {
              return NULL;
            } else if (node->left() == prev) {
              return node;
            }
          }
        }
      }
      node_type *prev() {
        normalize_root_path();
        node_type *node = left();
        if (node != NULL) {
          return node->subtree_max();
        } else {
          node_type *prev = NULL;
          node = this;
          for (;;) {
            prev = node;
            node = node->parent();
            if (node == NULL) {
              return NULL;
            } else if (node->right() == prev) {
              return node;
            }
          }
        }
      }
      node_type *subtree_min() {
        node_type *node = this;
        while (node->left() != NULL) {
          node = node->left();
        }
        return node;
      }
      node_type *subtree_max() {
        node_type *node = this;
        while (node->right() != NULL) {
          node = node->right();
        }
        return node;
      }

      size_t size() {
        return size_;
      }
      void subtree_reverse() {
        reversed_ ^= 1;
      }
      void normalize() {
        if (reversed_) {
          std::swap(left_, right_);
          if (left_ != NULL) {
            left_->subtree_reverse();
          }
          if (right_ != NULL) {
            right_->subtree_reverse();
          }
          reversed_ = false;
        }
      }
      void normalize_root_path() {
        node_type *node = parent();
        if (node != NULL) {
          node->normalize_root_path();
        }
        normalize();
      }

      value_type val_;

    private:
      static const bool kDefLeft = 0;

      bool left_index() {
        int index = kDefLeft;
        node_type *el = this;
        while (el != NULL) {
          if (el->reversed_) {
            ++index;
          }
          el = el->parent();
        }
        return (index % 2);
      }

      node_type *left_ = NULL, *right_ = NULL;
      node_type *parent_ = NULL;
      bool reversed_ = false;
      size_t size_ = 1;
  };

  template<typename V, bool IsForward> class Iterator
    : public boost::iterator_facade<
    Iterator<V, IsForward>,
    Node<V>*,
    boost::bidirectional_traversal_tag,
      V&> {
    public:
      typedef V value_type;
      typedef Node<value_type> node_type;

      Iterator() : current_(NULL) {
      }
      explicit Iterator(node_type *node) : current_(node) {
      }
      Iterator(const Iterator<value_type, IsForward> &other) :
        current_(other.current_) {
      }

    private:
      friend class boost::iterator_core_access;
      void increment() {
        if (IsForward) {
          current_ = current_->next();
        } else {
          current_ = current_->prev();
        }
      }
      void decrement() {
        if (IsForward) {
          current_ = current_->prev();
        } else {
          current_ = current_->next();
        }
      }
      bool equal(const Iterator<value_type, IsForward> &other) const {
        return this->current_ == other.current_;
      }
      value_type& dereference() const {
        return current_->val_;
      }
      node_type* current_;
  };

  template<typename T> class SplayTree {
    public:
      typedef T value_type;
      typedef Node<value_type> node_type;
      typedef Iterator<value_type, true> iterator;
      typedef const Iterator<value_type, true> const_iterator;
      typedef Iterator<value_type, false> reverse_iterator;
      typedef const Iterator<value_type, false> const_reverse_iterator;

      SplayTree() {}
      template<typename I> SplayTree(const I begin, const I end) {
        root_ = build_tree(begin, end);
      }
      ~SplayTree() {
        dispose_tree(root_);
      }
      iterator begin() {
        return (root_ == NULL) ? end() : iterator(root_->subtree_min());
      }
      iterator end() {
        return iterator();
      }
      reverse_iterator rbegin() {
        return (root_ == NULL) ? rend()
               : reverse_iterator(root_->subtree_max());
      }
      reverse_iterator rend() {
        return reverse_iterator();
      }
      size_t size() {
        return (root_ == NULL) ? 0 : root_->size();
      }
      bool empty() {
        return (root_ == NULL);
      }
      value_type& operator[](size_t i) {
        return find(i)->val_;
      }
      node_type *splay(size_t i) {
        root_ = splay_down(i);
        return root_;
      }
      SplayTree<value_type> split_higher(size_t i) {
        splay(i);
        node_type *new_root = root_->right();
        if (new_root != NULL) {
          new_root->make_root();
          root_->set_right(NULL);
        }
        return SplayTree(new_root);
      }
      SplayTree<value_type> split_lower(size_t i) {
        splay(i);
        node_type *new_root = root_->left();
        if (new_root != NULL) {
          new_root->make_root();
          root_->set_left(NULL);
        }
        return SplayTree(new_root);
      }
      void merge_right(SplayTree<value_type> &other) {  // NOLINT
        if (other.root_ == NULL) {
          return;
        }
        splay(root_->size() - 1);
        assert(root_->right() == NULL);
        root_->set_right(other.root_);
        other.root_ = NULL;
      }
      void merge_left(SplayTree<value_type> &other) {  // NOLINT
        if (other.root_ == NULL) {
          return;
        }
        splay(0);
        assert(root_->left() == NULL);
        root_->set_left(other.root_);
        other.root_ = NULL;
      }
      void reverse(size_t i, size_t j) {
        assert(i <= j);
        // split lower
        SplayTree<value_type> ltree = split_lower(i);
        // split higher
        SplayTree<value_type> rtree = split_higher(j - i);
        // reverse
        root_->subtree_reverse();
        // merge
        merge_left(ltree);
        merge_right(rtree);
      }

    private:
      explicit SplayTree(node_type *root) : root_(root) {
      }
      void splay_internal(node_type *const node) {
        node_type *const parent = node->parent();
        if (node == root_) {
          return;
        } else if (parent == root_) {
          if (node == parent->left()) {
            rotate_right(parent);
          } else {
            rotate_left(parent);
          }
        } else {
          node_type *const grand = parent->parent();
          if (node == parent->left() && parent == grand->left()) {
            rotate_right(grand);
            rotate_right(parent);
          } else if (node == parent->right() && parent == grand->right()) {
            rotate_left(grand);
            rotate_left(parent);
          } else if (node == parent->right() && parent == grand->left()) {
            rotate_left(parent);
            rotate_right(grand);
          } else if (node == parent->left() && parent == grand->right()) {
            rotate_right(parent);
            rotate_left(grand);
          }
        }
        splay_internal(node);
      }
      node_type *splay_down(size_t i) {
        node_type *parent = root_;
        node_type second_tree(-1);
        node_type *l = &second_tree, *r = &second_tree;

        for (;;) {
          size_t left_size = node_size(parent->left());
          if (left_size == i) {
            break;
          } else if (left_size > i) {
            node_type *node = parent->left();
            node->make_root();

            r->set_left_internal(parent);
            r = parent;

            parent = node;
          } else {
            node_type *node = parent->right();
            node->make_root();

            l->set_right_internal(parent);
            l = parent;

            parent = node;
            i -= left_size + 1;
          }
        }

        r->set_left_internal(parent->right());
        l->set_right_internal(parent->left());

        parent->set_left_internal(second_tree.right());
        parent->set_right_internal(second_tree.left());

        parent->make_root();

        while (r != NULL) {
          r->update_size();
          r = r->parent();
        }
        while (l != NULL) {
          l->update_size();
          l = l->parent();
        }

        return parent;
      }
      void rotate_right(node_type *parent) {
        node_type *const node = parent->left(),
                  *const grand = parent->parent();
        parent->set_left(node->right());
        if (grand != NULL) {
          if (parent == grand->right()) {
            grand->set_right(node);
          } else {
            grand->set_left(node);
          }
        }
        node->set_right(parent);
        if (parent == root_) {
          root_ = node;
          node->make_root();
        }
      }
      void rotate_left(node_type *parent) {
        node_type *const node = parent->right(),
                  *const grand = parent->parent();
        parent->set_right(node->left());
        if (grand != NULL) {
          if (parent == grand->left()) {
            grand->set_left(node);
          } else {
            grand->set_right(node);
          }
        }
        node->set_left(parent);
        if (parent == root_) {
          root_ = node;
          node->make_root();
        }
      }
      template<typename I> node_type *build_tree(const I begin, const I end) {
        if (begin >= end) {
          return NULL;
        }
        ssize_t m = (end - begin) / 2;
        node_type *node = new node_type(*(begin + m));
        node->set_left(build_tree(begin, begin + m));
        node->set_right(build_tree(begin + m + 1, end));
        return node;
      }
      void dispose_tree(node_type *node) {
        if (node == NULL) {
          return;
        }
        dispose_tree(node->left());
        dispose_tree(node->right());
        delete node;
      }
      node_type *find(size_t i) {
        node_type *node = root_;
        for (;;) {
          if (node == NULL) {
            return NULL;
          }
          node_type *left = node->left();
          size_t left_size = (left == NULL) ? 0 : left->size();
          if (left_size == i) {
            return node;
          } else if (left_size > i) {
            node = left;
          } else {
            i -= left_size + 1;
            node = node->right();
          }
        }
      }
      template<typename S, typename V> friend S& operator<<(
        S &s, SplayTree<V> &tree);

      node_type *root_ = NULL;
  };

  template<typename S, typename T> S& operator<<(S &s, SplayTree<T> &tree) {
    tree.root_->print_tree(s);
    return s;
  }
}

#endif  // TSP_SPLAYTREE_H_

