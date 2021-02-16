#ifndef __TINY_VECTOR_H
#define __TINY_VECTOR_H

#include <memory>
#include <algorithm>
#include <iterator>
#include "tiny_alloc.h"
#include "tiny_construct.h"

template <class T>
class vector {
    public:
        typedef T value_type;
        typedef value_type *pointer;
        typedef const value_type* const_pointer;
        typedef value_type *iterator;   // vector的迭代器是普通指针
        typedef const value_type* const_iterator;
        typedef value_type &reference;  // vector的[]
        typedef const value_type& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        typedef reverse_iterator<const_iterator> const_reverse_iterator;
        typedef reverse_iterator<iterator> reverse_iterator;

    protected:
        // 以下，simple_alloc是简化版空间配置器
        typedef simple_alloc<value_type> data_allocator;
        iterator start;     // 表示目前使用空间的头
        iterator finish;    // 表示目前使用空间的尾,指向最后一个元素的下一个位置
        iterator end_of_storage;    // 表示目前可用空间的尾

        void insert_aux(iterator position, const T &x);
        void deallocate() {
            if (start)
                data_allocator::deallocate(start, end_of_storage - start);
        }
        // 填充并予以初始化
        void fill_initialize(size_type n,const T& value) {
            start = allocate_and_fill(n, value);
            finish = start + n;
            end_of_storage = finish;
        }
    public:
        iterator begin() { return start; }
        const_iterator begin() const { return start; }

        iterator end() { return finish; }
        const_iterator end() const { return finish; }

        reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

        size_type size() const { return size_type(end() - begin()); }     // vector已用空间
        size_type max_size() const { return size_type(-1) / sizeof(T); }
        size_type capacity() const {
            return size_type(end_of_storage - begin());     // vector实际空间
        }
        bool empty() const { return begin() == end(); }

        const_reference operator[](size_type n) const { return *(begin() + n); }
        reference operator[](size_type n) { return *(begin() + n); }

        void range_check(size_type n) const {
            if (n >= this->size())
                return;
        }

        reference at(size_type n) { 
            range_check(n); 
            return (*this)[n];
        }
        const_reference at(size_type n) const {   
            range_check(n);
            return (*this)[n]; 
        }
    public:
        // 构造函数
        vector():start(0),finish(0),end_of_storage(0) {}
        vector(size_type n) { fill_initialize(n, T()); }
        vector(size_type n, const T &value) { fill_initialize(n, value); }
        vector(int n, const T &value) { fill_initialize(n, value); }
        vector(long n, const T &value) { fill_initialize(n, T()); }
        vector(const T* first, const T* last){
            fill_initialize(1, 0);      // 先分配一个元素的空间
            finish = uninitialized_copy(first, last, start); 
        }

        // 析构函数
        ~vector() {
            Destroy(start, finish);     // 全局函数
            //deallocate();
        }

        // 容器预留空间大小n
        void reserve(size_type n) {
            if (capacity() < n) {
                const size_type old_size = size();
                iterator tmp = allocate_and_copy(n, start, finish);
                Destroy(start, finish);
                data_allocator::deallocate(start, end_of_storage - start);
                start = tmp;
                finish = tmp + old_size;
                end_of_storage = start + n;
            }
        }

        reference front() { return *begin(); }      // 第一个元素
        reference back() { return *(end() - 1); }       // 最后一个元素
        void push_back(const T& x) {        // 将元素插入至最尾端
            if(finish != end_of_storage) {
                Construct(finish, x);
                ++finish;
            }
            else
                insert_aux(end(), x);
        }

        void swap(vector<T>& x) {
            std::swap(start, x.start);
            std::swap(finish, x.finish);
            std::swap(end_of_storage, x.end_of_storage);
        }

        void pop_back() {       // 将最尾端元素取出
            --finish;
            Destroy(finish);
        }

