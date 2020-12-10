#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <vector>
#include <cassert>

template<typename T>
class Array {
public:
    enum {
        PAGE_BITS = 8,
        PAGE_SIZE = 1 << PAGE_BITS
    };

    Array();

    ~Array();

    void Append(const T* elems, size_t size);

    size_t size() const;

    T& operator[](size_t idx);

private:
    size_t _NeedPages(size_t size) const;

    void _AddPages(size_t cnt, const T* elems, size_t size);

    void _UpdateUsed(size_t size);

private:
    std::vector<T*> _maps;
    size_t _used;
};

template<typename T>
Array<T>::Array() : _used(0) {

}

template<typename T>
Array<T>::~Array() {
    for (typename std::vector<T*>::iterator it = _maps.begin(); it != _maps.end(); ++it) {
        delete [] (*it);
    }
}

template<typename T>
void Array<T>::Append(const T* elems, size_t size) {
    if (_maps.empty()) {
        _AddPages(_NeedPages(size), elems, size);
        _UpdateUsed(size);
    } else {
        while (_used < PAGE_SIZE && size > 0) {
            *(_maps.back() + _used) = *elems;
            ++_used;
            ++elems;
            --size;
        }

        _AddPages(_NeedPages(size), elems, size);
        _UpdateUsed(size);
    }
}

template<typename T>
size_t Array<T>::size() const {
    return _maps.empty() ? 0 : (_maps.size() - 1) * PAGE_SIZE + _used;
}

template<typename T>
T& Array<T>::operator[](size_t idx) {
    assert(idx < size());
    return _maps[idx >> PAGE_BITS][idx & (PAGE_SIZE - 1)];
}

template<typename T>
size_t Array<T>::_NeedPages(size_t size) const {
    size_t needPages = size >> PAGE_BITS;
    if (size & (PAGE_SIZE - 1)) {
        ++needPages;
    }
    return needPages;
}

template<typename T>
void Array<T>::_AddPages(size_t cnt, const T* elems, size_t size) {
    for (size_t i = 0; i < cnt; ++i) {
        T* newPage = new T[PAGE_SIZE];
        size_t len = (size >> PAGE_BITS) ? PAGE_SIZE : (size & (PAGE_SIZE - 1));
        std::copy(elems, elems + len, newPage);
        elems += len;
        size -= len;
        _maps.push_back(newPage);
    }
}

template<typename T>
void Array<T>::_UpdateUsed(size_t size) {
    if ((_used + size) & (PAGE_SIZE - 1)) {
        _used = (_used + size) & (PAGE_SIZE - 1);
    } else {
        _used = PAGE_SIZE;
    }
}
#endif
