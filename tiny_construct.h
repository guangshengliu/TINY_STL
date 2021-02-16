#ifndef __TINY_CONSTRUCT_H
#define __TINY_CONSTRUCT_H
using namespace std;
#include <new.h>

/*此模板尚未弄懂如何实现
*为此调用STL提供的接口
*/

// 调用construct和destroy的全局函数

// 以下是Construct()的第一个版本
template <class T1, class T2>
inline void Construct(T1* p, const T2& value) {
  _Construct(p,value);
}

// 以下是Construct()的第二个版本
template <class T>
inline void Construct(T* p) {
  _Construct(p);
}

// 以下是destroy()的第一个版本，接受一个指针
template <class T>
inline void Destroy(T* pointer) {
  _Destroy(pointer);
}

// 以下是destory()第二版本，接受两个迭代器。
template <class ForwardIterator>
inline void Destroy(ForwardIterator first, ForwardIterator last) {
  _Destroy(first, last);
}

#endif