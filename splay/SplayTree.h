#ifndef SPLAY_SPLAYTREE_H_
#define SPLAY_SPLAYTREE_H_

#include <boost/utility.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <algorithm>

namespace splay {

  /**
   * @param node root of a subtree
   * @returns size of subtree
   **/
  template<typename N> inline size_t node_size(N node) {
    return (node == NULL) ? 0 : node->size();
  }

  /**
   * Node of a SplayTree.
   *
   * Left/right relaxation should be understood as follows.
   * Meaning of left/right field changes iff xor of all bits on the path to the
   * root is 1. This enables us to reverse entire subtree in constant time (by
   * flipping bit in the root). Normalization is needed to determine which child is
   * the real left/right. */
  template<typename V> class Node {
    public:
      typedef V value_type;
      typedef Node<value_type> node_type;

      /** @param val stored value */
      explicit Node(const value_type &val) : val_(val) {
      }

      /** @returns parent node */
      node_type *parent() {
        return parent_;
      }

      /** @brief detaches this node from parent */
      void make_root() {
        parent_ = NULL;
      }

      /** @returns true left child pointer */
      node_type *left() {
        normalize();
        return left_;
      }

      /**
       * @brief sets true left child pointer
       * @param node new child
       **/
      void set_left(node_type *node) {
        normalize();
        set_left_internal(node);
        update_size();
      }

      /**
       * @brief sets left child pointer (no relaxation)
       * @param node new child
       **/
      void set_left_internal(node_type *node) {
        left_ = node;
        if (node != NULL) {
          node->parent_ = this;
        }
      }

      /** @returns true right child pointer */
      node_type *right() {
        normalize();
        return right_;
      }

      /**
       * @brief sets true right child pointer
       * @param node new child
       **/
      void set_right(node_type *node) {
        normalize();
        set_right_internal(node);
        update_size();
      }

      /**
       * @brief sets right child pointer (no relaxation)
       * @param node new child
       **/
      void set_right_internal(node_type *node) {
        right_ = node;
        if (node != NULL) {
          node->parent_ = this;
        }
      }

      /** @brief recomputes subtree size from sizes of children's subtrees */
      void update_size() {
        size_ = 1 + ((left_ != NULL) ? left_->size_ : 0)
                + ((right_ != NULL) ? right_->size_ : 0);
      }

      /** @returns next in same tree according to infix order */
      node_type *next() {
        normalize_root_path();
        node_type *node = right();
        if (node != NULL) {
          return node->subtree_min();
        } else {
          node_type *last = NULL;
          node = this;
          for (;;) {
            last = node;
            node = node->parent();
            if (node == NULL) {
              return NULL;
            } else if (node->left() == last) {
              return node;
            }
          }
        }
      }

      /** @returns previous in same tree according to infix order */
      node_type *prev() {
        normalize_root_path();
        node_type *node = left();
        if (node != NULL) {
          return node->subtree_max();
        } else {
          node_type *last = NULL;
          node = this;
          for (;;) {
            last = node;
            node = node->parent();
            if (node == NULL) {
              return NULL;
            } else if (node->right() == last) {
              return node;
            }
          }
        }
      }

      /** @returns first node in subtree according to infix order */
      node_type *subtree_min() {
        node_type *node = this;
        while (node->left() != NULL) {
          node = node->left();
        }
        return node;
      }

      /** @returns last node in subtree according to infix order */
      node_type *subtree_max() {
        node_type *node = this;
        while (node->right() != NULL) {
          node = node->right();
        }
        return node;
      }

      /** @returns size of subtree */
      size_t size() {
        return size_;
      }

      /** @brief lazily reverses order in subtree */
      void subtree_reverse() {
        reversed_ ^= 1;
      }

      /** @brief locally relaxes tree */
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

      /** @brief relaxes all nodes on path from root to this */
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
      node_type *left_ = NULL, *right_ = NULL;
      node_type *parent_ = NULL;
      bool reversed_ = false;
      size_t size_ = 1;
  };

