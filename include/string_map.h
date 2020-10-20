#ifndef MAP_H
#define MAP_H

class LinearAllocator;

template<typename T>
class StringMap {
    REFLECT

    static const int maxKeyLength = 64;
    static const int capacity = 64;

    class Entry {
        REFLECT

    public:
        char key[maxKeyLength];
        T* value;
        Entry* next;
    };

    Entry buckets[capacity];
    LinearAllocator* allocator;

    static unsigned long hash(char* str);

public:
    explicit StringMap() = default;
    StringMap(LinearAllocator* allocator);

    //size_t getSize();

    T* get(char* name);
    void set(char* key, T* value);
    void clear();
    //void freeAndClear();
};

/* template<typename T>
REFLECTION_REGISTRATION(StringMap<T>) {
    CLASS->addField("buckets", &StringMap<T>::buckets)
        ->addField("allocator", &StringMap<T>::allocator);
}

template<typename T>
REFLECTION_REGISTRATION(StringMap<T>::Entry) {
    CLASS->addField("key", &Entry::key)
        ->addField("value", &Entry::value)
        ->addField("next", &Entry::next);
} */

#include "linear_allocator.h"

template<typename T>
StringMap<T>::StringMap(LinearAllocator* allocator) : allocator(allocator) {
    for(int i = 0; i < capacity; i++) {
        buckets[i].key[0] = '\0';
    }
}

template<typename T>
unsigned long StringMap<T>::hash(char* str)  {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

    return hash;
}

template<typename T>
T* StringMap<T>::get(char* key) {
    size_t pos = StringMap<T>::hash(key);
    pos = pos % capacity;

    Entry* iter = &buckets[pos];
    do {
        if(!strcmp(key, iter->key)) {
            break;
        }
        
        iter = iter->next;
    } while(iter);
    
    return iter ? iter->value : nullptr;
}

template<typename T>
void StringMap<T>::set(char* key, T* value) {
    size_t pos = StringMap<T>::hash(key);
    assert(*key != '\0');
    pos = pos % capacity;

    Entry* iter = &buckets[pos];
    do {
        if(!strcmp(key, iter->key)) {
            iter->value = value;
            break;
        }

        if(iter->key[0] != '\0' && !iter->next) {
            iter->next = new(*allocator) Entry;
            iter = iter->next;
            iter->key[0] = '\0';
        }
        
        if(iter->key[0] == '\0') {
            for(int i = 0; *key; i++) {
                iter->key[i] = *key++;
            }
            iter->value = value;
            iter->next = nullptr;

            break;
        }
        
        iter = iter->next;
    } while(iter);
}

template<typename T>
void StringMap<T>::clear() {
    for(int i = 0; i < this->capacity; i++) {
        if(this->buckets[i].key[0] != '\0') {
            Entry* it = &this->buckets[i];
            while(it) {
                it->key[0] = '\0';
                destroy(it, *allocator);
                Entry* tmp = it;
                it = it->next;
                tmp->next = nullptr;
            }
        }
    }
}

/*
//TODO: This should really be done from outside the class with some kind of iteration mechanic
template<typename T>
void StringMap<T>::freeAndClear() {
    for(int i = 0; i < this->capacity; i++) {
        if(this->buckets[i].key[0] != '\0') {
            Entry* it = &this->buckets[i];
            while(it) {
                it->key[0] = '\0';
                destroy(it->value, *allocator);
                destroy(it, *allocator);
                Entry* tmp = it;
                it = it->next;
                tmp->next = nullptr;
            }
        }
    }
}*/

#endif