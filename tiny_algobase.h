#ifndef __TINY_INTERNAL_ALGOBASE_H
#define __TINY_INTERNAL_ALGOBASE_H

#include <string.h>
#include <iterator>
#include <functional>
#include "tiny_pair.h"

// 编译器无力判别该类型
template<class T>
struct __type_traits {
    typedef __true_type this_dummy_member_must_be_first;
    typedef __true_type has_trivial_assignment_operator;
    typedef __false_type has_trivial_default_constructor;
    typedef __true_type has_trivial_copy_constructor;
    typedef __true_type has_trivial_destructor;
    typedef __false_type is_POD_type;
};

// 注意使用前必须判断两序列元素个数是否相同
// 比较两个序列在[first,last)区间内是否相等
template <class InputIterator1, class InputIterator2>
inline bool equal(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2) {
    for ( ; first1 != last1; ++first1, ++first2)
        if (*first1 != *first2)
            return false;
    return true;
}

// 比较两个序列在[first,last)区间内是否相等
// 允许指定仿函数pred作为比较依据
template <class InputIterator1, class InputIterator2, class BinaryPredicate>
inline bool equal(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, BinaryPredicate binary_pred) {
    for ( ; first1 != last1; ++first1, ++first2)
        if (!binary_pred(*first1, *first2))
            return false;
    return true;
}

// 将[first,last)内的所有元素改填新值
template <class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& value) {
    for ( ; first != last; ++first)     // 迭代走过整个区间
        *first = value;         // 设定新值
}

//  将[first,last)内的前n个元素改填新值
template <class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T& value) {
    for ( ; n > 0; --n, ++first)
        *first = value;
    return first;
}

// 特化版本
inline void fill(unsigned char* first, unsigned char* last,
                 const unsigned char& c) {
    unsigned char tmp = c;
    memset(first, tmp, last - first);
}

inline void fill(signed char* first, signed char* last,
                 const signed char& c) {
    signed char tmp = c;
    memset(first, static_cast<unsigned char>(tmp), last - first);
}

inline void fill(char* first, char* last, const char& c) {
    char tmp = c;
    memset(first, static_cast<unsigned char>(tmp), last - first);
}

// 交换迭代器所指的对象
template <class ForwardIterator1, class ForwardIterator2>
inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b) {
    typedef typename iterator_traits<ForwardIterator1>::value_type Valuetype;
    Valuetype tmp = *a;
    *a = *b;
    *b = tmp;
}

// 以字典排序对两个序列[first1,last1)和[first2,last2)进行比较
template <class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
                             InputIterator2 first2, InputIterator2 last2) {
    // 以下，任何一个序列到达尾端，就结束，否则两序列就相应元素一一对比
    for ( ; first1 != last1 && first2 != last2
            ; ++first1, ++first2) {
        if (*first1 < *first2)      // 第一序列元素值小于第二序列的相应元素值
            return true;
        if (*first2 < *first1)      // 第二序列元素值小于第一序列的相应元素值
            return false;
    }
    // 进行到这里，如果第一序列到达尾端而第二序列尚有余额，那么第一序列小于第二序列
    return first1 == last1 && first2 != last2;
}

// 对两个序列[first1,last1)和[first2,last2)进行比较
// 允许指定一个仿函数comp作为比较之用
template <class InputIterator1, class InputIterator2, class Compare>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
                             InputIterator2 first2, InputIterator2 last2,
                             Compare comp) {
    for ( ; first1 != last1 && first2 != last2
            ; ++first1, ++first2) {
        if (comp(*first1, *first2))
            return true;
        if (comp(*first2, *first1))
            return false;
    }
    return first1 == last1 && first2 != last2;
}

// 特化版本
inline bool 
lexicographical_compare(const unsigned char* first1,
                        const unsigned char* last1,
                        const unsigned char* first2,
                        const unsigned char* last2) {
  const size_t len1 = last1 - first1;
  const size_t len2 = last2 - first2;
  const int result = memcmp(first1, first2,__TINY_INTERNAL_ALGOBASE_H::min(len1, len2));
  return result != 0 ? result < 0 : len1 < len2;
}

template <class T>
inline const T& min(const T& a, const T& b) {
  return b < a ? b : a;
}

template <class T, class Compare>
inline const T& min(const T& a, const T& b, Compare comp) {
  return comp(b, a) ? b : a;
}

template <class T>
inline const T& max(const T& a, const T& b) {
  return  a < b ? b : a;
}

template <class T, class Compare>
inline const T& max(const T& a, const T& b, Compare comp) {
  return comp(a, b) ? b : a;
}

// 平行比较两个序列，指出两者之间的第一个不匹配点
template <class InputIterator1, class InputIterator2>
__TINY_PAIR_H::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
                                        InputIterator1 last1, InputIterator2 first2) {
    // 以下，如果序列一走完，就结束
    // 以下，如果序列一和序列二的对应元素相等，就结束
    // 显然，序列一的元素个数必须多过序列二的元素个数，否则结果无可预期
    while (first1 != last1 && *first1 == *first2) {
        ++first1;
        ++first2;
    }
    return __TINY_PAIR_H::pair<InputIterator1, InputIterator2>(first1, first2);
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
__TINY_PAIR_H::pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
                                        InputIterator1 last1, InputIterator2 first2,
                                        BinaryPredicate binary_pred) {
    while (first1 != last1 && binary_pred(*first1, *first2)) {
        ++first1;
        ++first2;
    }
    return __TINY_PAIR_H::pair<InputIterator1, InputIterator2>(first1, first2);
}

