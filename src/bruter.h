// libbruter
// libbruter use no other libraries beside the standard C library
// if libbruter does not work on a platform, it is a bug, and should be reported.
// it is meant to be compatible even with arduino and wasm, both tested.
// so any platform that has a standard C library should be able to compile and run libbruter.
// even non-gcc-like compilers like cl.exe are expected to work.

#ifndef BRUTER_H
#define BRUTER_H 1

// standard library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

// standard library
#include <string.h>
#include <math.h>
#include <time.h>
#include <float.h>

// not part of the standard library
#include <ctype.h>

// version
#define VERSION "0.7.9"

// types
#define TYPE_ANY 0
#define TYPE_NUMBER 1
#define TYPE_STRING 2

// we use Int and Float instead of int and float because we need to use always the pointer size for any type that might share the fundamental union type;
// bruter use a union as universal type, and bruter is able to manipulate and use pointers direcly so we need to use the pointer size;
#if __SIZEOF_POINTER__ == 8
    #define Int int64_t
    #define Float double
#else
    #define Int int32_t
    #define Float float
#endif

#define Byte uint8_t

// from bruter 0.7.7c include/c_list.h
#define List(T) struct \
{ \
    T *data; \
    Int size; \
    Int capacity; \
}

// malloc and initialize a new list
#define list_init(type) \
({ \
    type *list = (type*)malloc(sizeof(type)); \
    list->data = NULL; \
    list->size = 0; \
    list->capacity = 0; \
    list; \
})

// increase the capacity of the stack
#define list_double(s) do \
{ \
    (s).capacity = (s).capacity == 0 ? 1 : (s).capacity * 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
} while (0)

// decrease the capacity of the stack
#define list_half(s) do \
{ \
    (s).capacity /= 2; \
    (s).data = realloc((s).data, (s).capacity * sizeof(*(s).data)); \
    if ((s).size > (s).capacity) \
    { \
        (s).size = (s).capacity; \
    } \
} while (0)

// add an element to the end of the list
#define list_push(s, v) do \
{ \
    if ((s).size == (s).capacity) \
    { \
        list_double(s); \
    } \
    (s).data[(s).size++] = (v); \
} while (0)

// add an element to the beginning of the list
#define list_unshift(s, v) do \
{ \
    if ((s).size == (s).capacity) \
    { \
        list_double(s); \
    } \
    memmove(&(s).data[1], &(s).data[0], (s).size * sizeof(*(s).data)); \
    (s).data[0] = (v); \
    (s).size++; \
} while (0)

// remove the last element and return it
#define list_pop(s) ((s).data[--(s).size])

// remove the first element and return it
#define list_shift(s) \
({ \
    typeof((s).data[0]) ret = (s).data[0]; \
    if ((s).size > 1) { \
        memmove(&(s).data[0], &(s).data[1], ((s).size - 1) * sizeof(*(s).data)); \
    } \
    (s).size--; \
    ret; \
})

// free the list
#define list_free(s) \
({\
    free((s).data);\
    free(&s);\
})

//swap elements from index i1 to index i2
#define list_swap(s, i1, i2) do \
{ \
    typeof((s).data[i1]) tmp = (s).data[i1]; \
    (s).data[i1] = (s).data[i2]; \
    (s).data[i2] = tmp; \
} while (0)

// insert element v at index i
#define list_insert(s, i, v) do \
{ \
    if ((s).size == (s).capacity) \
    { \
        list_double(s); \
    } \
    if (i <= (s).size) { \
        memmove(&(s).data[i + 1], &(s).data[i], ((s).size - i) * sizeof(*(s).data)); \
        (s).data[i] = (v); \
        (s).size++; \
    } else { \
        printf("BRUTER_ERROR: index %ld out of range in list of size %ld\n", i, (s).size); \
        exit(EXIT_FAILURE); \
    } \
} while (0)



//remove element at index i and return it
// Remove element at index i and return it
#define list_remove(s, i) \
({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    memmove(&(s).data[i], &(s).data[i + 1], ((s).size - (i) - 1) * sizeof(*(s).data)); \
    (s).size--; \
    ret; \
})

//same as remove but does a swap and pop, faster but the order of the elements will change
#define list_fast_remove(s, i) \
({ \
    typeof((s).data[i]) ret = (s).data[i]; \
    list_swap(s, i, (s).size - 1); \
    list_pop(s); \
    ret; \
})

