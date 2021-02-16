#ifndef __TINY_TREE_H
#define __TINY_TREE_H

#include <iterator>
#include <memory>
#include "tiny_alloc.h"
#include "tiny_construct.h"

typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false;   // 红色为0
const __rb_tree_color_type __rb_tree_black = true;      // 黑色为1

struct __rb_tree_node_base
{
    typedef __rb_tree_color_type color_type;
    typedef __rb_tree_node_base *base_ptr;

    color_type color;   // 节点颜色，非红即黑
    base_ptr parent;    // RB树的许多操作，必须知道父节点
    base_ptr left;      // 指向左节点
    base_ptr right;     // 指向右节点

    static base_ptr minimum(base_ptr x) {
        while (x->left != 0)
            x = x->left;        // 一直向左走，就会找到最小值
        return x;           // 这是二叉搜索树的特性
    }

    static base_ptr maximum(base_ptr x) {
        while (x->right != 0)
            x = x->right;       // 一直向右走，就会找到最大值
        return x;           // 这是二叉搜索树的特性
    }
};

template <class Value>
struct __rb_tree_node : public __rb_tree_node_base
{
    typedef __rb_tree_node<Value> *link_type;
    Value value_filed;      // 节点值
};

// 基层迭代器
struct __rb_tree_base_iterator
{
    typedef __rb_tree_node_base::base_ptr base_ptr;
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;
    base_ptr node;      // 它用来与容器之间产生一个连结关系

    // 以下其实可实现于operator++内，因为再无他处会调用此函数
    void increment() {
        if (node->right != 0) {     // 如果有右子节点
            node = node->right;     // 就向右走
            while (node->left != 0) // 然后一直往左子树走到底
                node = node->left;  // 即是解答
        }
        else {                              // 没有右子节点
            base_ptr y = node->parent;      // 找出父节点
            while (node == y->right) {      // 如果现行节点本身是个右子节点
                node = y;                   // 就一直上溯，直到不为右子节点止
                y = y->parent;
            }
            if (node->right != y)           // 若此时的右子节点不等于此时的父节点
                node = y;                   // 此时的父节点即为解答
                                            // 否则此时的node为解答
        }
        // 以上判断若此时的右子节点不等于此时的父节点，是为了应付一种特殊情况
        // 我们欲寻找根节点的下一节点，而恰巧根节点无右子节点
        // 当然，以上特殊做法必须配合RB-tree根节点与特殊之header之间的特殊关系
    }

    // 以下其实可实现operator--内，因为再无他处会调用此函数
    void decrement() {
        if (node->color == __rb_tree_red &&     // 如果是红节点，且
            node->parent->parent == node)       // 父节点的父节点等于自己
            node = node->right;                 // 右子节点即为解答
        // 以上情况发生于node为header时(亦即node为end()时)
        // header之右子节点即mostright，指向整颗树的max节点
        else if (node->left != 0) {             // 如果有左子节点
            base_ptr y = node->left;            // 令y指向左子节点
            while (y->right == 0)               // 令y有右子节点时
                y = y->right;                   // 一直往右子节点走到底
            node = y;                           // 最后即为答案
        }
        else {                                  // 即非根节点，亦无左子节点
            base_ptr y = node->parent;          // 找出父节点
            while (node == y->left) {           // 当现行节点身为左子节点
                node = y;                       // 一直交替往上走，直到现行节点
                y = y->parent;                  // 不为左子节点
            }
            node = y;                           // 此时之父节点即为答案
        }
    }
};

template <class Value,class Ref,class Ptr>
struct __rb_tree_iterator : public __rb_tree_base_iterator
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef __rb_tree_iterator<Value, Value, Value *> iterator;
    typedef __rb_tree_iterator<Value, const Value &, const Value *> const_iterator;
    typedef __rb_tree_iterator<Value, Ref, Ptr> self;
    typedef __rb_tree_node<Value> *link_type;

    __rb_tree_iterator() {}
    __rb_tree_iterator(link_type x) { node = x; }
    __rb_tree_iterator(const iterator &it) { node = it.node; }

    reference operator*() const { return link_type(node)->value_filed; }
    pointer operator->() const { return &(operator*()); }

    self &operator++() { 
        increment();
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        increment();
        return tmp;
    }

    self &operator--() { 
        decrement();
        return *this;
    }
    self operator--(int) {
        self tmp = *this;
        decrement();
        return tmp;
    }

};

