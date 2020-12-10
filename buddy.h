#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <stdint.h>
#include <vector>

class Buddy {
public:
    Buddy(uint32_t level);
    uint32_t alloc(uint32_t size);
    void free(uint32_t offset);
    uint32_t size(uint32_t offset);

    static uint32_t npos;

private:
    bool _is_pow_of_2(uint32_t n);
    uint32_t _next_pow_of_2(uint32_t n);
    uint32_t _index_offset(uint32_t index, uint32_t level);
    void _mark_parent(uint32_t index);
    void _combine(uint32_t index);

    enum {
        NODE_UNUSED,
        NODE_USED,
        NODE_SPLIT,
        NODE_FULL
    };

    uint32_t _level;
    std::vector<uint32_t> _tree;
};

uint32_t Buddy::npos = UINT_MAX;

Buddy::Buddy(uint32_t level) : _level(level) {
    uint32_t size = 1 << (level + 1);
    _tree.resize(size, NODE_UNUSED);
}

bool Buddy::_is_pow_of_2(uint32_t n) {
    return !(n & (n - 1));
}

uint32_t Buddy::_next_pow_of_2(uint32_t n) {
    if (_is_pow_of_2(n)) {
        return n;
    }

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    return n + 1;
}

uint32_t Buddy::_index_offset(uint32_t index, uint32_t level) {
    uint32_t left = 0;
    uint32_t length = 1 << (_level - level);
    while (index > 1) {
        if (index & 1) {
            left += length;
        }

        index /= 2;
        length *= 2;
    }

    return left;
}

void Buddy::_mark_parent(uint32_t index) {
    for (;;) {
        uint32_t buddy = index + 1 - (index & 1) * 2;
        if (buddy > 0 && (_tree[buddy] == NODE_USED || _tree[buddy] == NODE_FULL)) {
            index /= 2;
            _tree[index] = NODE_FULL;
        } else {
            break;
        }
    }
}

uint32_t Buddy::alloc(uint32_t size) {
    size = _next_pow_of_2(size);
    uint32_t length = 1 << _level;

    if (size > length) {
        return npos;
    }

    uint32_t index = 1;
    uint32_t level = 0;

    while (index > 0) {
        if (size == length) {
            if (_tree[index] == NODE_UNUSED) {
                _tree[index] = NODE_USED;
                _mark_parent(index);
                return _index_offset(index, level);
            }
        } else {
            switch (_tree[index]) {
            case NODE_USED:
            case NODE_FULL:
                break;
            case NODE_UNUSED:
                _tree[index] = NODE_SPLIT;
                _tree[index * 2] = NODE_UNUSED;
                _tree[index * 2 + 1] = NODE_UNUSED;
            default:
                index = index * 2;
                length /= 2;
                ++level;
                continue;
            }
        }

        if (!(index & 1)) {
            ++index;
            continue;
        }
    }

    return npos;
}

void Buddy::_combine(uint32_t index) {
    while (index > 0) {
        _tree[index] = NODE_UNUSED;
        uint32_t buddy = index + 1 - (index & 1) * 2;
        if (_tree[buddy] == NODE_UNUSED) {
            index /= 2;
            continue;
        }

        break;
    }
}

void Buddy::free(uint32_t offset) {
    uint32_t left = 0;
    uint32_t length = 1 << _level;
    uint32_t index = 1;

    for (;;) {
        switch (_tree[index]) {
        case NODE_USED:
            _combine(index);
            return;
        case NODE_UNUSED:
            return;
        default:
            length /= 2;
            if (offset < left + length) {
                index = index * 2;
            } else {
                left += length;
                index = index * 2 + 1;
            }
            break;
        }
    }
}

uint32_t Buddy::size(uint32_t offset) {
    uint32_t left = 0;
    uint32_t length = 1 << _level;
    uint32_t index = 1;

    for (;;) {
        switch (_tree[index]) {
        case NODE_USED:
            return length;
        case NODE_UNUSED:
            return length;
        default:
            length /= 2;
            if (offset < left + length) {
                index = index * 2;
            } else {
                left += length;
                index = index * 2 + 1;
            }
            break;
        }
    }
}
#endif
