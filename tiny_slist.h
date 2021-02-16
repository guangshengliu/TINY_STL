#ifndef __TINY_SLIST_H
#define __TINY_SLIST_H

#include <iterator>
#include "tiny_alloc.h"
#include "tiny_construct.h"

struct __slist_node_base
{
    __slist_node_base *next;
};

template <class T>
struct __slist_node : public __slist_node_base
{
    T data;
};

// 全局函数，已知某一节点，插入新节点于其后
inline __slist_node_base* __slist_make_link (__slist_node_base *prev_node, __slist_node_base *new_node) {
    // 令new节点的下一节点为prev节点的下一节点
    new_node->next = prev_node->next;
    prev_node->next = new_node;     // 令prev节点的下一节点指向new节点
    return new_node;
}

// 全局函数，单向链表的大小(元素个数)
inline size_t __slist_size(__slist_node_base* node) {
    size_t result = 0;
    for (; node != 0; node = node->next)
        ++result;
    return result;
}

// 全局函数，寻找前继结点
inline __slist_node_base*
    __slist_previous(__slist_node_base* head, const __slist_node_base* node) {
    while (head && head->next != node)
        head = head->next;
    return head;
}
// 全局函数，寻找前继结点
inline const __slist_node_base*
__slist_previous(const __slist_node_base* head, const __slist_node_base* node) {
    while (head && head->next != node)
        head = head->next;
    return head;
}


// 全局函数，在pos位置后拼接形成新的链表
inline void __slist_splice_after(__slist_node_base* pos,
                                 __slist_node_base* before_first,
                                 __slist_node_base* before_last)
{
    if (pos != before_first && pos != before_last) {
        __slist_node_base* first = before_first->next;
        __slist_node_base* after = pos->next;
        before_first->next = before_last->next;
        pos->next = first;
        before_last->next = after;
    }
}
inline void
__slist_splice_after(__slist_node_base* pos, __slist_node_base* head)
{
    __slist_node_base* before_last = __slist_previous(head, 0);
    if (before_last != head) {
        __slist_node_base* after = pos->next;
        pos->next = head->next;
        head->next = 0;
        before_last->next = after;
    }
}

// 单向链表的迭代器基本结构
struct __slist_iterator_base
{
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef forward_iterator_tag iterator_category;     // 注意单向

    __slist_node_base *node;    // 指向节点基本结构

    __slist_iterator_base(__slist_node_base* x) : node(x) {}


    void incr() { node = node->next; }      // 前进一个节点

    bool operator==(const __slist_iterator_base& x) const {
        return node == x.node;
    }

    bool operator!=(const __slist_iterator_base& x) const {
        return node != x.node;
    }

};

// 单向链表的迭代器结构
template <class T,class Ref,class Ptr>
struct __slist_iterator : public __slist_iterator_base
{
    typedef __slist_iterator<T, T &, T *> iterator;
    typedef __slist_iterator<T, const T &, const T *> const_iterator;
    typedef __slist_iterator<T, Ref, Ptr> self;

    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef __slist_node<T> list_node;

    __slist_iterator(list_node* x) : __slist_iterator_base(x) {}
    // 调用是list<T>::end()时会造成__slist_iterator(0),于是调用上述函数
    __slist_iterator() : __slist_iterator_base(0) {}
    __slist_iterator(const iterator& x) : __slist_iterator_base(x.node) {}

    reference operator*() const { return ((list_node *)node)->data; }
    pointer operator->() const { return &(operator*()); }

    self& operator++() {
        incr();     // 前进一个节点
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        incr();
        return tmp;
    }
};

inline ptrdiff_t* distance_type(const __slist_iterator_base&) {
  return 0;
}

inline forward_iterator_tag iterator_category(const __slist_iterator_base&) {
  return forward_iterator_tag();
}

template <class T, class Ref, class Ptr> 
inline T* value_type(const __slist_iterator<T, Ref, Ptr>&) {
  return 0;
}

template <class T>
class slist
{
    public:
        typedef T value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        typedef __slist_iterator<T, T &, T *> iterator;
        typedef __slist_iterator<T, const T &, const T *> const_iterator;
    
    private:
        typedef __slist_node<T> list_node;
        typedef __slist_node_base list_node_base;
        typedef __slist_iterator_base iterator_base;

    list_node* create_node(const value_type& x) {
        list_node *node = this->get_node(); // 配置空间
        try {
            Construct(&node->data, x);      // 构造元素
            node->next = 0;
        }
        catch(const std::exception& e) {
            put_node(node);     // 分配失败释放空间
        }
        return node;
    }

    list_node* create_node() {
        list_node *node = this->get_node();     // 配置空间
        try {
            Construct(&node->data);      // 构造元素
            node->next = 0;
        }
        catch(const std::exception& e) {
            put_node(node);     // 分配失败释放空间
        }
        return node;
    }

    static void destory_node(list_node* node) {
        Destroy(&node->data);       // 将元素析构
        put_node(node);      // 释放空间
    }

    private:
        list_node_base head;    // 头部
    

    public:
        slist() { head.next = 0; }
        slist(const_iterator first, const_iterator last) { insert_after_range(&this->head, first, last); }
        slist(const value_type* first, const value_type* last) { insert_after_range(&this->head, first, last); }
        slist(const slist& x) { insert_after_range(&this->head, x.begin(), x.end()); }

        slist& operator= (const slist& x);

