#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <stdlib.h>
#include <new.h>

template<int base>
class Allocator {
public:
    static void* alloc(size_t size);
    static void free(void* p);

private:
    enum {
        SIZE_TYPE = 16,
        FILL_CNT = 20
    };

    union obj {
        obj* _next;
        char data[1];
    };

    static size_t _round_up(size_t size);
    static size_t _locate(size_t size);
    static size_t _demodulate(void*& p);
    static void* _modulate(void* p, size_t size);
    static void* fill_n(size_t unit_size, size_t n);

    static char* _begin;
    static char* _end;
    static obj* _list[SIZE_TYPE];
};

template<int base>
char* Allocator<base>::_begin = 0;

template<int base>
char* Allocator<base>::_end = 0;

template<int base>
typename Allocator<base>::obj* Allocator<base>::_list[SIZE_TYPE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

template<int base>
size_t Allocator<base>::_round_up(size_t size) {
    return (size + base - 1) & ~(base - 1);
}

template<int base>
size_t Allocator<base>::_locate(size_t size) {
    return (size + base - 1) / base - 1;
}

template<int base>
size_t Allocator<base>::_demodulate(void*& p) {
    unsigned char*& cur = (unsigned char*&)p;
    --cur;
    return *cur;
}

template<int base>
void* Allocator<base>::_modulate(void* p, size_t size) {
    unsigned char* cur = (unsigned char*)p;
    unsigned char index = (unsigned char)_locate(size);
    *cur = index >= SIZE_TYPE ? UCHAR_MAX : index;
    ++cur;
    return (void*)cur;
}

template<int base>
void* Allocator<base>::fill_n(size_t unit_size, size_t n) {
    size_t free_size = _end - _begin;
    size_t real_n = n;
    if (n > free_size / unit_size) {
        real_n = free_size / unit_size;
    }

    if (real_n >= 1) {
        size_t idx = _locate(unit_size);

        for (size_t i = 1; i < real_n; ++i) {
            char* cur = _begin + i * unit_size;
            obj* pre = _list[idx];
            _list[idx] = (obj*)cur;
            _list[idx]->_next = pre;
        }

        void* ret = (void*)_begin;
        _begin += real_n * unit_size;

        return _modulate((void*)ret, unit_size);
    } else {
        if (free_size > 0) {
            size_t idx = _locate(free_size);
            obj* pre = _list[idx];
            _list[idx] = (obj*)_begin;
            _list[idx]->_next = pre;
        }

        size_t alloc_size = unit_size * n * 2;
        _begin = (char*)malloc(alloc_size);
        _end = _begin + alloc_size;

        return fill_n(unit_size, n);
    }
}

template<int base>
void* Allocator<base>::alloc(size_t size) {
    size_t real_size = size + 1;
    size_t new_size = _round_up(real_size);
    size_t max_size = base * SIZE_TYPE;
    if (new_size > max_size) {
        void* p = malloc(real_size);
        return _modulate(p, UCHAR_MAX);
    } else {
        size_t idx = _locate(new_size);
        obj* cur = _list[idx];
        if (cur) {
            _list[idx] = _list[idx]->_next;
            return _modulate((void*)cur, new_size);
        } else {
            return fill_n(new_size, FILL_CNT);
        }
    }
}

template<int base>
void Allocator<base>::free(void* p) {
    size_t idx = _demodulate(p);
    if (idx >= SIZE_TYPE) {
        free(p);
    } else {
        obj* pre = _list[idx];
        _list[idx] = (obj*)p;
        _list[idx]->_next = pre;
    }
}
#endif
