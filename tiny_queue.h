#ifndef __TINY_QUEUE_H
#define __TINY_QUEUE_H
#include <iterator>
#include <memory>
#include "tiny_deque.h"

template <class T, class Sequence = deque<T> >
class queue;

template <class T, class Sequence>
inline bool operator==(const queue<T, Sequence>&, const queue<T, Sequence>&);

template <class T, class Sequence>
inline bool operator<(const queue<T, Sequence>&, const queue<T, Sequence>&);

template <class T, class Sequence>
class queue {
    friend bool operator== <>(const queue &x, const queue &y);
    friend bool operator< <>(const queue &x, const queue &y);
    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        typedef typename Sequence::reference reference;
        typedef typename Sequence::const_reference const_reference;
    protected:
        Sequence c;     // 底层容器
    public:
        // 以下完全利用Sequence c的操作，完成queue的操作
        bool empty() const { return c.empty(); }
        size_t size() const { return c.size(); }
        reference front() { return c.front(); }
        const_reference front() const { return c.front(); }
        reference back() { return c.back(); }
        const_reference back() const { return c.back(); }
        // deque是两头可进出，queue是末端进，前端出
        void push(const value_type &x) { c.push_back(x); }
        void pop() { c.pop_front(); }
};

template <class T, class Sequence>
bool operator==(const queue<T,Sequence>& x, const queue<T,Sequence>& y) {
    return x.c == y.c;
}

template <class T, class Sequence>
bool operator<(const queue<T,Sequence>& x, const queue<T,Sequence>& y) {
    return  x.c < y.c;
}

template <class T, class Sequence=vector<T>, class Compare = less<typename Sequence::value_type>>
class priority_queue {
    public:
        typedef typename Sequence::value_type value_type;
        typedef typename Sequence::size_type size_type;
        typedef typename Sequence::reference reference;
        typedef typename Sequence::const_reference const_reference;
    protected:
        Sequence c;     // 底层容器比较
        Compare comp;   // 元素大小比较标准
    public:
        priority_queue() : c() {}
        explicit priority_queue(const Compare& x) : c(),comp(x) {}
        priority_queue(const Compare& x, const Sequence& s) 
        : c(s), comp(x) { make_heap(c.begin(), c.end(), comp); }
        // 以下用到的都是泛型算法
        template <class InputIterator>
        priority_queue(InputIterator first, InputIterator last, const Compare &x)
            : c(first, last), comp(x) { make_heap(c.begin(), c.end(), comp); }
        template <class InputIterator>
        priority_queue(InputIterator first, InputIterator last)
            : c(first, last) { make_heap(c.begin(), c.end(), comp); }

        bool empty() const { return c.empty(); }
        size_type size() const { return c.szie(); }
        const_reference top() const { return c.front(); }
        reference top() { return c.front(); }
        void push(const value_type& x) {
            try {
                // 先利用底层容器的push_back()将新元素，推入末端，再重排heap
                c.push_back(x);
                push_heap(c.begin(), c.end(), comp);    // push_heap是泛型算法
            }
            catch(const std::exception& e) {
                c.clear();
            }
        }

        void pop() {
            try {
                // 从heap内取出一个元素，它并不是真正将元素弹出，而是重排heap
                // 再以底层容器的pop_back()取出被弹出的元素
                pop_heap(c.begin(), c.end(), comp);
                c.pop_back();
            }
            catch(const std::exception& e) {
                c.clear();
            }
        }
};
#endif