        iterator erase(iterator position) {     // 清除某位置上的元素
            if(position + 1 != end())
                copy(position + 1, finish, position);
            --finish;
            Destroy(finish);
            return position;
        }

        iterator erase(iterator frist,iterator last) {
            iterator i = copy(last, finish, frist);
            Destroy(i, finish);
            finish = finish - (last - frist);
            return frist;
        }

        void resize(size_type new_size,const T& x) {
            if(new_size<size())
                erase(begin() + new_size, end());
            else
                insert(end(), new_size - size(), x);
        }
        void resize(size_type new_size) { resize(new_size, T()); }
        void clear() { erase(begin(), end()); }
        // 插入元素
        void insert(iterator position, size_type n, const T &x);
        void insert(iterator position, const T &x) { insert_aux(position, x); }
        void insert_aux(iterator position);
        vector<T> &operator=(const vector<T> &x);

    protected:
        // 配置而后填充
        iterator allocate_and_fill(size_type n,const T& x) {
            iterator result = data_allocator::allocate(n);      // 配置n个元素空间
            uninitialized_fill_n(result, n, x);     //全局函数
            return result;
        }
        // 配置而后复制
        template <class ForwardIterator>
        iterator allocate_and_copy(size_type n, ForwardIterator first, ForwardIterator last)
        {
            iterator result = data_allocator::allocate(n);
            try {
                uninitialized_copy(first, last, result);
                return result;
            }
            catch(const std::exception& e) {
                data_allocator::deallocate(result, n);
                return result;
            }
        }
};

