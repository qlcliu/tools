#ifndef __INTERVAL_H__
#define __INTERVAL_H__

#include <stdlib.h>
#include <assert.h>

typedef struct Node {
    unsigned offset;
    Node* parent;
    Node* left;
    Node* right;
} Node;

void create(Node** head);
void insert(Node* head, unsigned pos, unsigned len);
void remove(Node* head, unsigned pos, unsigned len);
void destroy(Node* head);

Node* _left_root(Node* node) {
    while (node->parent && node != node->parent->right) {
        node = node->parent;
    }

    return node->parent;
}

unsigned _position(Node* node) {
    if (node) {
        return node->offset + _position(_left_root(node));
    }

    return 0;
}

Node* _find_node(Node* head, unsigned pos) {
    while (head) {
        if (pos == _position(head)) {
            break;
        } else if (pos < _position(head)) {
            head = head->left;
        } else {
            head = head->right;
        }
    }

    return head;
}

Node* _next_node(Node* node) {
    Node* p = 0;
    if (node->right) {
        p = node->right;
        while (p->left) {
            p = p->left;
        }
    } else if (node->parent) {
        if (node == node->parent->left) {
            p = node->parent;
        } else {
            p = node->parent;
            while (p) {
                if (p->parent && p != p->parent->left) {
                    p = p->parent;
                    continue;
                }
                break;
            }
            p = p && p->parent ? p->parent : 0;
        }
    }

    return p;
}

void _find_interval(Node* head, unsigned pos, Node** lower, Node** upper) {
    Node* start = head;
    while (head && pos >= _position(head)) {
        start = head;
        head = _next_node(head);
    }

    *lower = start;
    *upper = head;
}

void _insert_node(Node* head, unsigned pos) {
    Node* node = 0;
    Node* y = 0;
    Node* x = head;
    assert(head);

    // not split
    if (_find_node(head, pos)) {
        return;
    }

    node = (Node*)malloc(sizeof(Node));
    node->left = 0;
    node->right = 0;

    while (x) {
        y = x;
        if (pos < _position(x)) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    node->parent = y;
    if (pos < _position(y)) {
        y->left = node;
    } else {
        y->right = node;
    }

    node->offset = pos - _position(_left_root(node));
}

void _remove_node(Node* head, unsigned pos) {
    Node* parent = 0;
    Node* child = 0;
    Node* replace = 0;
    unsigned replace_pos = 0;
    Node* node = _find_node(head, pos);

    if (!node) {
        return;
    }

    if (node->left && node->right) {
        replace = node->right;
        while (replace->left) {
            replace = replace->left;
        }
        replace_pos = _position(replace);

        assert(node->parent);
        if (node->parent->left == node) {
            node->parent->left = replace;
        } else {
            node->parent->right = replace;
        }

        child = replace->right;
        parent = replace->parent;
        if (parent != node) {
            if (child) {
                child->parent = parent;
            }
            parent->left = child;
            replace->right = node->right;
            node->right->parent = replace;
        }

        replace->parent = node->parent;
        replace->left = node->left;
        node->left->parent = replace;

        replace->offset = replace_pos - _position(_left_root(replace));
        if (parent != node) {
            child = replace->right;
            while (child) {
                child->offset -= (replace_pos - pos);
                child = child->left;
            }
        }
    } else {
        if (node->left) {
            child = node->left;
        } else {
            child = node->right;
        }
        parent = node->parent;

        if (child) {
            child->parent = parent;
        }

        assert(parent);
        if (parent->left == node) {
            parent->left = child;
        } else {
            parent->right = child;
            child->offset += node->offset;
        }
    }
    free(node);
}

void create(Node** head) {
    assert(head && !*head);
    *head = (Node*)malloc(sizeof(Node));
    memset(*head, 0, sizeof(Node));
}

void insert(Node* head, unsigned pos, unsigned len) {
    Node* lower = 0;
    Node* upper = 0;

    _find_interval(head, pos, &lower, &upper);
    assert(lower);

    if (_position(lower) == pos) {
        if (upper) {
            upper->offset += len;
        } else {
            _insert_node(head, pos + len);
        }
    } else {
        if (upper) {
            _insert_node(head, pos);
            upper->offset += len;
            _insert_node(head, pos + len);
        } else {
            _insert_node(head, pos);
            _insert_node(head, pos + len);
        }
    }
}

void remove(Node* head, unsigned pos, unsigned len) {
    Node* lower = 0;
    Node* upper = 0;
    unsigned upper_pos = 0;

    _find_interval(head, pos, &lower, &upper);
    if (upper) {
        upper_pos = _position(upper);
        if (upper_pos >= pos + len) {
            upper->offset -= len;
        } else {
            _remove_node(head, upper_pos);
            remove(head, pos, len);
        }
    }
}

void destroy(Node* head) {
    if (head) {
        if (head->left) {
            destroy(head->left);
        }

        if (head->right) {
            destroy(head->right);
        }

        free(head);
    }
}
#endif