#define list_ocurrences(s, v) \
({ \
    Int i = 0; \
    while (i < (s).size && (s).data[i] != (v)) \
    { \
        i++; \
    } \
    i == (s).size ? -1 : i; \
})

#define list_find(s, v) \
({ \
    Int i = -1; \
    for (Int j = 0; j < (s).size; j++) \
    { \
        if ((s).data[j] == (v)) \
        { \
            i = j; \
            break; \
        } \
    } \
    i; \
})

#define list_reverse(s) do \
{ \
    for (Int i = 0; i < (s).size / 2; i++) \
    { \
        list_swap((s), i, (s).size - i - 1); \
    } \
} while (0)

#define list_set(s, i, v) \
(\
    (s).data[i] = (v)\
)

#define list_get(s, i) \
(\
    (s).data[i]\
)

//Value
typedef union 
{
    // these types depend on the size of the pointer
    Float number;
    Int integer;

    // these types are pointers
    char* string;
    void* pointer;
    
    // these types are arrays
    uint8_t u8[sizeof(Float)];
    uint16_t u16[sizeof(Float) / 2];
    uint32_t u32[sizeof(Float) / 4];

    int8_t i8[sizeof(Float)];
    int16_t i16[sizeof(Float) / 2];
    int32_t i32[sizeof(Float) / 4];

    float f32[sizeof(Float) / 4];

    #if __SIZEOF_POINTER__ == 8
        uint64_t u64[sizeof(Float) / 8];
        int64_t i64[sizeof(Float) / 8];
        double f64[sizeof(Float) / 8];
    #endif

} Value;

//List
typedef List(Value) ValueList;
typedef List(char*) StringList;
typedef List(Int) IntList;
typedef List(Byte) ByteList;


typedef struct
{
    ValueList *stack;
    ByteList *typestack;
    IntList *unused;
    
    // hashes
    StringList *hash_names;
    IntList *hash_indexes;

} VirtualMachine;

//Function
typedef Int (*Function)(VirtualMachine*, IntList*);
typedef void (*InitFunction)(VirtualMachine*);

//String
char* str_duplicate(const char *str);
char* str_nduplicate(const char *str, Int n);
char* str_format(const char *fmt, ...);
char* str_sub(const char *str, Int start, Int end);
char* str_concat(const char *str1, const char *str2);
Int str_find(const char *str, const char *substr);
char* str_replace(const char *str, const char *substr, const char *replacement);
char* str_replace_all(const char *str, const char *substr, const char *replacement);

StringList* str_split(char *str, char *delim);
StringList* str_split_char(char *str, char delim);
StringList* special_space_split(char *str);
StringList* special_split(char *str, char delim);

#define is_true(value, __type) (__type == value.integer == 0 ? 0 : 1)

// variable

VirtualMachine* make_vm();
void free_vm(VirtualMachine *vm);
void unuse_var(VirtualMachine *vm, Int index);

Int new_number(VirtualMachine *vm, Float number);
Int new_string(VirtualMachine *vm, char *str);
Int new_builtin(VirtualMachine *vm, Function function);
Int new_var(VirtualMachine *vm);

Value value_duplicate(Value value, Byte type);

Int register_var(VirtualMachine *vm, char* varname);
Int register_string(VirtualMachine *vm, char* varname, char* string);
Int register_number(VirtualMachine *vm, char* varname, Float number);
Int register_builtin(VirtualMachine *vm, char* varname, Function function);

Int hash_find(VirtualMachine *vm, char *key);
void hash_set(VirtualMachine *vm, char *key, Int index);
void hash_unset(VirtualMachine *vm, char *key);




// macros
#define data(index) (vm->stack->data[index])
#define data_t(index) (vm->typestack->data[index])

#define arg(index) (vm->stack->data[args->data[index]])
#define arg_i(index) (args->data[index])
#define arg_t(index) (vm->typestack->data[args->data[index]])

#define function(name) Int name(VirtualMachine *vm, IntList *args)
#define init(name) void init_##name(VirtualMachine *vm)

// eval
Int eval(VirtualMachine *vm, char *cmd);
Int interpret(VirtualMachine *vm, char *cmd);

// functions
IntList* parse(void* _vm, char* cmd);

#ifndef ARDUINO

char* readfile(char *filename);
void writefile(char *filename, char *content);
bool file_exists(char *filename);

#endif
#endif