#ifndef __TINY_MULTISET_H
#define __TINY_MULTISET_H

#include "tiny_tree.h"
#include <functional>

template <class Key,class Compare = less<Key> >
class multiset;

template <class Key, class Compare>
inline bool operator==(const multiset<Key,Compare>& x, const multiset<Key,Compare>& y);

template <class Key, class Compare>
inline bool operator<(const multiset<Key,Compare>& x, const multiset<Key,Compare>& y);

template <class Key,class Compare>
class multiset {
    public:
        // typedefs
        typedef Key key_type;
        typedef Key value_type;
        // 注意：以下key_compare和value_compare使用同一比较函数
        typedef Compare key_compare;
        typedef Compare value_compare;
    private:
        // 以下identity定义于<stl_functionh>
        typedef rb_tree<key_type, value_type, std::_Identity<value_type>, key_compare> rep_type;
        rep_type t;     // 采用红黑树(RB-tree)来表现set
    public:
        typedef typename rep_type::const_pointer poniter;
        typedef typename rep_type::const_pointer const_pointer;
        typedef typename rep_type::const_reference reference;
        typedef typename rep_type::const_reference const_reference;
        typedef typename rep_type::const_iterator iterator;
        // iterator定义为RB-tree的const_iterator，这表示set的
        // 迭代器无法执行写入操作，这是因为set的元素有一定次序安排
        // 不允许用户在任意处进行写入操作
        typedef typename rep_type::const_iterator const_iterator;
        typedef typename rep_type::const_reverse_iterator reverse_iterator;
        typedef typename rep_type::const_reverse_iterator const_reverse_iterator;
        typedef typename rep_type::size_type size_type;
        typedef typename rep_type::difference_type difference_type;

        // allocation/deallocation
        // 注意，set一定使用RB-tree的insert_unique()而非insert_equal()
        // multiset才使用RB-tree的insert_equal()
        // 因为set不允许相同键值存在，multiset才允许相同键值存在
        multiset() : t(Compare()) {}
        explicit multiset(const Compare& comp) : t(comp) {}

        template <class InputIterator>
        multiset(InputIterator first, InputIterator last)
            : t(Compare()) { t.insert_equal(first, last); }

        template <class InputIterator>
        multiset(InputIterator first, InputIterator last, const Compare &comp)
            : t(comp) { t.insert_equal(first, last); }
        
        multiset(const set<Key,Compare>& x) : t(x.t) {}
        multiset<Key,Compare>& operator=(const multiset<Key,Compare>& x) {
            t = x.t;
            return *this;
        }

        // 以下所有的set操作行为，RB-Tree都已提供，所以set只要传递参数调用即可

        key_compare key_comp() const { return t.key_comp(); }

        value_compare value_comp() const { return t.key_comp(); }

        iterator begin() const { return t.begin(); }
        iterator end() const { return t.end(); }
        reverse_iterator rbegin() const { return t.rbegin(); }
        reverse_iterator rend() const { return t.rend(); }
        bool empty() const { return t.empty(); }
        size_type size() const { return t.size(); }
        size_type max_size() const { return t.max_size(); }
        void swap(multiset<Key, Compare> &x) { t.swap(x.t); }

        // insert/erase
        void insert(const value_type& x) {
            t.insert_equal(x);
        }
        void insert(iterator position, const value_type &x) {
            typedef typename rep_type::iterator rep_iterator;
            t.insert_equal((rep_iterator &)position, x);
        }
        template <class InputIterator>
        void insert(InputIterator first, InputIterator last) {
            t.insert_equal(first, last);
        }
        void erase(iterator position) {
            typedef typename rep_type::iterator rep_iterator;
            t.erase((rep_iterator &)position);
        }
        size_type erase(const key_type& x) {
            return t.erase(x);
        }
        void erase(iterator first, iterator last) {
            typedef typename rep_type::iterator rep_iterator;
            t.erase((rep_iterator &)first, (rep_iterator &)last);
        }
        void clear() { t.clear(); }

        // set operations:
        iterator find(const key_type &x) const { return t.find(x); }
        size_type count(const key_type &x) const { return t.count(x); }
        iterator lower_bound(const key_type &x) const { return t.lower_bound(x); }
        iterator upper_bound(const key_type &x) const { return t.upper_bound(x); }
        pair<iterator,iterator> equal_range(const key_type& x) const {
            return t.equal_range(x);
        }

        friend bool operator==<>(const multiset &, const multiset &);
        friend bool operator< <>(const multiset &, const multiset &);
};

template <class Key, class Compare>
inline bool operator==(const multiset<Key, Compare> &x, const multiset<Key, Compare> &y) {
    return x.t = y.t;
}

template <class Key, class Compare>
inline bool operator<(const multiset<Key, Compare> &x, const multiset<Key, Compare> &y) {
    return x.t < y.t;
}

#endif