  /**
   * SplayTree elements iterator.
   *
   * Traversing order is determined by template argument.
   **/
  template<typename V, bool IsForward> class Iterator
    : public boost::iterator_facade <
    Iterator<V, IsForward>,
    Node<V>*,
    boost::bidirectional_traversal_tag,
      V& > {
    public:
      typedef V value_type;
      typedef Node<value_type> node_type;

      /** @brief iterator after last element */
      Iterator() : current_(NULL) {
      }

      /**
       * @brief iterator to element in given node
       * @param node node storing element pointed by iterator
       **/
      explicit Iterator(node_type *node) : current_(node) {
      }

      /**
       * @brief copy constructor
       * @param other iterator to be copied
       **/
      Iterator(const Iterator<value_type, IsForward> &other) :
        current_(other.current_) {
      }

    private:
      friend class boost::iterator_core_access;

      /** @brief increments iterator */
      void increment() {
        if (IsForward) {
          current_ = current_->next();
        } else {
          current_ = current_->prev();
        }
      }

      /** @brief decrements iterator */
      void decrement() {
        if (IsForward) {
          current_ = current_->prev();
        } else {
          current_ = current_->next();
        }
      }

      /**
       * @param other iterator to be compared with
       * @returns true iff iterators point to the same node
       **/
      bool equal(const Iterator<value_type, IsForward> &other) const {
        return this->current_ == other.current_;
      }

      /** @returns reference to pointed element */
      value_type& dereference() const {
        return current_->val_;
      }

      /** pointed node */
      node_type* current_;
  };

  /** @brief splay policy */
  enum SplayImplEnum {
    /** splaying goes from root, resulting tree is less balanced */
    kTopDownUnbalanced,
    /** splaying goes from root */
    kTopDown,
    /** splaying goes from node to become root */
    kBottomUp
  };

  /**
   * Splay trees with logarithmic reversing of any subsequence.
   *
   * All tree operations are amortized logarithmic time in size of tree,
   * each element is indexed by number of smaller elements than this element.
   * Note that lookups are also amortized logarithmic in size of tree. Order of
   * elements is induced from infix ordering of nodes storing these elements.
   **/
  template<typename T, enum SplayImplEnum SplayImpl = kTopDownUnbalanced>
  class SplayTree {
    public:
      typedef T value_type;
      typedef Node<value_type> node_type;
      typedef Iterator<value_type, true> iterator;
      typedef const Iterator<value_type, true> const_iterator;
      typedef Iterator<value_type, false> reverse_iterator;
      typedef const Iterator<value_type, false> const_reverse_iterator;

      SplayTree() {}

      /**
       * @brief constructs tree from elements between two iterators
       * @param b iterator to first element
       * @param e iterator to element after last
       **/
      template<typename I> SplayTree(const I b, const I e) {
        root_ = build_tree(b, e);
      }

      /**
       * @brief creates tree from elements in std::vector
       * @param array vector container
       **/
      template<typename A> explicit SplayTree(const A &array) {
        root_ = build_tree(array, 0, array.size());
      }

      ~SplayTree() {
        dispose_tree(root_);
      }

      /** @returns forward iterator to first element in container */
      iterator begin() {
        return (root_ == NULL) ? end() : iterator(root_->subtree_min());
      }

      /** @returns forward iterator to element after last in container */
      iterator end() {
        return iterator();
      }

      /** @returns reverse iterator to last element in container */
      reverse_iterator rbegin() {
        return (root_ == NULL) ? rend()
               : reverse_iterator(root_->subtree_max());
      }

      /** @returns reverse iterator to element before first in container */
      reverse_iterator rend() {
        return reverse_iterator();
      }

      /** @returns number of elements in tree */
      size_t size() const {
        return (root_ == NULL) ? 0 : root_->size();
      }

      /** @returns true iff tree contains no elements */
      bool empty() {
        return (root_ == NULL);
      }

      /** @param i index of referenced element */
      value_type& operator[](size_t i) const {
        return find(i)->val_;
      }

      /**
       * @brief splays tree according to splay policy
       * @param i index of element to become root
       **/
      node_type *splay(size_t i) {
        switch (SplayImpl) {
          case kTopDownUnbalanced:
          case kTopDown:
            root_ = splay_down(i);
            return root_;
          case kBottomUp:
            splay_internal(find(i));
            return root_;
        }
      }

      /**
       * @brief splits sequence, modified this contains elements {0, ..., i}
       * @param i index of last element of this after modification
       * @returns tree containing elements {i+1, ...}
       **/
      SplayTree<value_type, SplayImpl> split_higher(size_t i) {
        splay(i);
        node_type *new_root = root_->right();
        if (new_root != NULL) {
          new_root->make_root();
          root_->set_right(NULL);
        }
        return SplayTree<value_type, SplayImpl>(new_root);
      }

      /**
       * @brief splits sequence, modified this contains elements {i, ...}
       * @param i index of first element of this after modification
       * @returns tree containing elements {0, ..., i-1}
       **/
      SplayTree<value_type, SplayImpl> split_lower(size_t i) {
        splay(i);
        node_type *new_root = root_->left();
        if (new_root != NULL) {
          new_root->make_root();
          root_->set_left(NULL);
        }
        return SplayTree<value_type, SplayImpl>(new_root);
      }