inline bool operator==(const __rb_tree_base_iterator& x, const __rb_tree_base_iterator& y) {
  return x.node == y.node;
}

inline bool operator!=(const __rb_tree_base_iterator& x, const __rb_tree_base_iterator& y) {
  return x.node != y.node;
}

inline bidirectional_iterator_tag
iterator_category(const __rb_tree_base_iterator&) {
  return bidirectional_iterator_tag();
}

inline __rb_tree_base_iterator::difference_type*
distance_type(const __rb_tree_base_iterator&) {
  return (__rb_tree_base_iterator::difference_type*) 0;
}

template <class Value, class Ref, class Ptr>
inline Value* value_type(const __rb_tree_iterator<Value, Ref, Ptr>&) {
  return (Value*) 0;
}

template <class Key,class Value,class KeyOfValue,class Compare>
class rb_tree {
    protected:
        typedef void *void_pointer;
        typedef __rb_tree_node_base *base_ptr;
        typedef __rb_tree_node<Value> rb_tree_node;
        typedef simple_alloc<rb_tree_node> rb_tree_node_allocator;
        typedef __rb_tree_color_type color_type;
    
    public:
        typedef Key key_type;
        typedef Value value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef rb_tree_node *link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

    protected:
        link_type get_node() { return rb_tree_node_allocator::allocate(); }
        void put_node(link_type p) { rb_tree_node_allocator::deallocate(p); }

        link_type create_node(const value_type& x) {
            link_type tmp = get_node();            // 配置空间
            try {
                Construct(&tmp->value_filed, x);    // 构造内容
            }
            catch(const std::exception& e) {
                put_node(tmp);
            }
            return tmp;
        }

        link_type clone_node(link_type x) {         // 复制一个节点
            link_type tmp = create_node(x->value_filed);
            tmp->color = x->color;
            tmp->left = 0;
            tmp->right = 0;
            return tmp;
        }

        void destroy_node(link_type p) {
            Destroy(&p->value_filed);       // 析构内容
            put_node(p);                    // 释放内容
        }
    
    protected:
        // RB-Tree只以三笔数据表现
        size_type node_count;   // 追踪记录树的大小
        link_type header;       // 这是实现上的一个技巧
        Compare key_compare;    // 节点间的键值大小比较准则

        // 以下三个函数用来方便取得header的成员
        link_type &root() const { return (link_type &)header->parent; }
        link_type &leftmost() const { return (link_type &)header->left; }
        link_type &rightmost() const { return (link_type &)header->right; }

        // 以下六个函数用来方便取得节点x的成员
        static link_type& left(link_type x) {
            return (link_type &)(x->left);
        }
        static link_type& right(link_type x) {
            return (link_type &)(x->right);
        }
        static link_type& parent(link_type x) {
            return (link_type &)(x->parent);
        }
        static reference value(link_type x) {
            return x->value_filed;
        }
        static const Key& key(link_type x) {
            return KeyOfValue()(value(x));
        }
        static color_type& color(link_type x) {
            return (color_type &)(x->color);
        }

        // 以下六个函数用来方便取得节点x的成员
        static link_type& left(base_ptr x) {
            return (link_type &)(x->left);
        }
        static link_type& right(base_ptr x) {
            return (link_type &)(x->right);
        }
        static link_type& parent(base_ptr x) {
            return (link_type &)(x->parent);
        }
        static reference value(base_ptr x) {
            return ((link_type)x)->value_filed;
        }
        static const Key& key(base_ptr x) {
            return KeyOfValue()(value(link_type(x)));
        }
        static color_type& color(base_ptr x) {
            return (color_type &)((link_type)x->color);
        }