        ~slist() { clear(); }
    public:
        iterator begin() { return iterator((list_node *)head.next); }
        const_iterator begin() const { return const_iterator((list_node *)head.next);}

        iterator end() { return iterator(0); }
        const_iterator end() const { return const_iterator(0); }

        size_type size() const { return __slist_size(head.next); }
        size_type max_size() const { return size_type(-1); }

        bool empty() const { return head.next == 0; }

        void clear() { this->erase_after(&this->head, 0); }

        // 两个slist互换：只要将head交换互指即可
        void swap(slist& L)
        {
            list_node_base *tmp = head.next;
            head.next = L.head.next;
            L.head.next = tmp;
        }

    protected:
        typedef simple_alloc<list_node> list_node_allocator;
        __slist_node<T>* get_node() { return list_node_allocator::allocate(1); }
        void put_node(__slist_node<T> *p) { list_node_allocator::deallocate(p, 1); }

        __slist_node_base *erase_after(__slist_node_base *before_first, __slist_node_base *last_node);

        __slist_node_base* erase_after(__slist_node_base* pos) {
            __slist_node<T>* __next = (__slist_node<T>*) (pos->next);
            __slist_node_base* __next_next = __next->next;
            pos->next = __next_next;
            Destroy(&__next->data);
            put_node(__next);
            return __next_next;
        }

    public:
        // 取头部元素
        reference front() { return ((list_node *)head.next)->data; }
        const_reference front() const { return ((list_node *)head.next)->data; }

        // 从头部插入元素
        void push_front(const value_type& x) {
            __slist_make_link(&head, create_node(x));
        }

        // 从头部取走元素，删除之
        void pop_front() {
            list_node *node = (list_node *)head.next;
            head.next = node->next;
            destory_node(node);
        }

        // 寻找前继结点
        iterator previous(const_iterator pos) {
            return iterator((list_node*) __slist_previous(&this->head, pos.node));
        }
        const_iterator previous(const_iterator pos) const {
            return const_iterator((list_node*) __slist_previous(&this->head, pos.node));
        }

        // 删除pos位置之后的元素
        // 如果需要在开始位置删除元素，请调用pop_front()
        iterator erase_after(iterator pos) {
            return iterator((list_node*) this->erase_after(pos.node));
        }
        iterator erase_after(iterator before_first, iterator last) {
            return iterator((list_node*) this->erase_after(before_first.node, last.node));
        }
        // 删除pos位置的元素
        iterator erase(iterator pos) {
            return (list_node*) this->erase_after(__slist_previous(&this->head, pos.node));
        }
        iterator erase(iterator first, iterator last) {
            return (list_node*) this->erase_after(
            __slist_previous(&this->head, first.node), last.node);
        }

        // 重新指定容器有效的元素个数
        void resize(size_type new_size, const T& x);
        void resize(size_type new_size) { resize(new_size, T()); }

    private:
        list_node* insert_after(__slist_node_base* pos, const value_type& x) {
            return (list_node*) (__slist_make_link(pos, create_node(x)));
        }

        list_node* insert_after(__slist_node_base* pos) {
            return (list_node*) (__slist_make_link(pos, create_node()));
        }

        void insert_after_fill(__slist_node_base* pos, size_type n, const value_type& x) {
            for (size_type i = 0; i < n; ++i)
            pos = __slist_make_link(pos, create_node(x));
        }

    public:
        // 在pos位置之后插入元素
        iterator insert_after(iterator pos, const value_type& x) {
            return iterator(insert_after(pos.node, x));
        }
        iterator insert_after(iterator pos) {
            return insert_after(pos, value_type());
        }
        void insert_after(iterator pos, size_type n, const value_type& x) {
            insert_after_fill(pos.node, n, x);
        }
        // 在pos位置插入元素
        // 如果需要在开始位置插入元素，请调用push_front()
        iterator insert(iterator pos, const value_type& x) {
            return iterator(insert_after(__slist_previous(&this->head, pos.node), x));
        }
        iterator insert(iterator pos) {
            return iterator(insert_after(__slist_previous(&this->head, pos.node), value_type()));
        }
        void insert(iterator pos, size_type n, const value_type& x) {
            insert_after_fill(__slist_previous(&this->head, pos.node), n, x);
        }
        // 合并链表
        void merge(slist<T> &x);
};

template <class T> 
__slist_node_base* slist<T>::erase_after(__slist_node_base* before_first, __slist_node_base* last_node) {
    __slist_node<T> *cur = (__slist_node<T> *)(before_first->next);
    while (cur != last_node)
    {
        __slist_node<T> *tmp = cur;
        cur = (__slist_node<T> *)cur->next;
        Destroy(&tmp->data);
        put_node(tmp);
    }
    before_first->next = last_node;
    return last_node;
}

template <class T>
void slist<T>::resize(size_type len, const T& x)
{
    __slist_node_base* cur = &this->head;
    while (cur->next != 0 && len > 0) {
        --len;
        cur = cur->next;
    }
    if (cur->next) 
        this->erase_after(cur, 0);
    else
        insert_after_fill(cur, len, x);
}

template <class T>
void slist<T>::merge(slist<T>& x)
{
    __slist_node_base* n1 = &this->head;
    while (n1->next && x.head.next) {
        if (((__slist_node<T>*) x.head.next)->data < ((__slist_node<T>*)n1->next)->data)
            __slist_splice_after(n1, &x.head, x.head.next);
        n1 = n1->next;
    }
    if (x.head.next) {
        n1->next = x.head.next;
        x.head.next = 0;
    }
}

#endif