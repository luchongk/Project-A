#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "memory.h"
#include "types.h"
#include "strings.h"

//Will use FNV-1a for now
#define FNV1_32A_INIT 0x811c9dc5
#define FNV_32_PRIME 0x01000193

u32 fnv1_32a_hash(u8* to_hash, u64 length)
{
    u32 hash = FNV1_32A_INIT;
    
    u8* iter = (u8*)to_hash;
    u8* end  = iter + length;

    /*
     * FNV-1a hash each octet in the buffer
     */
    while(iter < end) {
        /* xor the bottom with the current octet */
        hash ^= (u32)*iter++;

        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hash *= FNV_32_PRIME;
    }

    return hash;
}

// This hash table does reallocation based on load factor.
// This means pointers to values stored in it are NOT stable.
// eg. If you call put() after get() you should NOT assume the pointer returned by get() still points to valid data.

template<typename K, typename V>
struct HashTable {
    struct Entry {
        K key;
        V value;
        u32 hash;
    };

    s64 allocated = 0;
    s64 count = 0;
    s64 start_size = 32;
    Entry* entries = nullptr;

    Allocator allocator = nullptr;
    void* allocator_data = nullptr;
};

template<typename K>
inline bool keys_are_equal(K key1, K key2) {
    return key1 == key2;
}

inline bool keys_are_equal(String key1, String key2) {
    return !compare(key1, key2);
}

template<typename K>
inline u32 get_hash(K key) {
    u32 hash = fnv1_32a_hash((u8*)&key, sizeof(K));
    if(hash == 0) hash += 2;   //0 is reserved for empty key
    else if(hash == 1) hash++; //1 is reserved for tombstone
    
    return hash;
}

inline u32 get_hash(String key) {
    u32 hash = fnv1_32a_hash(key.data, key.count);
    if(hash == 0) hash += 2;
    else if(hash == 1) hash++;

    return hash;
}

template<typename K, typename V>
V* get(HashTable<K,V>* table, K key) {
    assert(table);

    u32 hash = get_hash(key);

    for(int i = 0; i < table->allocated; i++) {
        int index = (hash + i) & (table->allocated - 1);
        auto entry = &table->entries[index];

        if(entry->hash == 0) {
            return nullptr;
        }
        else if(entry->hash == hash && keys_are_equal(entry->key, key)) {
            return &entry->value;
        }
    }

    assert(!"UNREACHABLE");
    return nullptr;
};

template<typename K, typename V>
void put(HashTable<K,V>* table, K key, V value) {
    assert(table);
    
    if(table->count >= 0.9f * table->allocated) {
        grow(table);
    }

    u32 hash = get_hash(key);

    bool verifying = false;
    for(int i = 0; i < table->allocated; i++) {
        int index = (hash + i) & (table->allocated - 1);
        auto entry = &table->entries[index];

        if(verifying) {
            if(keys_are_equal(entry->key, key)) {
                entry->hash = 0;
                return;
            }
        }
        else {
            if(entry->hash == 0) {
                table->count++;
                entry->hash = hash;
                entry->key = key;
                entry->value = value;
                return;
            }

            if(entry->hash == 1) {
                verifying = true;
                entry->hash = hash;
                entry->key = key;
                entry->value = value;
            }
            
            if(keys_are_equal(entry->key, key)) {
                entry->hash = hash;
                entry->key = key;
                entry->value = value;
                return;
            }
        }
    }

    assert(!"UNREACHABLE");
};

template<typename K, typename V>
bool remove(HashTable<K,V>* table, K key) {
    assert(table);

    u32 hash = get_hash(key);

    for(int i = 0; i < table->allocated; i++) {
        int index = (hash + i) & (table->allocated - 1);
        auto entry = &table->entries[index];

        if(entry->hash == 0) {
            return false;
        }
        else if(entry->hash == hash && keys_are_equal(entry->key, key)) {
            entry->hash = 0;
            table->count--;
            return true;
        }
    }

    assert(!"UNREACHABLE");
    return false;
}

template<typename K, typename V>
void grow(HashTable<K,V>* table) {
    s64 new_size = 2 * table->allocated;
    if(new_size < table->start_size) new_size = table->start_size;

    auto* old_entries = table->entries;
    s64 old_allocated = table->allocated;

    if(table->allocator) {
        table->entries = (HashTable<K,V>::Entry*)table->allocator(
            table->entries,
            0,
            sizeof(HashTable<K,V>::Entry) * new_size,
            table->allocator_data
        );
    }
    else {
        table->entries = alloc_<HashTable<K,V>::Entry>(new_size);
    }

    table->allocated = new_size;
    table->count = 0;

    for(int i = 0; i < table->allocated; i++) {
        table->entries[i].hash = 0;
    }

    if(old_entries) {
        for(int i = 0; i < old_allocated; i++) {
            if(old_entries[i].hash != 0) {
                //This way we are recalculating hash when it isn't needed. Change this.
                put(table, old_entries[i].key, old_entries[i].value);
            }
        }

        free_(old_entries);
    }
}

#endif