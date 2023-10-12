#ifndef STRINGS_H
#define STRINGS_H

#include "array.h"
#include "maths.h"

struct String : ArrayView<u8> {};

String from_cstring(char* cstring, int count = 0) {
    String result;
    result.data = (u8*)cstring;
    
    if(count > 0) {
        result.count = count;
    }
    else {
        result.count = (u32)strlen(cstring);
    }

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

    for(uint i = 0; i < a.count; i++) {
        if(a[i] != b[i]) return false;
    }
    
    return true;
}

String advance(String s, uint amount) {
    amount = min(amount, s.count);
    s.data  += amount;
    s.count -= amount;

    return s;
}

String eat_until(u8 until, String* context) {
    String result;
    result.data  = context->data;
    result.count = 0;

    For(*context) {
        if(result.count == context->count || *it == until) break;
        result.count++;
    }

    context->data  += result.count;
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
    assert(to_find.count > 0);

    String result;
    if(to_find.count > context.count) return result;

    for(uint i = 0; i < context.count - to_find.count + 1; i++) {
        if(context[i] == to_find[0]) {
            bool equal = true;
            for(uint j = 1; j < to_find.count; j++) {
                if(context[i+j] != to_find[j]) {
                    equal = false;
                    break;
                }
            }

            if(equal) {
                result.data  = &context[i];
                result.count = context.count - i + 1;
                return result;
            }
        }
    }

    return result;
}

bool starts_with(String start, String str) {
    if(start.count > str.count) return false;
    
    return memcmp(start.data, str.data, start.count) == 0;
}

#endif