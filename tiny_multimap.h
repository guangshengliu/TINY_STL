#ifndef __TINY_MULTIMAP_H
#define __TINY_MULTIMAP_H

#include "tiny_tree.h"
#include <functional>

template <class Key, class T, class Compare = less<Key>>
class multimap;

template <class Key, class T, class Compare>
inline bool operator==(const multimap<Key, T, Compare> &x, const multimap<Key, T, Compare> &y);

template <class Key, class T, class Compare>
inline bool operator<(const multimap<Key, T, Compare> &x, const multimap<Key, T, Compare> &y);

template <class Key, class T, class Compare>
class multimap {
    public:
        // typedfe:
        typedef Key key_type;       // 键值型别
        typedef T data_type;        // 数据(实值)型别
        typedef T mapped_type;
        typedef pair<const Key, T> value_type;      // 元素型别(键值/实值)
        typedef Compare key_compare;            // 键值比较函数

    // 以下定义一个functor，其作用就是调用元素比较函数
        class value_compare : public binary_function<value_type, value_type, bool> {
            friend class multimap<Key, T, Compare>;
            protected:
                Compare comp;
                value_compare(Compare c) : comp(c) {}
            public:
                bool operator()(const value_type& x,const value_type& y) const {
                    return comp(x.first, y.first);
                }
        };
    
    private:
        // 以下定义表述型别，以map元素型别的第一型别
        // 作为RB-tree节点的键值型别
        typedef rb_tree<key_type, value_type, _Select1st<value_type>, key_compare> rep_type;
        rep_type t;     // 以红黑树(RB-Tree)表现map
    public:
        typedef typename rep_type::pointer pointer;
        typedef typename rep_type::const_pointer const_pointer;
        typedef typename rep_type::reference reference;
        typedef typename rep_type::const_reference const_reference;
        typedef typename rep_type::iterator iterator;
        //  注意上一行，并不想set一样将iterator定义为RB-tree的const_iterator
        //  因为它允许用户通过其迭代器修改元素的实值(value)
        typedef typename rep_type::const_iterator const_iterator;
        typedef typename rep_type::reverse_iterator reverse_iterator;
        typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
        typedef typename rep_type::size_type size_type;
        typedef typename rep_type::difference_type difference_type;

        // allocation/deallocation
        // 注意，map一定使用底层RB-tree的insert_unique()而非insert_equal()
        // multimap才使用insert_equal()
        // 因为map不允许相同键值存在，multimap才允许相同键值存在

        multimap() : t(Compare()) {}
        explicit multimap(const Compare& comp) : t(comp) {}

        template <class InputIterator>
        multimap(InputIterator first, InputIterator last) : t(Compare()) {
            t.insert_equal(first, last);
        }

        template <class InputIterator>
        multimap(InputIterator first, InputIterator last, const Compare &comp)
            : t(comp) { t.insert_equal(first, last); }
        
        multimap(const multimap<Key,T,Compare>& x) : t(x.t) {}
        multimap<Key, T, Compare> &operator=(const multimap<Key, T, Compare> &x) {
            t = x.t;
            return *this;
        }

        // 以下所有的map操作行为，RB-tree都已经提供，map只需转调用即可

        key_compare key_comp() const { return t.key_comp(); }
        value_compare value_comp() const { return value_compare(t.key_comp()); }
        iterator begin() { return t.begin(); }
        const_iterator begin() const { return t.begin(); }
        iterator end() { return t.end(); }
        const_iterator end() const { return t.end(); }
        reverse_iterator rbegin() { return t.rbegin(); }
        const_reverse_iterator rbegin() const { return t.rbegin(); }
        reverse_iterator rend() { return t.rend(); }
        const_reverse_iterator rend() const { return t.rend(); }
        bool empty() const { return t.empty(); }
        size_type size() const { return t.size(); }
        size_type max_size() const { return t.max_size(); }
        
        void swap(map<Key, T, Compare> &x) { t.swap(x.t); }

        // insert/erase
        iterator insert(const value_type& x) {
            return t.insert_equal(x);
        }
        iterator insert(iterator position, const value_type &x) {
            return t.insert_equal(position, x);
        }
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            t.insert_equal(first, last);
        }
        void erase(iterator position) { t.erase(position); }
        size_type erase(const key_type &x) { return t.erase(x); }
        void erase(iterator first, iterator last) { t.erase(first, last); }
        void clear() { t.clear(); }

        // map operators:
        iterator find(const key_type &x) { return t.find(x); }
        const_iterator find(const key_type &x) const { return t.find(x); }
        size_type count(const key_type &x) const { return t.count(x); }
        iterator lower_bound(const key_type &x) { return t.lower_bound(x); }
        const_iterator lower_bound(const key_type &x) const {
            return t.lower_bound(x);
        }
        iterator upper_bound(const key_type &x) { return t.upper_bound(x); }
        const_iterator upper_bound(const key_type &x) const {
            return t.upper_bound(x);
        }

        pair<iterator,iterator> equal_range(const key_type& x) {
            return t.equal_range(x);
        }
        pair<const_iterator,const_iterator> equal_range(const key_type& x) const {
            return t.equal_range(x);
        }
        friend bool operator==<>(const multimap &x, const multimap &y);
        friend bool operator< <>(const multimap &x, const multimap &y);
};

template <class Key, class T, class Compare>
inline bool operator==(const multimap<Key, T, Compare> &x, const multimap<Key, T, Compare> &y) {
    return x.t == y.t;
}
template <class Key, class T, class Compare>
inline bool operator<(const multimap<Key, T, Compare> &x, const multimap<Key, T, Compare> &y) {
    return x.t < y.t;
}

#endif