// 从position开始，插入一个元素，元素初值为x
template <class T>
void vector<T>::insert_aux(iterator position,const T& x) {
    // 在备用空间起始处构造一个元素，并以vector最后一个元素值为其初值
    if(finish != end_of_storage) {
        Construct(finish, *(finish - 1));
        // 调整位置
        ++finish;
        T x_copy = x;
        copy_backward(position, finish - 2, finish - 1);
        *position = x_copy;
    }
    else {      // 已无备用空间
        const size_type old_size = size();
        const size_type len = old_size != 0 ? 2 * old_size : 1;
        // 以上配置原则：如果原大小为0，则配置1个
        // 如果原大小不为0，则配置原大小的两倍
        // 前半段用来放置原数据，后半段准备用来放置新数据

        iterator new_start = data_allocator::allocate(len);     // 实际配置
        iterator new_finish = new_start;
        try
        {
            // 将原vector的内容拷贝到新vector
            new_finish = uninitialized_copy(start, position, new_start);
            // 为新元素设定初值x
            Construct(new_finish, x);
            // 调整位置
            ++new_finish;
            // 将安插点的原内容也拷贝过来
            new_finish = uninitialized_copy(position, finish, new_finish);
        }
        catch(const std::exception& e)
        {
            // commit or rollback semantics
            Destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, len);
            throw;
        }
        
        // 析构并释放原vector
        Destroy(begin(), end());
        deallocate();

        // 调整迭代器，指向新的vector
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}
// 从position开始，插入一个元素，使用默认初值
template <class T>
void vector<T>::insert_aux(iterator position)
{
    if (finish != end_of_storage) {
        construct(finish, *(finish - 1));
        ++finish;
        copy_backward(position, finish - 2, finish - 1);
        *position = T();
    }
    else {
        const size_type old_size = size();
        const size_type len = old_size != 0 ? 2 * old_size : 1;
        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        try {
            new_finish = uninitialized_copy(start, position, new_start);
            Construct(new_finish);
            ++new_finish;
            new_finish = uninitialized_copy(position, finish, new_finish);
        }
        catch(const std::exception& e) {
            Destroy(new_start, new_finish);
            data_allocator::deallocate(new_start, len);
        }
        Destroy(begin(), end());
        data_allocator::deallocate(start, end_of_storage - start);
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

// 从position开始，插入n个元素，元素初值为x
template<class T>
void vector<T>::insert(iterator position,size_type n,const T& x) {
    if(n != 0) {
        if(size_type(end_of_storage-finish) >= n) {
            // 备用空间大于等于新增元素个数
            T x_copy = x;
            // 以下计算插入点之后的现有元素个数
            const size_type elems_after = finish - position;
            iterator old_finish = finish;
            if(elems_after > n ) {
                // 插入点之后的现有元素个数大于新增元素个数
                uninitialized_copy(finish - n, finish, finish);
                finish += n;    // 将vector尾端标记后移
                copy_backward(position, old_finish - n, old_finish);
                fill(position, position + n, x_copy);   // 从插入点开始填入新值
            }
            else{
                // 插入点之后的现有元素个数小于等于新增元素个数
                uninitialized_fill_n(finish, n - elems_after, x_copy);
                finish += n - elems_after;
                uninitialized_copy(position, old_finish, finish);
                finish += elems_after;
                fill(position, old_finish, x_copy);
            }
        }
        else{
            // 备用空间小于新增元素个数，必须配置额外的内存
            // 首先决定新长度：旧长度的两倍，或旧长度+新增元素个数
            const size_type old_size = size();
            const size_type len = old_size + max(old_size, n);
            // 以下配置新的vector空间
            iterator new_start = data_allocator::allocate(len);
            iterator new_finish = new_start;

            // 以下首先将旧vector的插入点之前的元素复杂到新的空间
            new_finish = uninitialized_copy(start, position, new_start);
            // 以下再将新增元素，初值皆为n，填入新空间
            new_finish = uninitialized_fill_n(new_finish, n, x);
            // 以下再将旧vector的插入点之后的元素复制到新空间
            new_finish = uninitialized_copy(position, finish, new_finish);

        # ifdef __STL_USE_EXCEPTIONS
            catch() {
                Destroy(new_start, new_finish);
                data_allocator::deallocate(new_start, len);
                throw;
            }
        # endif
            // 以下清楚并释放旧的vector
            Destroy(start, finish);
            deallocate();
            // 以下调整位置标记
            start = new_start;
            finish = new_finish;
            end_of_storage = new_start + len;
        }
    }
}
// ==运算符重载
template <class T>
inline bool 
operator==(vector<T>& x, vector<T>& y) 
{
    return x.size() == y.size()&&
        equal(x.begin(), x.end(), y.begin());
}
// <运算符重载
template <class T>
inline bool 
operator< (vector<T>& x, vector<T>& y)
{
  return lexicographical_compare(x.begin(), x.end(), 
                                 y.begin(), y.end());
}
// !=运算符重载
template <class T>
inline bool
operator!=(vector<T>& x, vector<T>& y) {
  return !(x == y);
}

// >运算符重载
template <class T>
inline bool
operator>(vector<T>& x, vector<T>& y) {
  return y < x;
}

// <=运算符重载
template <class T>
inline bool
operator<=(vector<T>& x, vector<T>& y) {
  return !(y < x);
}

// >=运算符重载
template <class T>
inline bool
operator>=(vector<T>& x, vector<T>& y) {
  return !(x < y);
}

template <class T>
vector<T>& vector<T>::operator=(const vector<T>& x)
{
    if (&x != this) {
        const size_type xlen = x.size();
        if (xlen > capacity()) {
            iterator tmp = _allocate_and_copy(xlen, x.begin(), x.end());
            Destroy(start, finish);
            deallocate(start, end_of_storage - start);
            start = tmp;
            end_of_storage = start + xlen;
        }
        else if (size() >= xlen) {
            iterator i = copy(x.begin(), x.end(), begin());
            destroy(i, finish);
        }
        else {
            copy(x.begin(), x.begin() + size(), start);
            uninitialized_copy(x.begin() + size(), x.end(), finish);
        }
        finish = start + xlen;
    }
    return *this;
}

#endif