        // 求取极大值和极小值，node class有实现此功能，交给它们完成即可
        static link_type minimum(link_type x) {
            return (link_type)__rb_tree_node_base::minimum(x);
        }
        static link_type maximum(link_type x) {
            return (link_type)__rb_tree_node_base::maximum(x);
        }

    public:
        typedef __rb_tree_iterator<value_type, reference, pointer> iterator;
        typedef __rb_tree_iterator<value_type, const_reference, const_pointer> const_iterator;

        typedef reverse_iterator<const_iterator> const_reverse_iterator;
        typedef reverse_iterator<iterator> reverse_iterator;

    private:
        iterator __insert(base_ptr x, base_ptr y, const value_type &v);
        link_type __copy(link_type x, link_type p);
        void __erase(link_type x);
        void init() {
            header = get_node();            // 产生一个节点空间，令header指向它
            color(header) = __rb_tree_red;  // 令header为红色，用来区分header
                                            // 和root，在iterator.operator--之中
            root() = 0;
            leftmost() = header;            // 令header的左子节点为自己
            rightmost() = header;           // 令header的右子节点为自己
        }

    public:
        // allocation/deallocation
        rb_tree(const Compare &comp = Compare()) : node_count(0), key_compare(comp) { init(); }

        ~rb_tree() {
            clear();
            put_node(header);
        }
        rb_tree<Key, Value, KeyOfValue, Compare> &operator=(const rb_tree<Key, Value, KeyOfValue, Compare> &x);

    public:
        // accessors
        Compare key_comp() const { return key_compare; }
        iterator begin() { return leftmost(); }     // RB树的起头为最左(最小)节点处
        const_iterator begin() const { return leftmost(); }

        iterator end() { return header; }           // RB树的终点为header所指处
        const_iterator end() const { return header; }

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

        bool empty() const { return node_count; }
        size_type size() const { return node_count; }
        size_type max_size() const { return size_type(-1); }

        void swap(rb_tree<Key, Value, KeyOfValue, Compare> &t) {
            std::swap(header, t.header);
            std::swap(node_count, t.node_count);
            std::swap(key_compare, t.key_compare);
        }

        void erase(iterator position);
        size_type erase(const key_type& x);
        void erase(iterator first, iterator last);
        void erase(const key_type* first, const key_type* last);
        void clear() {
            if (node_count != 0) {
                __erase(root());        // 调用内部的删除函数
                leftmost() = header;
                root() = 0;
                rightmost() = header;
                node_count = 0;
            }
        }

    public:
        // 将x插入到RB-tree中(保持节点值独一无二)
        pair<iterator, bool> insert_unique(const value_type &x);
        // 将x插入到RB-tree中(允许节点值重复)
        iterator insert_equal(const value_type &x);
        
        // 将x插入到RB-tree中position位置(保持节点值独一无二)
        iterator insert_unique(iterator position, const value_type& x);
        // 将x插入到RB-tree中position位置(允许节点值重复)
        iterator insert_equal(iterator position, const value_type& x);
        template <class InputIterator>

        // set需要的插入操作
        void insert_unique(InputIterator first, InputIterator last);
        template <class InputIterator>
        void insert_equal(InputIterator first, InputIterator last);

    public:
        // set操作
        iterator find(const key_type& x);
        const_iterator find(const key_type& x) const;
        size_type count(const key_type& x) const;
        iterator lower_bound(const key_type& x);
        const_iterator lower_bound(const key_type& x) const;
        iterator upper_bound(const key_type& x);
        const_iterator upper_bound(const key_type& x) const;
        pair<iterator,iterator> equal_range(const key_type& x);
        pair<const_iterator, const_iterator> equal_range(const key_type& x) const;

};

