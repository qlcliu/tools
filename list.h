#ifndef __LIST_H__
#define __LIST_H__

/*
 *    struct leaf
 *    {
 *        TLink<leaf> leafLink;
 *        char color;
 *    };
 *
 *    LIST_DECLARE(leaf, leafLink) leaves;
 *
 *    leaf* blue = new leaf;
 *    blue->color = 'b';
 *    leaves.InsertTail(blue);
 *
 *    delete blue;
 *    assert(leaves.Empty());
 */

#define LIST_DECLARE(T, link) TListDeclare<T, offsetof(T, link)>

template<typename T>
class TLink {
public:
    TLink(size_t offset = 0);
    ~TLink();

    bool IsLinked() const;
    void Unlink();

    T* Prev();
    T* Next();
    const T* Prev() const;
    const T* Next() const;

    void SetOffset(size_t offset);
    TLink<T>* NextLink();
    TLink<T>* PrevLink();
    void InsertBefore(T* node, TLink<T>* nextLink);
    void InsertAfter(T* node, TLink<T>* prevLink);

private:
    TLink(const TLink&);
    TLink& operator=(const TLink&);
    void RemoveFromList();

    T* m_nextNode;
    TLink<T>* m_prevLink;
};

template<typename T>
class TList {
public:
    TList();
    ~TList();

    bool Empty() const;
    void UnlinkAll();
    void DeleteAll();

    T* Head();
    T* Tail();
    const T* Head() const;
    const T* Tail() const;

    T* Prev(T* node);
    T* Next(T* node);
    const T* Prev(const T* node) const;
    const T* Next(const T* node) const;

    void InsertHead(T* node);
    void InsertTail(T* node);
    void InsertBefore(T* node, T* before);
    void InsertAfter(T* node, T* after);

private:
    template<typename T, size_t offset> friend class TListDeclare;

    TList(size_t offset);
    TList(const TList &);
    TList & operator=(const TList &);
    TLink<T>* GetLinkFromNode(const T* node) const;

    TLink<T> m_link;
    size_t m_offset;
};

template<typename T, size_t offset>
class TListDeclare : public TList<T> {
public:
    TListDeclare();
};

template<typename T>
TLink<T>::TLink(size_t offset = 0) {
    m_nextNode = (T*)((size_t)this + 1 - offset);
    m_prevLink = this;
}

template<typename T>
TLink<T>::~TLink() {
    RemoveFromList();
}

template<typename T>
void TLink<T>::SetOffset(size_t offset) {
    m_nextNode = (T*)((size_t)this + 1 - offset);
    m_prevLink = this;
}

template<typename T>
TLink<T>* TLink<T>::NextLink() {
    size_t offset = (size_t)this - ((size_t)m_prevLink->m_nextNode & ~1);
    return (TLink<T>*)(((size_t)m_nextNode & ~1) + offset);
}

template<typename T>
void TLink<T>::RemoveFromList() {
    NextLink()->m_prevLink = m_prevLink;
    m_prevLink->m_nextNode = m_nextNode;
}

template<typename T>
void TLink<T>::InsertBefore(T* node, TLink<T>* nextLink) {
    RemoveFromList();

    m_prevLink = nextLink->m_prevLink;
    m_nextNode = m_prevLink->m_nextNode;

    nextLink->m_prevLink->m_nextNode = node;
    nextLink->m_prevLink = this;
}

template<typename T>
void TLink<T>::InsertAfter(T* node, TLink<T>* prevLink) {
    RemoveFromList();

    m_prevLink = prevLink;
    m_nextNode = prevLink->m_nextNode;

    prevLink->NextLink()->m_prevLink = this;
    prevLink->m_nextNode = node;
}

template<typename T>
bool TLink<T>::IsLinked() const {
    return m_prevLink != this;
}

template<typename T>
void TLink<T>::Unlink() {
    RemoveFromList();

    m_nextNode = (T*)((size_t) this + 1);
    m_prevLink = this;
}

template<typename T>
TLink<T>* TLink<T>::PrevLink() {
    return m_prevLink;
}

template<typename T>
T* TLink<T>::Prev() {
    T* prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t)prevNode & 1) {
        return NULL;
    }

    return prevNode;
}

template<typename T>
const T* TLink<T>::Prev() const {
    const T* prevNode = m_prevLink->m_prevLink->m_nextNode;
    if ((size_t)prevNode & 1) {
        return NULL;
    }

    return prevNode;
}

template<typename T>
T* TLink<T>::Next() {
    if ((size_t)m_nextNode & 1) {
        return NULL;
    }

    return m_nextNode;
}

template<typename T>
const T* TLink<T>::Next() const {
    if ((size_t)m_nextNode & 1) {
        return NULL;
    }

    return m_nextNode;
}

template<typename T>
TList<T>::TList() : m_link(), m_offset((size_t)-1) {

}

template<typename T>
TList<T>::TList(size_t offset) : m_link(offset), m_offset(offset) {

}

template<typename T>
TList<T>::~TList() {
    UnlinkAll();
}

template<typename T>
bool TList<T>::Empty() const {
    return m_link.Next() == NULL;
}

template<typename T>
void TList<T>::UnlinkAll() {
    for (;;) {
        TLink<T>* link = m_link.PrevLink();
        if (link == &m_link) {
            break;
        }
        link->Unlink();
    }
}

template<typename T>
void TList<T>::DeleteAll() {
    while (T* node = m_link.Next()) {
        delete node;
    }
}

template<typename T>
T* TList<T>::Head() {
    return m_link.Next();
}

template<typename T>
T* TList<T>::Tail() {
    return m_link.Prev();
}

template<typename T>
const T* TList<T>::Head() const {
    return m_link.Next();
}

template<typename T>
const T* TList<T>::Tail() const {
    return m_link.Prev();
}

template<typename T>
T* TList<T>::Prev(T* node) {
    return GetLinkFromNode(node)->Prev();
}

template<typename T>
const T* TList<T>::Prev(const T* node) const {
    return GetLinkFromNode(node)->Prev();
}

template<typename T>
T* TList<T>::Next(T* node) {
    return GetLinkFromNode(node)->Next();
}

template<typename T>
const T* TList<T>::Next(const T* node) const {
    return GetLinkFromNode(node)->Next();
}

template<typename T>
void TList<T>::InsertHead(T* node) {
    InsertAfter(node, NULL);
}

template<typename T>
void TList<T>::InsertTail(T* node) {
    InsertBefore(node, NULL);
}

template<typename T>
void TList<T>::InsertBefore(T* node, T* before) {
    ASSERT(!((size_t)node & 1));
    GetLinkFromNode(node)->InsertBefore(
        node,
        before ? GetLinkFromNode(before) : &m_link);
}

template<typename T>
void TList<T>::InsertAfter(T* node, T* after) {
    ASSERT(!((size_t)node & 1));
    GetLinkFromNode(node)->InsertAfter(
        node,
        after ? GetLinkFromNode(after) : &m_link);
}

template<typename T>
TLink<T>* TList<T>::GetLinkFromNode(const T* node) const {
    ASSERT(m_offset != (size_t)-1);
    return (TLink<T>*)((size_t)node + m_offset);
}

template<typename T, size_t offset>
TListDeclare<T, offset>::TListDeclare() : TList<T>(offset) {

}
#endif
