#ifndef __TINY_ALLOC_H
#define __TINY_ALLOC_H

#include <cstddef>
#include <malloc.h>

using namespace std;
template<class T>
class simple_alloc {

public:
    static T* allocate(size_t n) {
        return 0 == n ? 0 : (T*) malloc(n * sizeof (T));
    }
    static T* allocate(void) {
        return (T*) malloc(sizeof (T));
    }
    static void deallocate(T* p, size_t n) {
        if(n != 0)
            free(p);
    }
    static void deallocate(T* p) {
        free(p);
    }
};

#endif