template <class Key, class Value, class KeyOfValue, class Compare>
rb_tree<Key,Value,KeyOfValue,Compare>& 
rb_tree<Key,Value,KeyOfValue,Compare>::operator=(const rb_tree<Key,Value,KeyOfValue,Compare>& x)
{
    if (this != &x) {
                                    // Note that _Key may be a constant type.
        clear();
        node_count = 0;
        key_compare = x.key_compare;        
        if (x.root() == 0) {
            root() = 0;
            leftmost() = header;
            rightmost() = header;
        }
        else {
            root() = copy(x.root(), header);
            leftmost() = minimum(root());
            rightmost() = maximum(root());
            node_count = x.node_count;
        }
    }
    return *this;
}

// 插入新值：节点键值允许重复
// 注意，返回值是一个RB-tree迭代器，指向新增节点
template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator
rb_tree<Key,Value,KeyOfValue,Compare>::insert_equal(const Value& v)
{
    link_type y = header;
    link_type x = root();       // 从根节点开始
    while (x != 0) {            // 从根节点开始，往下寻找适当的插入点
        y = x;
        x = key_compare(KeyOfValue()(v), key(x)) ? left(x) : right(x);
        // 以上，遇大则往左，遇小则往右
    }
    return __insert(x, y, v);
    // 以上，x为新值插入点，y为插入点之父节点，v为新值
}

// 插入新值：节点键值不允许重复，若重复则插入无效
// 注意，返回值是个pair，第一个元素是个RB-tree迭代器，指向新增节点
// 第二叉素表示插入成功与否
template <class Key,class Value,class KeyOfValue,class Compare>
pair<typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator,bool>
rb_tree<Key,Value,KeyOfValue,Compare>::insert_unique(const Value& v) {
    link_type y = header;
    link_type x = root();       // 从根结点开始
    bool comp = true;
    while (x != 0) {              // 从根结点开始，往下寻找适当的插入点
        y = x;
        comp = key_compare(KeyOfValue()(v), key(x));    // v键值小于目前节点之键值
        x = comp ? left(x) : right(x);      // 遇大则往左，遇小于或等于则往右
    }
    // 离开while循环之后，y所指即插入点之父节点(此时的它必为叶节点)

    iterator j = iterator(y);   // 令迭代器j指向插入点之父节点y
    if(comp){
        // 如果离开while循环时comp为真(表示遇大，将插入于左侧)
        if (j == begin())       // 如果插入点之父节点为最左结点
            return pair<iterator, bool>(__insert(x, y, v), true);
            // 以上，x为插入点，y为插入点之父节点，v为新值
        else        // 否则(插入点之父节点不为最左节点)
            --j;    // 调整j
    }
    if(key_compare(key(j.node),KeyOfValue()(v)))
        // 新键值不与既有节点之键值重复，于是执行安插操作
        return pair<iterator, bool>(__insert(x, y, v), true);
        // 以上，x为新值插入点，y为插入点之父节点，v为新值
    // 进行至此，表示新值一定与树中键值重复，那么就不插入新值
    return pair<iterator, bool>(j, false);
}

template <class Key,class Value,class KeyOfValue,class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator
rb_tree<Key,Value,KeyOfValue,Compare>::
__insert(base_ptr x_,base_ptr y_,const Value& v) {
    // 参数x_为新值插入点，参数y_为插入点之父节点，参数v为新值
    link_type x = (link_type)x_;
    link_type y = (link_type)y_;
    link_type z;

    // key_compare是键值大小比较准则
    if(y == header || x != 0 || key_compare(KeyOfValue()(v),key(y))) {
        z = create_node(v);     // 产生一个新节点
        left(y) = z;            // 这使得当y即为header时，leftmost()=z
        if (y == header) {
            root() = z;
            rightmost() = z;
        }
        else if (y == leftmost())   // 如果y为最左节点
            leftmost() = z;         // 维护leftmost()，使它永远指向最左节点
        }
    else {
        z = create_node(v);         // 产生一个新节点
        right(y) = z;               // 令新节点成为插入节点之父节点y的右子节点
        if (y == rightmost())
            rightmost() = z;        // 维护rightmost(),使它永远指向最右节点
    }
    parent(z) = y;                  // 设定新节点的父节点
    left(z) = 0;                    // 设定新节点的左子节点
    right(z) = 0;                   // 设定新节点的右子节点
                                    // 新节点的颜色将在__rb_tree_rebalance()设定
    __rb_tree_rebalance(z, header->parent);     // 参数一为新增节点，参数二为root
    ++node_count;                   // 节点数累加
    return iterator(z);             // 返回一个迭代器，指向新增节点
}

