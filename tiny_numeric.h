#ifndef __TINY_INTERNAL_NUMERIC_H
#define __TINY_INTERNAL_NUMERIC_H

#include <functional>
#include <iterator>

// 对区间[first,last)内元素累加
template<class InputIterator, class T>
T accumulate(InputIterator first, InputIterator last, T init) {
    for (; first != last;++first)
        init = init + *first;       // 将每个元素累加到初值init身上
    return init;
}

// 对区间[first,last)内元素累加
template <class InputIterator, class T, class BinaryOperation>
T accumulate(InputIterator first, InputIterator last, T init,
               BinaryOperation binary_op) {
    for ( ; first != last; ++first)
        init = binary_op(init, *first);     // 对每一个元素执行二元操作
    return init;
}

/*
*   测试用例
*   __TINY_VECTOR_H::vector<int> num = {5, 1};
*   ostream_iterator<int> oit(cout, " ");
*   adjacent_difference(num.begin(), num.end(),oit);
*   std::cout << std::endl;
*/

// 利用operator-计算差额
// 计算[first,last)中相邻元素的差额
template <class InputIterator, class OutputIterator>
OutputIterator
adjacent_difference(InputIterator first,
                    InputIterator last, OutputIterator result) {
    typedef typename iterator_traits<InputIterator>::value_type ValueType;
    if (first == last) return result;
    *result = *first;       // 首先记录第一个元素
    ValueType value = *first;
    while (++first != last) {       // 走过整个区间
        ValueType tmp = *first;
        *++result = tmp - value;    // 将相邻两元素的差额，赋值给目的端
        value = tmp;
    }
    return ++result;
}

// 利用外界提供的二元仿函数
// 计算[first,last)中相邻元素的差额
template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator 
adjacent_difference(InputIterator first, InputIterator last,
                    OutputIterator result, BinaryOperation binary_op) {
    typedef typename iterator_traits<InputIterator>::value_type ValueType;
    if (first == last) return result;       // 首先记录第一个元素
    *result = *first;
    ValueType value = *first;
    while (++first != last) {       // 走过整个区间
        ValueType tmp = *first;
        *++result = binary_op(tmp, value);  // 将相邻两元素的运算结果，赋值给目的端
        value = tmp;
    }
    return ++result;
}


// 计算[first1,last1)和[first2,first2+(last1-first1))的一般内积
template <class InputIterator1, class InputIterator2, class T>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, T init) {
    // 以第一序列元素个数为据，将两个序列都走一遍
    for ( ; first1 != last1; ++first1, ++first2)
        init = init + (*first1 * *first2);
    return init;
}

// 计算[first1,last1)和[first2,first2+(last1-first1))的一般内积
template <class InputIterator1, class InputIterator2, class T,
          class BinaryOperation1, class BinaryOperation2>
T inner_product(InputIterator1 first1, InputIterator1 last1,
                  InputIterator2 first2, T init, 
                  BinaryOperation1 binary_op1,
                  BinaryOperation2 binary_op2) {
    // 以第一序列之元素个数为据，将两个序列都走一遍
    for ( ; first1 != last1; ++first1, ++first2)
        // 以外界提供的仿函数来取代第一版本中的operator*和operator+
        init = binary_op1(init, binary_op2(*first1, *first2));
    return init;
}


/*
*   测试用例
*   __TINY_VECTOR_H::vector<int> num = {5, 1};
*    ostream_iterator<int> oit(std::cout, " ");
*   partial_sum(num.begin(), num.end(), oit);
*   std::cout << std::endl;
*/

// 用于计算区间[first,last)内总和
template <class InputIterator, class OutputIterator>
OutputIterator 
partial_sum(InputIterator first, InputIterator last,
            OutputIterator result) {
    typedef typename iterator_traits<InputIterator>::value_type ValueType;
    if (first == last) return result;
    *result = *first;
    ValueType value = *first;
    while (++first != last) {
        value = value + *first;         // 前n个元素的总和
        *++result = value;          // 指定给目的端
    }
    return ++result;
}

// 用于计算区间[first,last)内总和
template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator 
partial_sum(InputIterator first, InputIterator last,
            OutputIterator result, BinaryOperation binary_op) {
    typedef typename iterator_traits<InputIterator>::value_type ValueType;
    if (first == last) return result;
    *result = *first;
    ValueType value = *first;
    while (++first != last) {
        value = binary_op(value, *first);       // 前n个元素的总和
        *++result = value;              // 指定给目的端
    }
    return ++result;
}

template <class T>
struct Multiplies : public binary_function<T, T, T> {
    T operator()(const T& x, const T& y) const { return x * y; }
};

// 乘幂
template <class T, class Integer>
inline T __power(T x, Integer n)
{
  return __power(x, n, Multiplies<T>());      // 指定运算型式为乘法
}

template <class T, class Integer, class MonoidOperation>
inline T power(T x, Integer n, MonoidOperation opr)
{
  return __power(x, n, opr);
}

template <class T, class Integer>
inline T power(T x, Integer n)
{
  return __power(x, n);
}

// 幂次方，如果指定为乘法运算，则当n>=0时返回x^n
template <class T, class Integer, class MonoidOperation>
T __power(T x, Integer n, MonoidOperation opr)
{
    if (n == 0)
        return T(1);
    else {
        while ((n & 1) == 0) {
            n >>= 1;
            x = opr(x, x);
        }

        T result = x;
        n >>= 1;
        while (n != 0) {
            x = opr(x, x);
            if ((n & 1) != 0)
                result = opr(result, x);
            n >>= 1;
        }
        return result;
    }
}

// 在[first,last)区间内填入value,value+1,value+2...
template <class ForwardIter, class T>
void 
iota(ForwardIter first, ForwardIter last, T value) {
    while (first != last)
        *first++ = value++;
}

#endif