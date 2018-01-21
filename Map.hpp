#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <netinet/in.h>
#include <cassert>


#ifndef MAP_HPP
#define MAP_HPP

bool isHeads()
{
    double n=(double)rand()/RAND_MAX;
    return (n>=0.5);
}

std::size_t getRandomHeight()
{
    std::size_t res=1;
    while (isHeads())
        res++;
    return res;
}

const char HEAD='H',TAIL='T';
template<typename Key_T, typename Mapped_T>
class Map {
    typedef std::pair<Key_T, Mapped_T> ValueType;
    //=======================SKIPNODE CLASS===================================
    class SkipNode {
    public:
        char type;
        std::size_t height;
        std::vector<SkipNode *> forward_ptrs;
        SkipNode *prev;
        explicit SkipNode(std::size_t h) : type('\0'), height(h), prev(0) {
            for (int i = 0; i < height; i++)
                forward_ptrs.emplace_back(nullptr);
        }
        explicit SkipNode(char t) : type(t), height(1), prev(0) {}
        SkipNode(const SkipNode &node) : prev(0) {
            type = node.type;
            height = node.height;
            for (int i = 0; i < height; i++)
                forward_ptrs.emplace_back(nullptr);
        }
        virtual ~SkipNode() {}
    };
    class DataNode : public SkipNode {
    public:
        ValueType data;
        explicit DataNode(std::pair<Key_T, Mapped_T> p, std::size_t h) : data(p), SkipNode(h) {}
        DataNode(const DataNode &node) : SkipNode(node), data(node.data) {}
    };


    SkipNode *head;
    SkipNode *tail;
    std::size_t nSize;
public:
    //=================================ITERATOR=======================================================
    class Iterator {
        SkipNode *current;
        explicit Iterator(SkipNode *node) : current(node) {}
    public:
        Iterator &operator++();
        Iterator operator++(int);
        Iterator &operator--();
        Iterator operator--(int);
        ValueType &operator*() const;
        ValueType *operator->() const;
        friend bool operator==(const Iterator &it1, const Iterator &it2) {
            return (it1.current == it2.current);
        }
        friend bool operator!=(const Iterator &it1, const Iterator &it2) {
            return (it1.current != it2.current);
        }
        friend class Map;
    };
    //=================================CONSTITERATOR=======================================================
    class ConstIterator {
        const SkipNode *current;
    public:
        explicit ConstIterator(const SkipNode *node) : current(node) {};
        ConstIterator(const Iterator &);
        ConstIterator &operator++();
        ConstIterator operator++(int);
        ConstIterator &operator--();
        ConstIterator operator--(int);
        const ValueType &operator*() const;
        const ValueType *operator->() const;
        friend bool operator==(const ConstIterator &it1, const ConstIterator &it2) {
            return (it1.current == it2.current);
        }
        friend bool operator==(const ConstIterator &it1, const Iterator &it2) {
            return (it1.current == it2.current);
        }
        friend bool operator==(const Iterator &it1, const ConstIterator &it2) {
            return (it1.current == it2.current);
        }
        friend bool operator!=(const ConstIterator &it1, const ConstIterator &it2) {
            return (it1.current != it2.current);
        }
        friend bool operator!=(const ConstIterator &it1, const Iterator &it2) {
            return (it1.current != it2.current);
        }
        friend bool operator!=(const Iterator &it1, const ConstIterator &it2) {
            return (it1.current != it2.current);
        }
    };

    //=================================REVERSEITERATOR=======================================================
    class ReverseIterator {
        SkipNode *current;
        explicit ReverseIterator(SkipNode *node) : current(node) {}
    public:
        ReverseIterator &operator++();
        ReverseIterator operator++(int);
        ReverseIterator &operator--();
        ReverseIterator operator--(int);
        ValueType &operator*() const;
        ValueType *operator->() const;
        friend bool operator==(const ReverseIterator &it1, const ReverseIterator &it2) {
            return (it1.current == it2.current);
        }
        friend bool operator!=(const ReverseIterator &it1, const ReverseIterator &it2) {
            return (it1.current != it2.current);
        }
        friend class Map;
    };

    //=================================MAP CONSTRUCTORS=======================================================
    Map();
    Map(const Map &);
    Map &operator=(const Map &);
    Map(std::initializer_list<std::pair<const Key_T, Mapped_T>>);
    ~Map();

    //=================================SIZE OPERATORS=========================================================
    size_t size() const;
    bool empty() const;

    //=================================ITERATOR OPERATIONS====================================================
    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;
    ReverseIterator rbegin();
    ReverseIterator rend();