// 全局函数
// 新节点必为红节点，如果插入之处父节点亦为红节点，就违反红黑树规则
// 此时可能需做树形旋转
inline void
__rb_tree_rotate_left(__rb_tree_node_base* x, __rb_tree_node_base*& root) {
    // x为旋转点
    __rb_tree_node_base *y = x->right;  // 令y为旋转点的右子节点
    x->right = y->left;
    if (y->left != 0)
        y->left->parent = x;        // 设定父节点
    y->parent = x->parent;

    // 令y完全顶替x的地位
    if(x == root)                       // x为根节点
        root = y;
    else if(x == x->parent->left)       // x为其父节点的左子节点
        x->parent->left = y;
    else                                // x为其父节点的右子节点
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

// 全局函数
// 新节点必为红节点，如果插入之处父节点亦为红节点，就违反红黑树规则
// 此时可能需做树形旋转
inline void
__rb_tree_rotate_right(__rb_tree_node_base* x, __rb_tree_node_base*& root) {
     // x为旋转点
    __rb_tree_node_base *y = x->left;  // 令y为旋转点的右子节点
    x->left = y->right;
    if (y->right != 0)
        y->right->parent = x;        // 设定父节点
    y->parent = x->parent;

    // 令y完全顶替x的地位
    if(x == root)                       // x为根节点
        root = y;
    else if(x == x->parent->right)       // x为其父节点的左子节点
        x->parent->right = y;
    else                                // x为其父节点的右子节点
        x->parent->left = y;
    y->right = x;
    x->parent = y;
}

// 全局函数
// 重新令树形平衡(改变颜色及旋转树型)
// 参数一为新增节点，参数二为root
inline void
__rb_tree_rebalance(__rb_tree_node_base* x,__rb_tree_node_base*& root) {
    x->color = __rb_tree_red;       // 新节点必为红
    while (x != root && x->parent->color == __rb_tree_red) {    // 父节点为红
        if (x->parent == x->parent->parent->left) {             // 父节点为祖父节点之左子节点
            __rb_tree_node_base *y = x->parent->parent->right;  // 令y为伯父节点
            if (y && y->color == __rb_tree_red) {               // 伯父节点存在，且为红
                x->parent->color = __rb_tree_black;             // 更改父节点为黑
                y->color = __rb_tree_black;                     // 更改伯父节点为黑
                x->parent->parent->color = __rb_tree_red;       // 更改祖父节点为红
                x = x->parent->parent;
            }
            else {      // 无伯父节点，或伯父节点为黑
                if(x == x->parent->right) {                     // 如果新节点为父节点之左子节点
                    x = x->parent;
                    __rb_tree_rotate_left(x, root);             // 第一参数为左旋转
                }
                x->parent->color = __rb_tree_black;             // 改变颜色
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_right(x->parent->parent, root); // 第一参数为右旋转
            }
        }
        else {      // 父节点为祖父节点之右子节点
            __rb_tree_node_base *y = x->parent->parent->left;     // 令y为伯父节点
            if (y && y->color == __rb_tree_red) {               // 有伯父节点，且为红
                x->parent->color = __rb_tree_black;            // 更改父节点为黑
                y->color = __rb_tree_black;                     // 更改祖父节点为红
                x->parent->parent->color = __rb_tree_red;
                x = x->parent->parent;      // 准备继续往上层检查
            }
            else {      // 无伯父节点，或伯父节点为黑
                if (x==x->parent->left) {           // 如果新节点为父节点之左子节点
                    x = x->parent;
                    __rb_tree_rotate_right(x, root);    // 第一参数为右旋转
                }
                x->parent->color = __rb_tree_black;     // 改变颜色
                x->parent->parent->color = __rb_tree_red;
                __rb_tree_rotate_left(x->parent->parent, root);     // 第一参数为左旋转
            }
        }
    }
    root->color = __rb_tree_black;      // 根节点永远为黑
}


inline __rb_tree_node_base *
rb_tree_rebalance_for_erase(__rb_tree_node_base *z, __rb_tree_node_base *&root,
                            __rb_tree_node_base *&leftmost, __rb_tree_node_base *&rightmost)
{
    __rb_tree_node_base* y = z;
    __rb_tree_node_base* x = 0;
    __rb_tree_node_base* x_parent = 0;
    if (y->left == 0)     // __z has at most one non-null child. y == z.
        x = y->right;     // __x might be null.
    else {
        if (y->right == 0)  // __z has exactly one non-null child. y == z.
            x = y->left;    // __x is not null.
        else {                   // __z has two non-null children.  Set __y to
            y = y->right;   //   __z's successor.  __x might be null.
            while (y->left != 0)
                y = y->left;
            x = y->right;
        }
    }
    if (y != z) {          // relink y in place of z.  y is z's successor
        z->left->parent = y; 
        y->left = z->left;
        if (y != z->right) {
            x_parent = y->parent;
            if (x) x->parent = y->parent;
            y->parent->left = x;      // __y must be a child of _M_left
            y->right = z->right;
            z->right->parent = y;
        }
        else
            x_parent = y;  
        if (root == z)
            root = y;
        else if (z->parent->left == z)
            z->parent->left = y;
        else 
            z->parent->right = y;
        y->parent = z->parent;
        std::swap(y->color, z->color);
        y = z;
        // __y now points to node to be actually deleted
    }
    else {                        // __y == __z
        x_parent = y->parent;
        if (x) 
            x->parent = y->parent;   
        if (root == z)
            root = x;
        else 
            if (z->parent->left == z)
                z->parent->left = x;
        else
            z->parent->right = x;
        if (leftmost == z) {
            if (z->right == 0)        // __z->_M_left must be null also
                leftmost = z->parent;
        }
        // makes __leftmost == _M_header if __z == __root
        else
            leftmost = __rb_tree_node_base::minimum(x);
        if (rightmost == z)  {
            if (z->left == 0)         // __z->_M_right must be null also
                rightmost = z->parent;  
        }
        // makes __rightmost == _M_header if __z == __root
        else                      // __x == __z->_M_left
            rightmost = __rb_tree_node_base::maximum(x);
    }
    if (y->color != __rb_tree_red) { 
        while (x != root && (x == 0 || x->color == __rb_tree_black))
        if (x == x_parent->left) {
            __rb_tree_node_base* w = x_parent->right;
            if (w->color == __rb_tree_red) {
                w->color = __rb_tree_black;
                x_parent->color = __rb_tree_red;
                __rb_tree_rotate_left(x_parent, root);
                w = x_parent->right;
            }
            if ((w->left == 0 || 
                w->left->color == __rb_tree_black) &&
                (w->right == 0 || w->right->color == __rb_tree_black)) {
                w->color = __rb_tree_red;
                x = x_parent;
                x_parent = x_parent->parent;
            } 
            else {
                if (w->right == 0 || 
                    w->right->color == __rb_tree_black) {
                    if (w->left) w->left->color = __rb_tree_black;
                    w->color = __rb_tree_red;
                    __rb_tree_rotate_right(w, root);
                    w = x_parent->right;
                }
                w->color = x_parent->color;
                x_parent->color = __rb_tree_black;
                if (w->right) w->right->color = __rb_tree_black;
                __rb_tree_rotate_left(x_parent, root);
                break;
            }
        } 
        else {                  // same as above, with _M_right <-> _M_left.
            __rb_tree_node_base* w = x_parent->left;
            if (w->color == __rb_tree_red) {
                w->color = __rb_tree_black;
                x_parent->color = __rb_tree_red;
                __rb_tree_rotate_right(x_parent, root);
                w = x_parent->left;
            }
            if ((w->right == 0 || 
                w->right->color == __rb_tree_black) &&
                (w->left == 0 || 
                w->left->color == __rb_tree_black)) {
                w->color = __rb_tree_red;
                x = x_parent;
                x_parent = x_parent->parent;
            } 
            else {
                if (w->left == 0 || 
                    w->left->color == __rb_tree_black) {
                    if (w->right) w->right->color = __rb_tree_black;
                    w->color = __rb_tree_red;
                    __rb_tree_rotate_left(w, root);
                    w = x_parent->left;
                }
                w->color = x_parent->color;
                x_parent->color = __rb_tree_black;
                if (w->left) w->left->color = __rb_tree_black;
                __rb_tree_rotate_right(x_parent, root);
                break;
            }
        }
        if (x) x->color = __rb_tree_black;
    }
    return y;
}


// 内部的删除函数
template <class Key, class Value, class KeyOfValue, class Compare>
void rb_tree<Key,Value,KeyOfValue,Compare>::__erase(link_type x)
{
    while (x != 0) {
        __erase(right(x));
        link_type y = left(x);
        destroy_node(x);
        x = y;
    }
}

template <class Key, class Value, class KeyOfValue, class Compare>
inline void rb_tree<Key,Value,KeyOfValue,Compare>::erase(iterator position)
{
    link_type y =
        (link_type)rb_tree_rebalance_for_erase(position.node, header->parent,
                                               header->left, header->right);
    destroy_node(y);
    --node_count;
}

// 删除键值为key的节点
template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::size_type 
rb_tree<Key,Value,KeyOfValue,Compare>::erase(const Key& x)
{
  pair<iterator,iterator> p = equal_range(x);
  size_type n = 0;
  n = distance(p.first, p.second);
  erase(p.first, p.second);
  return n;
}

// 删除指针范围内的节点
template <class Key, class Value, class KeyOfValue, class Compare>
void rb_tree<Key,Value,KeyOfValue,Compare>::erase(iterator first, iterator last) {
    if (first == begin() && last == end())
        clear();
    else
        while (first != last) erase(first++);
}

// 删除一定数值范围内的节点
template <class Key, class Value, class KeyOfValue, class Compare>
void rb_tree<Key,Value,KeyOfValue,Compare>::erase(const Key* first, const Key* last) {
    while (first != last) erase(*first++);
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key, Value, KeyOfValue, Compare>::iterator 
rb_tree<Key, Value, KeyOfValue, Compare>::insert_unique(iterator position, const Value& v)
{
    if (position.node == header->left) { // begin()
        if (size() > 0 && 
            key_compare(KeyOfValue()(v), key(position.node)))
        return __insert(position.node, position.node, v);
        // first argument just needs to be non-null 
        else
            return insert_unique(v).first;
    } else if (position.node == header) { // end()
        if (key_compare(key(rightmost()), KeyOfValue()(v)))
            return __insert(0, rightmost(), v);
        else
        return insert_unique(v).first;
    } else {
        iterator before = position;
        --before;
        if (key_compare(key(before.node), KeyOfValue()(v)) 
            && key_compare(KeyOfValue()(v), key(position.node))) {
        if (right(before.node) == 0)
            return __insert(0, before.node, v); 
        else
            return __insert(position.node, position.node, v);
        // first argument just needs to be non-null 
        } else
        return insert_unique(v).first;
    }
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::insert_equal(iterator position, const Value& v)
{
    if (position.node == header->left) { // begin()
        if (size() > 0 && 
            !key_compare(key(position.node), KeyOfValue()(v)))
        return __insert(position.node, position.node, v);
        // first argument just needs to be non-null 
        else
        return insert_equal(v);
    } else if (position.node == header) {// end()
        if (!key_compare(KeyOfValue()(v), key(rightmost())))
        return __insert(0, rightmost(), v);
        else
        return insert_equal(v);
    } else {
        iterator before = position;
        --before;
        if (!key_compare(KeyOfValue()(v), key(before.node))
            && !key_compare(key(position.node), KeyOfValue()(v))) {
            if (right(before.node) == 0)
                return __insert(0, before.node, v); 
            else
                return __insert(position.node, position.node, v);
            // first argument just needs to be non-null 
        } 
        else
            return insert_equal(v);
    }
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::find(const Key& k)
{
    link_type y = header;
    link_type x = root();

    while (x != 0)
        // 以下，key_compare是节点键值大小比较准则
        if (!key_compare(key(x), k))
            // 进行到这里，表示x键值大于k,遇到大值就向左走
            y = x, x = left(x);
        else
            // 进行到这里，表示x键值小于k,遇到小值就向右走
            x = right(x);

    iterator j = iterator(y);
    return (j == end() || key_compare(k, key(j.node))) ? end() : j;
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::const_iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::find(const Key& k) const
{
    link_type y = header; /* Last node which is not less than __k. */
    link_type x = root(); /* Current node. */

    while (x != 0) {
        if (!key_compare(key(x), k))
        y = x, x = left(x);
        else
        x = right(x);
    }
    const_iterator j = const_iterator(y);
    return (j == end() || key_compare(k, key(j.node))) ? end() : j;
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::size_type 
rb_tree<Key,Value,KeyOfValue,Compare>::count(const Key& k) const
{
    pair<const_iterator, const_iterator> p = equal_range(k);
    size_type n = 0;
    n = distance(p.first, p.second);
    return n;
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::lower_bound(const Key& k)
{
    link_type y = header; /* Last node which is not less than __k. */
    link_type x = root(); /* Current node. */

    while (x != 0) 
        if (!key_compare(key(x), k))
            y = x, x = left(x);
        else
            x = right(x);

    return iterator(y);
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::const_iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::lower_bound(const Key& k) const
{
    link_type y = header; /* Last node which is not less than __k. */
    link_type x = root(); /* Current node. */

    while (x != 0) 
        if (!key_compare(key(x), k))
            y = x, x = left(x);
        else
            x = right(x);

    return const_iterator(y);
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::upper_bound(const Key& k)
{
    link_type y = header; /* Last node which is greater than __k. */
    link_type x = root(); /* Current node. */

    while (x != 0) 
        if (key_compare(k, key(x)))
            y = x, x = left(x);
        else
            x = right(x);

    return iterator(y);
}

template <class Key, class Value, class KeyOfValue, class Compare>
typename rb_tree<Key,Value,KeyOfValue,Compare>::const_iterator 
rb_tree<Key,Value,KeyOfValue,Compare>::upper_bound(const Key& k) const
{
    link_type y = header; /* Last node which is greater than __k. */
    link_type x = root(); /* Current node. */

    while (x != 0) 
        if (key_compare(k, key(x)))
            y = x, x = left(x);
        else
            x = right(x);

    return const_iterator(y);
}

template <class Key, class Value, class KeyOfValue, class Compare>
inline 
pair<typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator,
     typename rb_tree<Key,Value,KeyOfValue,Compare>::iterator>
rb_tree<Key,Value,KeyOfValue,Compare>::equal_range(const Key& k)
{
    return pair<iterator, iterator>(lower_bound(k), upper_bound(k));
}

template <class Key, class Value, class KeyOfValue, class Compare>
inline 
pair<typename rb_tree<Key, Value, KeyOfValue, Compare>::const_iterator,
     typename rb_tree<Key, Value, KeyOfValue, Compare>::const_iterator>
rb_tree<Key, Value, KeyOfValue, Compare>::equal_range(const Key& k) const
{
    return pair<const_iterator, const_iterator>(lower_bound(k), upper_bound(k));
}

template <class Key, class Value, class KeyOfValue, class Compare>
template <class InputIterator>
void rb_tree<Key, Value, KeyOfValue, Compare>::insert_equal(InputIterator first, InputIterator last) {
  for ( ; first != last; ++first)
    insert_equal(*first);
}

template <class Key, class Value, class KeyOfValue, class Compare>
template <class InputIterator>
void rb_tree<Key, Value, KeyOfValue, Compare>::insert_unique(InputIterator first, InputIterator last) {
  for ( ; first != last; ++first)
    insert_unique(*first);
}

#endif