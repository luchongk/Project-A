#ifndef STRINGS_H
#define STRINGS_H

#include "array.h"
#include "maths.h"

struct String : ArrayView<u8> {
    u8& operator[](int i) {
        return this->data[i];
    }
};

String from_cstring(char* cstring, int count) {
    String result;
    result.count = count;
    result.data = (u8*)cstring;

    return result;
}

char* to_cstring(String string) {
    char* result = alloc_<char>(string.count+1);
    memcpy(result, string.data, string.count);
    result[string.count] = '\0';

    return result;
}

String new_string(int count) {
    u8* bytes = alloc_<u8>(count);

    String result;
    result.count = count;
    result.data = bytes;

    return result;
}

String operator ""_s(const char* str, size_t size) {
    return from_cstring((char*)str, (int)size);
}

bool equals(String a, String b) {
    if(a.count != b.count) return false;

    return strncmp((const char*)a.data, (const char*)b.data, a.count) == 0;
}

int compare(String a, String b) {
    if(a.count < b.count) return -1;
    if(a.count > b.count) return 1;

    if(a.count == 0) return 0;
    
    s64 length = min(a.count, b.count);
    return strncmp((const char*)a.data, (const char*)b.data, length);
}

String advance(String s, uint amount) {
    amount = min(amount, s.count);
    s.data += amount;
    s.count -= amount;

    return s;
}

String eat_until(u8 until, String* context) {
    String result;
    result.data = context->data;
    result.count = 0;

    For(*context) {
        if(result.count == context->count || *it == until) break;
        result.count++;
    }

    context->data += result.count;
    context->count -= result.count;

    return result;
}

String eat_line(String* s) {
    String result = eat_until('\n', s);

    // If we didn't reach the end, eat \n
    if(s->count > 0) {
        s->data++;
        s->count--;
    }

    return result;
}

String find_from_left(String to_find, String context) {
    String result;

    uint bytes_left = context.count;
    For(context) {
        if(memcmp(it, to_find.data, min(bytes_left, to_find.count)) == 0) {
            result.data = it;
            result.count = bytes_left;

            return result;
        }
        bytes_left--;
    }

    result.data = nullptr;
    result.count = 0;

    return result;
}

bool starts_with(String start, String str) {
    auto length = min(start.count, str.count);
    
    return memcmp(start.data, str.data, length) == 0;
}

#endif