#ifndef __TINY_HASH_MAP_H
#define __TINY_HASH_MAP_H

#include <functional>
#include "tiny_hashtable.h"

template <class Key, class T, class HashFcn = hash<Key>, class EqualKey = equal_to<Key>>
class hash_map;

template <class Key, class T, class HashFcn, class EqualKey>
inline bool operator==(const hash_map<Key, T, HashFcn, EqualKey> &,
                       const hash_map<Key, T, HashFcn, EqualKey> &);

template <class Key, class T, class HashFcn, class EqualKey>
class hash_map
{
    private:
        typedef hashtable<pair<const Key,T>,Key,HashFcn,_Select1st<pair<const Key,T> >,EqualKey> ht;
        ht rep;

    public:
        typedef typename ht::key_type key_type;
        typedef T data_type;
        typedef T mapped_type;
        typedef typename ht::value_type value_type;
        typedef typename ht::hasher hasher;
        typedef typename ht::key_equal key_equal;
        
        typedef typename ht::size_type size_type;
        typedef typename ht::difference_type difference_type;
        typedef typename ht::pointer pointer;
        typedef typename ht::const_pointer const_pointer;
        typedef typename ht::reference reference;
        typedef typename ht::const_reference const_reference;

        typedef typename ht::iterator iterator;
        typedef typename ht::const_iterator const_iterator;

        hasher hash_funct() const { return rep.hash_funct(); }
        key_equal key_eq() const { return rep.key_eq(); }

    public:
        // 缺省使用大小为100的表格，将由hash table调整为最接近且较大之质数
        hash_map() : rep(100, hasher(), key_equal()) {}
        explicit hash_map(size_type n) : rep(n, hasher(), key_equal()) {}
        hash_map(size_type n, const hasher& hf) : rep(n, hf, key_equal()) {}
        hash_map(size_type n, const hasher &hf, const key_equal &eql)
            : rep(n, hf, eql) {}

        // 以下，插入操作全部使用insert_unique()，不允许键值重复
        template <class InputIterator>
        hash_map(InputIterator f, InputIterator l)
            : rep(100, hasher(), key_equal()) { rep.insert_unique(f, l); }
        template <class InputIterator>
        hash_map(InputIterator f, InputIterator l, size_type n)
            : rep(n, hasher(), key_equal()) { rep.insert_unique(f, l); }
        template <class InputIterator>
        hash_map(InputIterator f, InputIterator l, size_type n, const hasher& hf)
            : rep(n, hf, key_equal()) { rep.insert_unique(f, l); }
        template <class InputIterator>
        hash_map(InputIterator f, InputIterator l, size_type n,
                const hasher& hf, const key_equal& eql)
            : rep(n, hf, eql) { rep.insert_unique(f, l); }

    public:
        size_type size() const { return rep.size(); }
        size_type max_size() const { return rep.max_size(); }
        bool empty() const { return rep.empty(); }
        void swap(hash_map& hs) { rep.swap(hs.rep); }
        friend bool operator==<>(const hash_map &, const hash_map &);

        iterator begin() { return rep.begin(); }
        iterator end() { return rep.end(); }
        const_iterator begin() const { return rep.begin(); }
        const_iterator end() const { return rep.end(); }

    public:
        pair<iterator,bool> insert(const value_type& obj)
            { return rep.insert_unique(obj); }
        void insert(const value_type* f, const value_type* l) {
            rep.insert_unique(f, l);
        }
        void insert(const_iterator f, const_iterator l)
            { rep.insert_unique(f, l); }
        pair<iterator,bool> insert_noresize(const value_type& obj)
            { return rep.insert_unique_noresize(obj); }    

        iterator find(const key_type& key) { return rep.find(key); }
        const_iterator find(const key_type& key) const 
            { return rep.find(key); }

        T& operator[](const key_type& key) {
            return rep.find_or_insert(value_type(key, T())).second;
        }

        size_type count(const key_type& key) const { return rep.count(key); }
        
        pair<iterator, iterator> equal_range(const key_type& key)
            { return rep.equal_range(key); }
        pair<const_iterator, const_iterator>
        equal_range(const key_type& key) const
            { return rep.equal_range(key); }

        size_type erase(const key_type& key) {return rep.erase(key); }
        void erase(iterator it) { rep.erase(it); }
        void erase(iterator f, iterator l) { rep.erase(f, l); }
        void clear() { rep.clear(); }

        void resize(size_type hint) { rep.resize(hint); }
        size_type bucket_count() const { return rep.bucket_count(); }
        size_type max_bucket_count() const { return rep.max_bucket_count(); }
        size_type elems_in_bucket(size_type n) const
            { return rep.elems_in_bucket(n); }
};