      /**
       * @brief merges given tree to the right of the biggest element of this
       * @param other tree to be merged
       **/
      template<enum SplayImplEnum S>
      void merge_right(SplayTree<value_type, S> &other) {
        if (other.root_ == NULL) {
          return;
        }
        splay(root_->size() - 1);
        assert(root_->right() == NULL);
        root_->set_right(other.root_);
        other.root_ = NULL;
      }

      /**
       * @brief merges given tree to the left of the smallest element of this
       * @param other tree to be merged
       **/
      template<enum SplayImplEnum S>
      void merge_left(SplayTree<value_type, S> &other) {
        if (other.root_ == NULL) {
          return;
        }
        splay(0);
        assert(root_->left() == NULL);
        root_->set_left(other.root_);
        other.root_ = NULL;
      }

      /**
       * @brief reverses subsequence of elements with indices in {i, ..., j}
       * @param i index of first element of subsequence
       * @param j index of last element of subsequence
       **/
      void reverse(size_t i, size_t j) {
        assert(i <= j);
        // split lower
        SplayTree<value_type, SplayImpl> ltree = split_lower(i);
        // split higher
        SplayTree<value_type, SplayImpl> rtree = split_higher(j - i);
        // reverse
        root_->subtree_reverse();
        // merge
        merge_left(ltree);
        merge_right(rtree);
      }

    private:
      /** @brief creates tree with given node as a root */
      explicit SplayTree(node_type *root) : root_(root) {
      }

      /**
       * @brief splays given node to tree root
       * @param node node of a tree to be moved to root
       **/
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

      /**
       * @brief splays node with given index to tree root
       * @param i index of a node to become root
       **/
      node_type *splay_down(size_t i) {
        node_type *parent = root_;
        node_type second_tree(-1);
        node_type *l = &second_tree, *r = &second_tree;

        for (;;) {
          size_t left_size = node_size(parent->left());
          if (left_size == i) {
            break;
          } else if (left_size > i) {
            if (SplayImpl != kTopDownUnbalanced) {
              if (node_size(parent->left()->left()) > i) {
                node_type *node = parent->left();
                parent->set_left(node->right());
                node->set_right_internal(parent);
                node->make_root();
                parent = node;
              }
            }
            node_type *node = parent->left();
            node->make_root();
            r->set_left_internal(parent);
            r = parent;

            parent = node;
          } else {
            if (SplayImpl != kTopDownUnbalanced) {
              if (left_size + 1 + node_size(parent->right()->left()) < i) {
                node_type *node = parent->right();
                parent->set_right(node->left());
                node->set_left_internal(parent);
                node->make_root();
                left_size = parent->size();
                parent = node;
              }
            }
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

      /**
       * @brief rotates tree right over given node
       * @param parent pivot of rotation
       **/
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

      /**
       * @brief rotates tree left over given node
       * @param parent pivot of rotation
       **/
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

      /**
       * @brief recursively creates balanced tree from a structure described
       *        by two random access iterators
       * @param b iterator to first element
       * @param e iterator to element after last
       **/
      template<typename I> node_type *build_tree(const I b, const I e) {
        if (b >= e) {
          return NULL;
        }
        ssize_t m = (e - b) / 2;
        node_type *node = new node_type(*(b + m));
        node->set_left(build_tree(b, b + m));
        node->set_right(build_tree(b + m + 1, e));
        return node;
      }

      /**
       * @brief recursively creates balanced tree from a structure with random
       *        access operator []
       * @param array structure holding data to be copied into tree
       * @param b index of first element in structure to be placed in tree
       * @param e index of element after last to be placed in tree
       **/
      template<typename A> node_type *build_tree(const A &array,
          const size_t b, const size_t e) {
        if (b >= e) {
          return NULL;
        }
        ssize_t m = (e + b) / 2;
        node_type *node = new node_type(array[m]);
        node->set_left(build_tree(array, b, m));
        node->set_right(build_tree(array, m + 1, e));
        return node;
      }

      /**
       * @brief recursively removes subtree
       * @param node pointer to subtree to be removed
       **/
      void dispose_tree(node_type *node) {
        if (node == NULL) {
          return;
        }
        dispose_tree(node->left());
        dispose_tree(node->right());
        delete node;
      }

      /**
       * @brief find n-th element in tree (counting from zero)
       * @param i number of elements smaller than element to be returned
       * @returns pointer to found node or NULL if doesn't exist
       **/
      node_type *find(size_t i) const {
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

      /**
       * @brief SplayTree stream output operator
       * @param stream output stream
       * @param tree splay tree
       **/
      template<typename S>
      friend S& operator<<(S &stream, SplayTree<T, SplayImpl> &tree) {
        tree.root_->print_tree(stream);
        return stream;
      }

      /** root node of a tree */
      node_type *root_ = NULL;
  };
}

#endif  // SPLAY_SPLAYTREE_H_
