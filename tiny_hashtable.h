#ifndef __TINY_HASHTABLE_H
#define __TINY_HASHTABLE_H

#include <iterator>
#include <algorithm>
#include "tiny_alloc.h"
#include "tiny_construct.h"
#include "tiny_vector.h"

template <class Value>
struct __hashtable_node
{
    __hashtable_node* next;
    Value val;
};

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
class hashtable;

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
struct __hashtable_iterator;

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
struct __hashtable_const_iterator;

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
struct __hashtable_iterator {
    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef Value &reference;
    typedef Value pointer;

    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey> hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> const_iterator;
    typedef __hashtable_node<Value> node;

    node *cur;      // 迭代器目前所指之节点
    hashtable *ht;  // 保持对容器的连结关系

    __hashtable_iterator(node *n, hashtable *tab) : cur(n), ht(tab) {}
    __hashtable_iterator() {}
    reference operator*() const { return cur->val; }
    pointer operator->() const { return &(operator*()); }
    iterator &operator++();
    iterator operator++(int);
    bool operator==(const iterator &it) const { return cur == it.cur; }
    bool operator!=(const iterator &it) const { return cur != it.cur; }
};

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
struct __hashtable_const_iterator {
    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey> hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> const_iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef const Value &reference;
    typedef const Value pointer;

    const node *cur;      // 迭代器目前所指之节点
    const hashtable *ht;  // 保持对容器的连结关系

    __hashtable_const_iterator(const node* n, const hashtable* tab) : cur(n), ht(tab) {}
    __hashtable_const_iterator() {}
    __hashtable_const_iterator(const iterator& it) : cur(it.cur), ht(it.ht) {}
    reference operator*() const { return cur->val; }
    pointer operator->() const { return &(operator*()); }
    const_iterator& operator++();
    const_iterator operator++(int);
    bool operator==(const const_iterator& it) const { return cur == it.cur; }
    bool operator!=(const const_iterator& it) const { return cur != it.cur; }
};

// 注意：假设long至少有32bits
static const int __stl_num_primes = 28;
static const unsigned long __stl_prime_list[__stl_num_primes] =
{
    53ul,         97ul,         193ul,       389ul,       769ul,
    1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul, 
    1610612741ul, 3221225473ul, 4294967291ul
};

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
bool operator==(const hashtable<Value, Key, HashFcn, ExtractKey, EqualKey> &ht1,
                const hashtable<Value, Key, HashFcn, ExtractKey, EqualKey> &ht2);