    //=================================ELEMENT ACCESS=======================================================
    Iterator find(const Key_T &);
    ConstIterator find(const Key_T &) const;
    Mapped_T &at(Key_T);
    const Mapped_T &at(const Key_T &) const;
    Mapped_T &operator[](const Key_T &);

    //==============================MODIFIERS===============================================================
    std::pair<Iterator, bool> insert(const std::pair<Key_T, Mapped_T> &);
    template <typename IT_T> void insert(IT_T range_beg, IT_T range_end);
    void erase(const Key_T &);
    void erase(Iterator pos);
    void clear();

    //=================================COMPARISON====================================================
    template <typename K,typename M>
    friend bool operator==(const Map &, const Map &);
    template <typename K,typename M>
    friend bool operator!=(const Map &, const Map &);
    template <typename K,typename M>
    friend bool operator<(const Map &, const Map &);

    //=================================HELPERS====================================================
private:
    SkipNode *findNode(const Key_T&) const;
    void addEmptyLayer();
};

//**************************************IMPLEMENTATION****************************************************

//=================================MAP CONSTRUCTORS=======================================================

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T>::Map():nSize(0) {
    head = new SkipNode(HEAD);
    tail = new SkipNode(TAIL);
    head->forward_ptrs.push_back(tail);
    tail->forward_ptrs.emplace_back(nullptr);
}

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T>::Map(const Map &map) {
    Map<Key_T, Mapped_T>::SkipNode *orgl = map.head, *curr = orgl, *tmp;

    while (curr) {
        tmp = curr->forward_ptrs[0];

        curr->forward_ptrs[0] = (curr->type == '\0') ? new DataNode(*static_cast<DataNode *>(curr))
                                                     : new SkipNode(*curr);
        curr->forward_ptrs[0]->forward_ptrs[0] = tmp;
        curr = tmp;
    }

    curr = orgl;


    while (curr) {
        for (int i = 1; i < curr->forward_ptrs.size(); i++) {
            curr->forward_ptrs[0]->forward_ptrs[i] = curr->forward_ptrs[i] ? curr->forward_ptrs[i]->forward_ptrs[0]
                                                                           : curr->forward_ptrs[i];
        }
        curr->forward_ptrs[0]->prev = curr->prev ? curr->prev->forward_ptrs[0] : curr->prev;
        curr = curr->forward_ptrs[0] ? curr->forward_ptrs[0]->forward_ptrs[0] : curr->forward_ptrs[0];
    }

    Map<Key_T, Mapped_T>::SkipNode *org = orgl, *copy = orgl->forward_ptrs[0];

    tmp = copy;
    while (org && copy) {
        org->forward_ptrs[0] = org->forward_ptrs[0] ?
                               org->forward_ptrs[0]->forward_ptrs[0] : org->forward_ptrs[0];
        copy->forward_ptrs[0] = copy->forward_ptrs[0] ?
                                copy->forward_ptrs[0]->forward_ptrs[0] : copy->forward_ptrs[0];
        org = org->forward_ptrs[0];
        if (copy->type == TAIL)
            this->tail = copy;
        copy = copy->forward_ptrs[0];
    }
    nSize = map.nSize;
    head = tmp;

}

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T>::Map(std::initializer_list<std::pair<const Key_T, Mapped_T>> list):Map() {
    nSize=0;
    auto *start = list.begin();
    for (; start != list.end(); start++) {
        insert(*(start));
    }
}

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T> &Map<Key_T, Mapped_T>::operator=(const Map<Key_T, Mapped_T> &map) {
    if(*this==map)
    {
        return *this;
    }
    Map<Key_T, Mapped_T>::SkipNode *orgl = map.head, *curr = orgl, *tmp;
    clear();
    delete head;
    delete tail;
    while (curr) {
        tmp = curr->forward_ptrs[0];

        curr->forward_ptrs[0] = (curr->type == '\0') ? new DataNode(*static_cast<DataNode *>(curr))
                                                     : new SkipNode(*curr);
        curr->forward_ptrs[0]->forward_ptrs[0] = tmp;
        curr = tmp;
    }

    curr = orgl;


    while (curr) {
        for (int i = 1; i < curr->forward_ptrs.size(); i++) {
            curr->forward_ptrs[0]->forward_ptrs[i] = curr->forward_ptrs[i] ? curr->forward_ptrs[i]->forward_ptrs[0]
                                                                           : curr->forward_ptrs[i];
        }
        curr->forward_ptrs[0]->prev = curr->prev ? curr->prev->forward_ptrs[0] : curr->prev;
        curr = curr->forward_ptrs[0] ? curr->forward_ptrs[0]->forward_ptrs[0] : curr->forward_ptrs[0];
    }

    Map<Key_T, Mapped_T>::SkipNode *org = orgl, *copy = orgl->forward_ptrs[0];

    tmp = copy;
    while (org && copy) {
        org->forward_ptrs[0] = org->forward_ptrs[0] ?
                               org->forward_ptrs[0]->forward_ptrs[0] : org->forward_ptrs[0];
        copy->forward_ptrs[0] = copy->forward_ptrs[0] ?
                                copy->forward_ptrs[0]->forward_ptrs[0] : copy->forward_ptrs[0];
        org = org->forward_ptrs[0];
        if (copy->type == TAIL)
            tail = copy;
        copy = copy->forward_ptrs[0];
    }

    head = tmp;
    nSize = map.nSize;
    return *this;
}

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T>::~Map() {
    SkipNode *tmp;
    while (head != tail) {
        tmp = head;
        head = head->forward_ptrs[0];
        delete tmp;
    }
    delete tail;
}