// 交换两个对象的内容
template <class T>
inline void swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

/*
*   测试用例：
*   __TINY_VECTOR_H::vector<int> num;
    for (int i = 0; i < 4; i++)
        num.push_back(i);
    ostream_iterator<int> oit(cout, " ");
    copy(num.begin(), num.end(),oit);
*
*/

// 完全泛化版本
template <class InputIterator, class OutputIterator>
struct __copy_dispatch
{
    OutputIterator operator()(InputIterator first, InputIterator last,
                                OutputIterator result) {
        return __copy(first, last, result, __iterator_category(first));
    }
};

// 完全泛化版本
template <class InputIterator, class OutputIterator, class Distance>
inline OutputIterator copy(InputIterator first, InputIterator last,
                          OutputIterator result)
{
    return __copy_dispatch<InputIterator,OutputIterator>()
        (first, last, result);
}

// 偏特化版本
inline char* copy(const char* first, const char* last, char* result) {
    memmove(result, first, last - first);
    return result + (last - first);
}

inline wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result) {
    memmove(result, first, sizeof(wchar_t) * (last - first));
    return result + (last - first);
}

// 偏特化版本
template <class T>
struct __copy_dispatch<T*,T*>
{
    T* operator()(T* first, T* last, T* result) {
        typedef typename __type_traits<T>::has_trivial_assignment_operator t;
        return __copy_t(first, last, result, t());
    }
};

template <class T>
struct __copy_dispatch<const T*,T*>
{
    T* operator()(const T* first, const T* last, T* result) {
        typedef typename __type_traits<T>::has_trivial_assignment_operator t;
        return __copy_t(first, last, result, t());
    }
};

// InputIterator版本
template <class InputIterator, class OutputIterator>
inline OutputIterator
__copy(InputIterator first, InputIterator last,
       OutputIterator result, input_iterator_tag) {
    for (; first != last; ++result, ++first)
        *result = *first;
    return result;
}

// RandomAccessIterator版本
template <class RandomAccessIterator, class OutputIterator>
inline OutputIterator
__copy(RandomAccessIterator first, RandomAccessIterator last,
       OutputIterator result, random_access_iterator_tag) {
    typedef typename iterator_traits<RandomAccessIterator>::distance_type Distance; 
    for (Distance n = last - first; n > 0; --n, ++result, ++first)
        *result = *first;
    return result;
}

template <class RandomAccessIter, class OutputIterator, class Distance>
inline OutputIterator
__copy_d(RandomAccessIter first, RandomAccessIter last,
       OutputIterator result, Distance*) {
    for (Distance n = last - first; n > 0; --n, ++result, ++first)
        *result = *first;
    return result;
}

template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __true_type) {
    memmove(result, first, sizeof(T) * (last - first));
    return result + (last - first);
}

template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __false_type) {
    memmove(result, first, sizeof(T) * (last - first));
    return __copy_d(first, last, result, (ptrdiff_t *)0);
}

template <class RandomAccessIterator, class BidirectionalIterator, class Distance>
inline BidirectionalIterator __copy_backward(RandomAccessIterator first, 
                                          RandomAccessIterator last, 
                                          BidirectionalIterator result,
                                          random_access_iterator_tag,
                                          Distance*) {
    for (Distance n = last - first; n > 0; --n)
        *--result = *--last;
    return result;
}

// 完全泛化版
template <class BidirectionalIterator1, class BidirectionalIterator2,
          class BoolType>
struct __copy_backward_dispatch
{
  typedef typename iterator_traits<BidirectionalIterator1>::iterator_category 
          Cat;
  typedef typename iterator_traits<BidirectionalIterator1>::difference_type
          Distance;

  static BidirectionalIterator2 copy(BidirectionalIterator1 first, 
                                  BidirectionalIterator1 last, 
                                  BidirectionalIterator2 result) {
    return __copy_backward(first, last, result, Cat(), (Distance*) 0);
  }
};

// 完全泛化版
template <class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
                                            BidirectionalIterator2 result) {
    typedef typename __type_traits<typename iterator_traits<BidirectionalIterator2>::value_type>
        ::has_trivial_assignment_operator Trivial;
    return __copy_backward_dispatch<BidirectionalIterator1, BidirectionalIterator2, Trivial>
        ::copy(first, last, result);
}

template <class BidirectionalIterator1, class BidirectionalIterator2, 
          class Distance>
inline BidirectionalIterator2 __copy_backward(BidirectionalIterator1 first, 
                                           BidirectionalIterator1 last, 
                                           BidirectionalIterator2 result,
                                           bidirectional_iterator_tag,
                                           Distance*) {
    while (first != last)
        *--result = *--last;
    return result;
}

// 偏特化版本
template <class T>
struct __copy_backward_dispatch<T*, T*, __true_type>
{
    static T* copy(const T* first, const T* last, T* result) {
        const ptrdiff_t Num = last - first;
        memmove(result - Num, first, sizeof(T) * Num);
        return result - Num;
    }
};

template <class T>
struct __copy_backward_dispatch<const T*, T*, __true_type>
{
    static T* copy(const T* first, const T* last, T* result) {
        return  __copy_backward_dispatch<T*, T*, __true_type>
        ::copy(first, last, result);
    }
};

#endif