// 以下找出上述28个质数之中，最接近并大于或等于n的那个质数
inline unsigned long __stl_next_prime(unsigned long n)
{
    const unsigned long* first = __stl_prime_list;
    const unsigned long* last = __stl_prime_list + (int)__stl_num_primes;
    const unsigned long* pos = lower_bound(first, last, n);
    // 以上，lower_bound()是泛型算法
    // 使用lower_bound()，序列需先排序
    return pos == last ? *(last - 1) : *pos;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
class hashtable {
    public:
        // 为template型别参数重新定义一个名称
        typedef Value value_type;
        typedef Key key_type;
        typedef HashFcn hasher;
        typedef EqualKey key_equal;

        typedef size_t size_type;
        typedef ptrdiff_t         difference_type;
        typedef value_type*       pointer;
        typedef const value_type* const_pointer;
        typedef value_type&       reference;
        typedef const value_type& const_reference;

        hasher hash_funct() const { return hash; }
        key_equal key_eq() const { return equals; }

    private:
        hasher hash;
        key_equal equals;
        ExtractKey get_key;

        typedef __hashtable_node<Value> node;
        typedef simple_alloc<node> node_allocator;

        __TINY_VECTOR_H::vector<node *> buckets;
        size_type num_elements;
        // 构造结点
        node* get_node() { return node_allocator::allocate(1); }
        // 析构结点
        void put_node(node* p) { node_allocator::deallocate(p, 1); }
    public:
        typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>
                iterator;
        typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> 
                const_iterator;

        friend struct
        __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>;
        friend struct
        __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>;

        friend bool operator== <>(const hashtable &, const hashtable &);

    public:
        // bucket个数即buckets vector的大小
        size_type bucket_count() const { return buckets.size(); }
        // 总共可以有多少buckets，以下是hash_table的一个member function
        size_type max_bucket_count() const { return __stl_prime_list[__stl_num_primes - 1]; }
        // 统计bucket中是元素个数
        size_type elems_in_bucket(size_type bucket) const {
            size_type result = 0;
            for (node* cur = buckets[bucket]; cur; cur = cur->next)
                result += 1;
            return result;
        }
        // 返回最接近n并大于等于n的质数
        size_type next_size(size_type n) const { return __stl_next_prime(n); }
        
        // 版本一：接受实值value和buckets个数
        size_type bkt_num(const value_type& obj,size_t n) const {
            return bkt_num_key(get_key(obj), n);    // 调用版本4
        }
        // 版本二：只接受实值(value)
        size_type bkt_num(const value_type& obj) const {
            return bkt_num_key(get_key(obj));       // 调用版本3
        }
        // 版本三：只接受键值
        size_type bkt_num_key(const key_type& key) const {
            return bkt_num_key(key, buckets.size());    // 调用版本4
        }
        // 版本四：接受键值和buckets个数
        size_type bkt_num_key(const key_type &key, size_type n) const {
            return hash(key) % n;
        }

    private:
        node* new_node(const value_type& obj) {
            node *n = node_allocator::allocate();
            n->next = 0;
            try {
                Construct(&n->val, obj);
                return n;
            }
            catch(const std::exception& e) {
                node_allocator::deallocate(n);
                return n;
            }
        }
        void delete_node(node* n) {
            Destroy(&n->val);
            node_allocator::deallocate(n);
        }

    public:
        // 初始化构造函数
        hashtable(size_type n, const HashFcn &hf, const EqualKey &eql)
            : hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0) {
            initializer_buckets(n);
        }

        // 拷贝构造函数
        hashtable(const hashtable &ht) : hash(ht.hash), equals(ht.equals),
                                         get_key(ht.get_key), num_elements(0) {
            copy_from(ht);
        }

        // 构造函数
        hashtable(size_type n, const HashFcn &hf,
                  const EqualKey &eql, const ExtractKey &ext)
            : hash(hf), equals(eql), get_key(ext), num_elements(0) {
            initializer_buckets(n);
        }

        hashtable& operator= (const hashtable& ht) {
            if (&ht != this) {
                clear();
                hash = ht.hash;
                equals = ht.equals;
                get_key = ht.get_key;
                copy_from(ht);
            }
            return *this;
        }

        ~hashtable() { clear(); }

        size_type size() const { return num_elements; }
        size_type max_size() const { return size_type(-1); }
        bool empty() const { return size() == 0; }

        void swap(hashtable& ht) {
            std::swap(hash, ht.hash);
            std::swap(equals, ht.equals);
            std::swap(get_key, ht.get_key);
            buckets.swap(ht.buckets);
            std::swap(num_elements, ht.num_elements);
        }

        iterator begin() { 
            for (size_type n = 0; n < buckets.size(); ++n)
                if (buckets[n])
                    return iterator(buckets[n], this);
            return end();
        }

        iterator end() { return iterator(0, this); }

        const_iterator begin() const {
            for (size_type n = 0; n < buckets.size(); ++n)
                if (buckets[n])
                    return const_iterator(buckets[n], this);
            return end();
        }

        const_iterator end() const { return const_iterator(0, this); }

    private:
        void clear();

        // 拷贝函数
        void copy_from(const hashtable &ht);
        // 初始化buckets
        void initializer_buckets(size_type n) {
            const size_type n_buckets = next_size(n);
            buckets.reserve(n_buckets);
            buckets.insert(buckets.end(), n_buckets, (node *)0);
            num_elements = 0;
        }
        // 在不需要重建表格的情况下插入新节点，键值不允许重复
        pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator, bool>
        insert_unique_noresize(const value_type& obj);

        typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator
        insert_equal_noresize(const value_type& obj);

        void erase_bucket(const size_type n, node *first, node *last);
        void erase_bucket(const size_type n, node *last);

    public:
        // 重新定义容器预留空间大小
        void resize(size_type num_elements_hint);
        // 插入元素，不允许重复
        pair<iterator,bool> insert_unique(const value_type& obj) {
            resize(num_elements + 1);   // 判断是否需要重建表格，如需要就扩充
            return insert_unique_noresize(obj);
        }
        // 插入元素，允许重复
        iterator insert_equal(const value_type& obj) {
            resize(num_elements + 1);
            return insert_equal_noresize(obj);
        }

        template <class InputIterator>
        void insert_unique(InputIterator f, InputIterator l, input_iterator_tag) {
            for ( ; f != l; ++f)
                insert_unique(*f);
        }

        template <class InputIterator>
        void insert_equal(InputIterator f, InputIterator l, input_iterator_tag) {
            for ( ; f != l; ++f)
                insert_equal(*f);
        }

        template <class ForwardIterator>
        void insert_unique(ForwardIterator f, ForwardIterator l, forward_iterator_tag) {
            size_type n = 0;
            distance(f, l, n);
            resize(num_elements + n);
            for ( ; n > 0; --n, ++f)
                insert_unique_noresize(*f);
        }

        template <class ForwardIterator>
        void insert_equal(ForwardIterator f, ForwardIterator l, forward_iterator_tag)
        {
            size_type n = 0;
            distance(f, l, n);
            resize(num_elements + n);
            for ( ; n > 0; --n, ++f)
                insert_equal_noresize(*f);
        }

        // 根据下标位置查找或插入元素
        reference find_or_insert(const value_type& obj);

        iterator find(const key_type& key) {
            size_type n = bkt_num_key(key);
            node* first;
            for ( first = buckets[n];first && !equals(get_key(first->val), key);
                first = first->next)
            {}
            return iterator(first, this);
        }

        const_iterator find(const key_type &key) const
        {
            size_type n = bkt_num_key(key);
            const node* first;
            for ( first = buckets[n];first && !equals(get_key(first->val), key);
                first = first->next)
            {}
            return const_iterator(first, this);
        }

        size_type count(const key_type& key) const {
            const size_type n = bkt_num_key(key);
            size_type result = 0;

            for (const node* cur = buckets[n]; cur; cur = cur->next)
            if (equals(get_key(cur->val), key))
                ++result;
            return result;
        }
    public:
        pair<iterator, iterator> equal_range(const key_type& key);

        pair<const_iterator, const_iterator> equal_range(const key_type& key) const;

        size_type erase(const key_type& key);
        void erase(const iterator& it);
        void erase(iterator first, iterator last);

        void erase(const const_iterator& it);
        void erase(const_iterator first, const_iterator last);
};

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> &
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>::operator++() {
    const node *old = cur;
    cur = cur->next;        // 如果存在，就是它，否则就进入以下if流程
    if(!cur) {
        // 根据元素值，定位出下一个bucket，其起头处就是我们的目的地
        size_type bucket = ht->bkt_num(old->val);
        while (!cur && ++bucket < ht->buckets.size())   // 注意，operator++
            cur = ht->buckets[bucket];
    }
    return *this;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
inline __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>::operator++(int) {
    iterator tmp = *this;
    ++*this;        // 调用operator++()
    return tmp;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
__hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey> &
__hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>::operator++()
{
    const node *old = cur;
    cur = cur->next;        // 如果存在，就是它，否则就进入以下if流程
    if(!cur) {
        // 根据元素值，定位出下一个bucket，其起头处就是我们的目的地
        size_type bucket = ht->bkt_num(old->val);
        while (!cur && ++bucket < ht->buckets.size())   // 注意，operator++
            cur = ht->buckets[bucket];
    }
    return *this;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
inline __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>
__hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey>::operator++(int) {
    iterator tmp = *this;
    ++*this;        // 调用operator++()
    return tmp;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
bool operator==(const hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>& ht1,
                const hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>& ht2)
{
    typedef typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::node node;
    if (ht1.buckets.size() != ht2.buckets.size())
        return false;
    for (int n = 0; n < ht1.buckets.size(); ++n) {
        node* cur1 = ht1.buckets[n];
        node* cur2 = ht2.buckets[n];
        for ( ; cur1 && cur2 && cur1->val == cur2->val;
            cur1 = cur1->next, cur2 = cur2->next)
        {}
        if (cur1 || cur2)
        return false;
    }
    return true;
}

// 以下函数判断是否需要重建表格，如果不需要，立即回返
template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::resize(size_type num_elements_hint)
{
  const size_type old_n = buckets.size();
    if (num_elements_hint > old_n) {        // 确定真的需要重新配置
        const size_type n = next_size(num_elements_hint);   // 找出下一个质数
        if (n > old_n) {
            __TINY_VECTOR_H::vector<node *> tmp(n, (node *)0);       // 设立新的buckets
            try {
                // 以下处理每一个旧的buckets
                for (size_type bucket = 0; bucket < old_n; ++bucket) {
                node* first = buckets[bucket];      // 指向节点所对应之串行的起始节点
                    // 以下处理每一个旧buckets所含的每一个节点
                    while (first) {     // 串行还没结束时
                        // 以下找出节点落在哪一个新buckets内
                        size_type new_bucket = bkt_num(first->val, n);
                        // 令旧bucket指向其所对应之串行的下一个节点
                        buckets[bucket] = first->next;
                        // 将当前节点插入到新bucket内，成为其对应串行的第一个节点
                        first->next = tmp[new_bucket];
                        tmp[new_bucket] = first;
                        // 回到旧bucket所指的待处理串行，准备处理下一个节点
                        first = buckets[bucket];          
                    }
                }
                buckets.swap(tmp);
            }
            catch(const std::exception& e) {
                for (size_type bucket = 0; bucket < tmp.size(); ++bucket) {
                    while (tmp[bucket]) {
                        node* next = tmp[bucket]->next;
                        delete_node(tmp[bucket]);
                        tmp[bucket] = next;
                    }
                }
                throw;
            }
        }
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator,bool>
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::insert_unique_noresize(const value_type& obj) {
    const size_type n = bkt_num(obj);
    node* first = buckets[n];

    for (node* cur = first; cur; cur = cur->next) 
        if (equals(get_key(cur->val), get_key(obj)))
            return pair<iterator, bool>(iterator(cur, this), false);

    node* tmp = new_node(obj);
    tmp->next = first;
    buckets[n] = tmp;
    ++num_elements;
    return pair<iterator, bool>(iterator(tmp, this), true);
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::insert_equal_noresize(const value_type& obj)
{
    const size_type n = bkt_num(obj);
    node* first = buckets[n];

    for (node* cur = first; cur; cur = cur->next) 
    {
        if (equals(get_key(cur->val), get_key(obj))) {
            node* tmp = new_node(obj);
            tmp->next = cur->next;
            cur->next = tmp;
            ++num_elements;
            return iterator(tmp, this);
        }
    }

    node* tmp = new_node(obj);
    tmp->next = first;
    buckets[n] = tmp;
    ++num_elements;
    return iterator(tmp, this);
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::clear()
{
    // 针对每一个bucket
    for (size_type i = 0; i < buckets.size(); ++i) {
        node* cur = buckets[i];
        // 将bucket list中的每一个节点删除掉
        while (cur != 0) {
            node* next = cur->next;
            delete_node(cur);
            cur = next;
        }
        buckets[i] = 0;     // 令bucket内容为null指针
    }
    num_elements = 0;       // 令总节点个数为0
    // 注意，buckets vector并未释放掉空间，仍保有原来大小
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::copy_from(const hashtable& ht)
{
    // 先清除已方的buckets vector，这操作是调用vector::clear，将整个容器清空
    buckets.clear();
    // 为己方的buckets vector保留空间，使与对方相同
    // 如果己方空间大于对方，就不动，如果己方空间小于对方，就会增大
    buckets.reserve(ht.buckets.size());
    // 从己方的buckets vector尾端开始，插入n个元素，其值为null的指针
    // 注意，此时buckets vector为空，所以所谓尾端，就是起头处
    buckets.insert(buckets.end(), ht.buckets.size(), (node*) 0);
    try {
        // 针对buckets vector
        for (size_type i = 0; i < ht.buckets.size(); ++i) {
            // 复制vector的每一个元素
            const node* cur = ht.buckets[i];
            if (cur) {
                node* copy = new_node(cur->val);
                buckets[i] = copy;

                // 针对同一个bucket list，复制每一个节点
                for (node* next = cur->next; next; 
                    cur = next, next = cur->next) {
                    copy->next = new_node(next->val);
                    copy = copy->next;
                }
            }
        }
        num_elements = ht.num_elements;     // 重新登录节点个数（hashtable的大小）
    }
    catch (const std::exception& e) {
        clear();
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator,
     typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::iterator> 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::equal_range(const key_type& key)
{
    typedef pair<iterator, iterator> Pii;
    const size_type n = bkt_num_key(key);

    for (node* first = buckets[n]; first; first = first->next)
        if (equals(get_key(first->val), key)) {
        for (node* cur = first->next; cur; cur = cur->next)
            if (!equals(get_key(cur->val), key))
            return Pii(iterator(first, this), iterator(cur, this));
        for (size_type m = n + 1; m < buckets.size(); ++m)
            if (buckets[m])
            return Pii(iterator(first, this),
                        iterator(buckets[m], this));
        return Pii(iterator(first, this), end());
        }
    return Pii(end(), end());
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::const_iterator, 
     typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::const_iterator> 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::equal_range(const key_type& key) const
{
    typedef pair<const_iterator, const_iterator> Pii;
    const size_type n = bkt_num_key(key);

    for (const node* first = buckets[n] ;
        first; 
        first = first->next) {
        if (equals(get_key(first->val), key)) {
            for (const node* cur = first->next;
                cur;
                cur = cur->next)
                if (!equals(get_key(cur->val), key))
                return Pii(const_iterator(first, this),
                            const_iterator(cur, this));
            for (size_type m = n + 1; m < buckets.size(); ++m)
                if (buckets[m])
                return Pii(const_iterator(first, this),
                            const_iterator(buckets[m], this));
            return Pii(const_iterator(first, this), end());
        }
    }
    return Pii(end(), end());
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::size_type 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::erase(const key_type& key)
{
    const size_type n = bkt_num_key(key);
    node* first = buckets[n];
    size_type erased = 0;

    if (first) {
        node* cur = first;
        node* next = cur->next;
        while (next) {
            if (equals(get_key(next->val), key)) {
                cur->next = next->next;
                delete_node(next);
                next = cur->next;
                ++erased;
                --num_elements;
            }
            else {
                cur = next;
                next = cur->next;
            }
        }
        if (equals(get_key(first->val), key)) {
            buckets[n] = first->next;
            delete_node(first);
            ++erased;
            --num_elements;
        }
    }
    return erased;
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::erase(const iterator& it)
{
    node* p = it.cur;
    if (p) {
        const size_type n = bkt_num(p->val);
        node* cur = buckets[n];

        if (cur == p) {
            buckets[n] = cur->next;
            delete_node(cur);
            --num_elements;
        }
        else {
            node* next = cur->next;
            while (next) {
                if (next == p) {
                    cur->next = next->next;
                    delete_node(next);
                    --num_elements;
                    break;
                }
                else {
                    cur = next;
                    next = cur->next;
                }
            }
        }
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>
  ::erase(iterator first, iterator last)
{
    size_type __f_bucket = first.cur ? 
        bkt_num(first.cur->val) : buckets.size();
    size_type __l_bucket = last.cur ? 
        bkt_num(last.cur->val) : buckets.size();

    if (first.cur == last.cur)
        return;
    else if (__f_bucket == __l_bucket)
        erase_bucket(__f_bucket, first.cur, last.cur);
    else {
        erase_bucket(__f_bucket, first.cur, 0);
        for (size_type n = __f_bucket + 1; n < __l_bucket; ++n)
            erase_bucket(n, 0);
        if (__l_bucket != buckets.size())
            erase_bucket(__l_bucket, last.cur);
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
inline void
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::erase(const_iterator first,
                                             const_iterator last)
{
    erase(iterator(const_cast<node*>(first.cur),
                    const_cast<hashtable*>(first.ht)),
            iterator(const_cast<node*>(last.cur),
                    const_cast<hashtable*>(last.ht)));
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
inline void
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::erase(const const_iterator& it)
{
    erase(iterator(const_cast<node*>(it.cur),
                    const_cast<hashtable*>(it.ht)));
}


template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>
  ::erase_bucket(const size_type n, node* first, node* last)
{
    node* cur = buckets[n];
    if (cur == first)
        erase_bucket(n, last);
    else {
        node* next;
        for (next = cur->next; 
            next != first; 
            cur = next, next = cur->next)
        ;
        while (next != last) {
            cur->next = next->next;
            delete_node(next);
            next = cur->next;
            --num_elements;
        }
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>
  ::erase_bucket(const size_type n, node* last)
{
    node* cur = buckets[n];
    while (cur != last) {
        node* next = cur->next;
        delete_node(cur);
        cur = next;
        buckets[n] = cur;
        --num_elements;
    }
}

template <class Value, class Key, class HashFcn, class ExtractKey, class EqualKey>
typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::reference 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey>::find_or_insert(const value_type& obj)
{
    resize(num_elements + 1);

    size_type n = bkt_num(obj);
    node* first = buckets[n];

    for (node* cur = first; cur; cur = cur->next)
        if (equals(get_key(cur->val), get_key(obj)))
            return cur->val;

    node* tmp = new_node(obj);
    tmp->next = first;
    buckets[n] = tmp;
    ++num_elements;
    return tmp->val;
}

#endif