//=======================================SIZE OPERATORS====================================================

template<typename Key_T, typename Mapped_T>
size_t Map<Key_T, Mapped_T>::size() const {
    return nSize;
}

template<typename Key_T, typename Mapped_T>
bool Map<Key_T, Mapped_T>::empty() const {
    return (nSize == 0);
}

//=================================ITERATOR OPERATIONS====================================================

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator Map<Key_T, Mapped_T>::begin() {
    Map<Key_T, Mapped_T>::Iterator iterator(head->forward_ptrs[0]);
    return iterator;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator Map<Key_T, Mapped_T>::end() {
    Map<Key_T, Mapped_T>::Iterator iterator(tail);
    return iterator;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator Map<Key_T, Mapped_T>::begin() const {
    return ConstIterator(head->forward_ptrs[0]);
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator Map<Key_T, Mapped_T>::end() const {
    return ConstIterator(tail);
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator Map<Key_T, Mapped_T>::rbegin() {
    return ReverseIterator(tail->prev);
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator Map<Key_T, Mapped_T>::rend() {
    return Map::ReverseIterator(head);
}

//=================================ELEMENT ACCESS=======================================================

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator Map<Key_T, Mapped_T>::find(const Key_T &key) {
    auto *pos = findNode(key);
    if (pos) {
        return Iterator(pos);
    }
    return end();
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator Map<Key_T, Mapped_T>::find(const Key_T &key) const {
    auto *pos = findNode(key);
    if (pos) {
        return ConstIterator(pos);
    }
    return end();
}

template<typename Key_T, typename Mapped_T>
Mapped_T &Map<Key_T, Mapped_T>::at(Key_T key) {
    auto *pos = findNode(key);
    if (pos) {
        return static_cast<DataNode *>(pos)->data.second;
    }
    throw std::out_of_range("Not Found");
}

template<typename Key_T, typename Mapped_T>
const Mapped_T &Map<Key_T, Mapped_T>::at(const Key_T &key) const {
    auto *pos = findNode(key);
    if (pos) {
        return static_cast<const DataNode *>(pos)->data.second;
    }
    throw std::out_of_range("Not Found");
}

template<typename Key_T, typename Mapped_T>
Mapped_T &Map<Key_T,Mapped_T>::operator[](const Key_T &key) {
    auto *pos = findNode(key);
    if(pos)
    {
        return static_cast<DataNode *>(pos)->data.second;
    }
    Mapped_T obj{};
    auto p=insert(std::pair<Key_T,Mapped_T>(key,obj));
    return (*(p.first)).second;
}

//==============================MODIFIERS===============================================================

template<typename Key_T, typename Mapped_T>
std::pair<typename Map<Key_T,Mapped_T>::Iterator, bool> Map<Key_T, Mapped_T>::insert(const std::pair<Key_T, Mapped_T> &pair) {
    SkipNode *it = head;
    std::size_t randomHeight = getRandomHeight();

    std::vector<SkipNode *> updates;
    for (long curr_ht_index = head->forward_ptrs.size() - 1; curr_ht_index >= 0; curr_ht_index--) {
        while ((it->forward_ptrs[curr_ht_index] != tail) &&
               (static_cast<DataNode *>(it->forward_ptrs[curr_ht_index])->data.first < pair.first)) {
            it = it->forward_ptrs[curr_ht_index];
        }
        if (curr_ht_index < randomHeight) {
            updates.push_back(it);
        }
    }


    if ((it->forward_ptrs[0]->type!=TAIL)&& static_cast<DataNode *>(it->forward_ptrs[0])->data.first == pair.first) {
        std::pair<Map<Key_T,Mapped_T>::Iterator,bool> res(Map<Key_T,Mapped_T>::Iterator(it->forward_ptrs[0]),false);
        return res;
    }

    SkipNode *newNode = new DataNode(pair, randomHeight);
    newNode->prev = updates[updates.size() - 1];
    it->forward_ptrs[0]->prev = newNode;

    if (randomHeight > head->forward_ptrs.size()) {
        for (unsigned long i = head->forward_ptrs.size(); i < randomHeight; ++i) {
            addEmptyLayer();
            head->forward_ptrs[i] = newNode;
            newNode->forward_ptrs[i] = tail;
        }
    }

    std::size_t current_height = updates.size() - 1;
    for (SkipNode *node:updates) {
        newNode->forward_ptrs[current_height] = node->forward_ptrs[current_height];
        node->forward_ptrs[current_height] = newNode;
        current_height--;
    }
    nSize++;

    return std::pair<Map<Key_T,Mapped_T>::Iterator,bool>(Iterator(newNode),true);
}

template <typename Key_T,typename Mapped_T>
template<typename IT_T>
void Map<Key_T,Mapped_T>::insert(IT_T range_beg, IT_T range_end) {
    while(range_beg!=range_end)
    {
        insert(*range_beg);
        range_beg++;
    }
}

template<typename Key_T, typename Mapped_T>
void Map<Key_T, Mapped_T>::erase(const Key_T &key) {
    SkipNode *it = head;
    std::vector<SkipNode *> updates(head->forward_ptrs);
    for (long curr_ht_index = head->forward_ptrs.size() - 1; curr_ht_index >= 0; curr_ht_index--) {
        while ((it->forward_ptrs[curr_ht_index] != tail) &&
               (static_cast<DataNode *>(it->forward_ptrs[curr_ht_index])->data.first < key)) {
            it = it->forward_ptrs[curr_ht_index];
        }
        updates[curr_ht_index] = it;
    }
    it = it->forward_ptrs[0];
    if ((it->type != HEAD) && !(static_cast<DataNode *>(it)->data.first == key)) {
        throw std::out_of_range("Not Found");
    }
    for (int i = 0, ht = updates.size() - 1; i < updates.size(); i++, ht--) {
        if (updates[i]->forward_ptrs[i] != it) {
            break;
        }
        updates[i]->forward_ptrs[i] = it->forward_ptrs[i];
    }
    it->forward_ptrs[0]->prev = it->prev;
    delete it;
    nSize--;
    if (nSize == 0) {
        clear();
    }
}

template<typename Key_T, typename Mapped_T>
void Map<Key_T, Mapped_T>::erase(Map<Key_T, Mapped_T>::Iterator pos) {
    Key_T key = (*pos).first;
    erase(key);
}

template<typename Key_T, typename Mapped_T>
void Map<Key_T, Mapped_T>::clear() {
    SkipNode *tmp;
    while (head != tail) {
        tmp = head;
        head = head->forward_ptrs[0];
        delete tmp;
    }
    delete tail;
    head = new SkipNode(HEAD);
    tail = new SkipNode(TAIL);
    head->forward_ptrs.push_back(tail);
    tail->forward_ptrs.emplace_back(nullptr);
    nSize = 0;
}

//=============================================COMPARISON=======================================

template<typename Key_T, typename Mapped_T>
bool operator==(const Map<Key_T,Mapped_T> &map1, const Map<Key_T,Mapped_T> &map2) {
    if(map1.size()!=map2.size())
    {
        return false;
    }

    auto map1_it=map1.begin();
    auto map2_it=map2.begin();

    for(;map1_it!=map1.end();map1_it++,map2_it++)
    {
        if((*map1_it)!=(*map2_it))
        {
            return false;
        }
    }
    return true;
}

template<typename Key_T, typename Mapped_T>
bool operator!=(const Map<Key_T,Mapped_T> &map1, const Map<Key_T,Mapped_T> &map2) {
    return !(map1==map2);
}

template<typename Key_T, typename Mapped_T>
bool operator<(const Map<Key_T,Mapped_T> &map1, const Map<Key_T,Mapped_T> &map2) {
    auto map1_it=map1.begin();
    auto map2_it=map2.begin();
    while((map1_it!=map1.end()&&map2_it!=map2.end()))
    {
        if((*map1_it)>(*map2_it))
        {
            return false;
        }
        else if((*map1_it)<(*map2_it))
        {
            return true;
        }
        map1_it++; map2_it++;
    }
    return map1.size() < map2.size();
}


//==================================ITERATORS IMPLEMENTATION=============================================

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator &Map<Key_T, Mapped_T>::ReverseIterator::operator++() {
    current = current->prev;
    return *this;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator Map<Key_T, Mapped_T>::ReverseIterator::operator++(int n) {
    ReverseIterator prev(current);
    current = current->prev;
    return prev;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator &Map<Key_T, Mapped_T>::ReverseIterator::operator--() {
    current = current->forward_ptrs[0];
    return *this;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ReverseIterator Map<Key_T, Mapped_T>::ReverseIterator::operator--(int n) {
    ReverseIterator prev(current);
    current = current->forward_ptrs[0];
    return prev;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ValueType &Map<Key_T, Mapped_T>::ReverseIterator::operator*() const {
    return static_cast<DataNode *>(current)->data;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ValueType *Map<Key_T, Mapped_T>::ReverseIterator::operator->() const {
    std::pair<Key_T, Mapped_T> *pair = &static_cast<DataNode *>(current)->data;
    return pair;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator Map<Key_T, Mapped_T>::ConstIterator::operator++(int n) {
    ConstIterator prev(current);
    current = current->forward_ptrs[0];
    return prev;
}

template<typename Key_T, typename Mapped_T>
Map<Key_T, Mapped_T>::ConstIterator::ConstIterator(const Map<Key_T, Mapped_T>::Iterator &it) {
    current = it.current;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator &Map<Key_T, Mapped_T>::ConstIterator::operator++() {
    current = current->forward_ptrs[0];
    return *this;;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator &Map<Key_T, Mapped_T>::ConstIterator::operator--() {
    current = current->prev;
    return *this;;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ConstIterator Map<Key_T, Mapped_T>::ConstIterator::operator--(int n) {
    const SkipNode *prev = current;
    current = current->prev;
    return ConstIterator(prev);
}

template<typename Key_T, typename Mapped_T>
const typename Map<Key_T, Mapped_T>::ValueType &Map<Key_T, Mapped_T>::ConstIterator::operator*() const {
    return static_cast<const DataNode *>(current)->data;
}

template<typename Key_T, typename Mapped_T>
const typename Map<Key_T, Mapped_T>::ValueType *Map<Key_T, Mapped_T>::ConstIterator::operator->() const {
    const std::pair<Key_T, Mapped_T> *pair = &static_cast<const DataNode *>(current)->data;
    return pair;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator &Map<Key_T, Mapped_T>::Iterator::operator++() {
    current = current->forward_ptrs[0];
    return *this;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator Map<Key_T, Mapped_T>::Iterator::operator++(int n) {
    SkipNode *previous = current;
    current = current->forward_ptrs[0];
    return Iterator(previous);
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator &Map<Key_T, Mapped_T>::Iterator::operator--() {
    current = current->prev;
    return *this;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::Iterator Map<Key_T, Mapped_T>::Iterator::operator--(int) {
    SkipNode *prev = current;
    current = current->prev;
    return Iterator(prev);
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ValueType &Map<Key_T, Mapped_T>::Iterator::operator*() const {
    return static_cast<DataNode *>(current)->data;
}

template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::ValueType *Map<Key_T, Mapped_T>::Iterator::operator->() const {
    std::pair<Key_T, Mapped_T> *pair = &static_cast<DataNode *>(current)->data;
    return pair;
}

//============================================HELPERS===================================================
template<typename Key_T, typename Mapped_T>
typename Map<Key_T, Mapped_T>::SkipNode *Map<Key_T, Mapped_T>::findNode(const Key_T & key) const {
    SkipNode *it = head;

    for (long curr_ht_index = head->forward_ptrs.size() - 1; curr_ht_index >= 0; curr_ht_index--) {
        while ((it->forward_ptrs[curr_ht_index] != tail) &&
               (static_cast<DataNode *>(it->forward_ptrs[curr_ht_index])->data.first < key)) {
            it = it->forward_ptrs[curr_ht_index];
        }
    }

    it = it->forward_ptrs[0];

    if ((it != tail) && static_cast<DataNode *>(it)->data.first == key) {
        return it;
    } else
        return nullptr;
}

template<typename Key_T, typename Mapped_T>
void Map<Key_T, Mapped_T>::addEmptyLayer() {
    head->forward_ptrs.emplace_back(nullptr);
    head->height++;
    tail->forward_ptrs.emplace_back(nullptr);
    tail->height++;
}



#endif