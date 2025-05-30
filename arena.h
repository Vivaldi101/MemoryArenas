#pragma once

#include <stddef.h>
#include <malloc.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#include "list.h"

#define arena_iterate(a, s, type, var, code_block) \
do { \
    arena scratch_ = push(&(a), type, (s)); \
    type* (var) = scratch_.beg; \
    size_t count_ = scratch_size(scratch_) / sizeof(type); \
    for (size_t i = 0; i < count_; ++i) code_block \
} while (0)

#define arena_t(a) typeof(a)

#define page_size (4096)
#define align_page_size (4096 -1)

#define scratch_count_left(a, s) (size)(((a).end - (a).beg) / (s))
#define arena_end(a, p) ((byte*)(a)->end == (byte*)(p))
#define scratch_end(a, p) ((byte*)(a).end == (byte*)(p))
#define scratch_left(a,t)  (size)(((byte*)(a).end - (byte*)(a).beg) / sizeof(t))
#define arena_left(a,t)    (size)(((a)->end - (a)->beg) / sizeof(t))
#define arena_full(a)      ((a)->beg == (a)->end)   // or empty for stub arenas
#define arena_empty(a)      (arena_full(a))         // or empty for stub arenas
#define arena_loop(i, a, p) for(size (i) = 0; (i) < scratch_left((a), *(p)); ++(i))
#define arena_offset(i, a, t) (t*)a.beg + (i)

#define scratch_count(a, s, t)  ((s) - scratch_left(a, t))
#define arena_count(a, s, t)  ((s) - arena_left(a, t))

#define arena_stub(p, a) ((p) == (a)->end)
#define scratch_stub(p, a) arena_stub(p, &a)

#define arena_end_count(p, a, n) (void*)(p)!=(a)->end?(p)+(n):(p)
#define scratch_end_count(p, a, n) (void*)(p)!=(a).end?(p)+(n):(p)

#define scratch_invariant(s, a, t) assert((s) <= scratch_left((a), t))

#define scratch_size(s) (size)((byte*)(s).end - (byte*)(s).beg)
#define scratch_empty(s) (((s).beg == (s).end))

#define newx(a,b,c,d,e,...) e
#define push(...)            newx(__VA_ARGS__,new4,new3,new2)(__VA_ARGS__)
#define new2(a, t)          alloc(a, sizeof(t), __alignof(t), 1, 0)
#define new3(a, t, n)       alloc(a, sizeof(t), __alignof(t), n, 0)
#define new4(a, t, n, f)    alloc(a, sizeof(t), __alignof(t), n, f)

#define newx_size(a,b,c,d,e,...) e
#define new_size(...)            newx_size(__VA_ARGS__,new4_size,new3_size,new2_size)(__VA_ARGS__)
#define new2_size(a, t)          alloc(a, t, t, 1, 0)
#define new3_size(a, t, n)       alloc(a, t, t, n, 0)
#define new4_size(a, t, n, f)    alloc(a, t, t, n, f)

#define sizeof(x)       (size)sizeof(x)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define lengthof(s)     (countof(s) - 1)
#define amountof(a, t)  ((a) * sizeof(t))

typedef uint8_t   u8;
typedef int32_t   b32;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
//typedef char      byte;
typedef ptrdiff_t size;
typedef size_t    usize;

#define s8(s) (s8){(u8 *)s, lengthof(s)}
typedef struct
{
   u8* data;
   size len;
} s8;

typedef struct arena
{
   void* beg;
   void* end;  // one past the end
   //list_head arenas;
} arena;

// TODO: use GetSystemInfo
static arena arena_new(void* base, size cap)
{
   assert(base && cap > 0);

   //cap = (cap + align_page_size) & ~align_page_size; // align to page size

   arena result = {0};

   result.beg = VirtualAlloc(base, cap, MEM_COMMIT, PAGE_READWRITE);
   result.end = (byte*)result.beg + cap;

   assert(result.beg < result.end);

   return result;
}

static void arena_expand(arena* a, size new_cap)
{
   assert((uintptr_t)a->end <= ((1ull << 48)-1) - page_size);
   arena new_arena = arena_new((byte*)a->end, new_cap);
   new_arena.beg = a->end;

   assert(new_arena.beg == a->end);

   a->beg = new_arena.beg; // expanded arena beginning
   a->end = (byte*)a->beg + new_cap;

   assert(a->beg < a->end);   // invariant
   assert(a->end == (byte*)a->beg + new_cap);   // post
}

static void* alloc(arena* a, size alloc_size, size align, size count, u32 flag)
{
   // align allocation to next aligned boundary
   void* p = (void*)(((uptr)a->beg + (align - 1)) & (-align));

   if(count <= 0 || count > ((byte*)a->end - (byte*)p) / alloc_size) // empty or overflow
   {
      arena_expand(a, ((count * alloc_size) + align_page_size) & ~align_page_size);
      p = a->beg;
   }

   a->beg = (byte*)p + (count * alloc_size);                         // advance arena 

   assert(((uptr)p & (align - 1)) == 0);                             // aligned result

   return p;
}

static bool arena_reset(arena* a)
{
   return VirtualAlloc(a->beg, (byte*)a->end - (byte*)a->beg, MEM_RESET, PAGE_READWRITE) != 0;
}

static bool arena_decommit(arena* a)
{
   return VirtualFree(a->beg, 0, MEM_DECOMMIT) != 0;
}