template <class Key, class T, class HashFcn, class EqualKey>
inline bool operator==(const hash_map<Key, T, HashFcn, EqualKey> &hm1,
                       const hash_map<Key, T, HashFcn, EqualKey> &hm2)
{
  return hm1.rep == hm2.rep;
}

template <class Key, class T, class HashFcn = hash<Key>, class EqualKey = equal_to<Key>>
class hash_multimap;

template <class Key, class T, class HashFcn, class EqualKey>
inline bool operator==(const hash_multimap<Key, T, HashFcn, EqualKey> & hm1,
                       const hash_multimap<Key, T, HashFcn, EqualKey> & hm2);

template <class Key, class T, class HashFcn, class EqualKey>
class hash_multimap
{
    private:
        typedef hashtable<pair<const Key, T>, Key, HashFcn, _Select1st<pair<const Key, T>>, EqualKey> ht;
        ht rep;

    public:
        typedef typename ht::key_type key_type;
        typedef T data_type;
        typedef T mapped_type;
        typedef typename ht::value_type value_type;
        typedef typename ht::hasher hasher;
        typedef typename ht::key_equal key_equal;

        typedef typename ht::size_type size_type;
        typedef typename ht::difference_type difference_type;
        typedef typename ht::pointer pointer;
        typedef typename ht::const_pointer const_pointer;
        typedef typename ht::reference reference;
        typedef typename ht::const_reference const_reference;

        typedef typename ht::iterator iterator;
        typedef typename ht::const_iterator const_iterator;

        hasher hash_funct() const { return rep.hash_funct(); }
        key_equal key_eq() const { return rep.key_eq(); }

    public:
        hash_multimap() : rep(100, hasher(), key_equal()) {}
        explicit hash_multimap(size_type n) : rep(n, hasher(), key_equal()) {}
        hash_multimap(size_type n, const hasher& hf) : rep(n, hf, key_equal()) {}
        hash_multimap(size_type n, const hasher& hf, const key_equal& eql)
            : rep(n, hf, eql) {}

        template <class InputIterator>
        hash_multimap(InputIterator f, InputIterator l) : rep(100, hasher(), key_equal())
            { rep.insert_equal(f, l); }
        template <class InputIterator>
        hash_multimap(InputIterator f, InputIterator l, size_type n) : rep(n, hasher(), key_equal())
            { rep.insert_equal(f, l); }
        template <class InputIterator>
        hash_multimap(InputIterator f, InputIterator l, size_type n, const hasher &hf)
            : rep(n, hf, key_equal()) { rep.insert_equal(f, l); }
        template <class InputIterator>
        hash_multimap(InputIterator f, InputIterator l, size_type n,
                          const hasher &hf, const key_equal &eql)
            : rep(n, hf, eql) { rep.insert_equal(f, l); }
        
        size_type size() const { return rep.size(); }
        size_type max_size() const { return rep.max_size(); }
        bool empty() const { return rep.empty(); }
        void swap(hash_multimap& hs) { rep.swap(hs.rep); }

        friend bool operator==<>(const hash_multimap &, const hash_multimap &);

        iterator begin() { return rep.begin(); }
        iterator end() { return rep.end(); }
        const_iterator begin() const { return rep.begin(); }
        const_iterator end() const { return rep.end(); }

    public:
        iterator insert(const value_type& obj) 
            { return rep.insert_equal(obj); }
        template <class InputIterator>
        void insert(InputIterator f, InputIterator l) 
            { rep.insert_equal(f,l); }
        iterator insert_noresize(const value_type& obj)
            { return rep.insert_equal_noresize(obj); }    

        iterator find(const key_type& key) { return rep.find(key); }
        const_iterator find(const key_type& key) const 
            { return rep.find(key); }

        size_type count(const key_type& key) const { return rep.count(key); }
        
        pair<iterator, iterator> equal_range(const key_type& key)
            { return rep.equal_range(key); }
        pair<const_iterator, const_iterator>
        equal_range(const key_type& key) const
            { return rep.equal_range(key); }

        size_type erase(const key_type& key) {return rep.erase(key); }
        void erase(iterator it) { rep.erase(it); }
        void erase(iterator f, iterator l) { rep.erase(f, l); }
        void clear() { rep.clear(); }

    public:
    void resize(size_type hint) { rep.resize(hint); }
    size_type bucket_count() const { return rep.bucket_count(); }
    size_type max_bucket_count() const { return rep.max_bucket_count(); }
    size_type elems_in_bucket(size_type n) const
        { return rep.elems_in_bucket(n); }
};

template <class Key, class T, class HashFcn, class EqualKey>
inline bool operator==(const hash_multimap<Key, T, HashFcn, EqualKey>& hm1,
           const hash_multimap<Key, T, HashFcn, EqualKey>& hm2)
{
  return hm1.rep == hm2.rep;
}

#endif