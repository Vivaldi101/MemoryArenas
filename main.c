#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "list.h"
#include "arena.h"

#define implies(p, q) (!(p) || (q))
#define iff(p, q) (implies(p, q) && implies(q, p))

#define defer(begin, end) int (_i_); for((_i_) = (begin, 0); !(_i_); ++(_i_), end)
#define ensure(begin, end) int (_i_); assert(begin); for((_i_) = ((begin), 0); !(_i_); ++(_i_), assert(end))

typedef struct thing
{
   char buf[123];
   int a, b, c;
} thing;

typedef struct thing2
{
   size a;
   size b;
   size c;
} thing2;

static void do_work2(arena temp)
{
   thing* t;
   arena_loop(i, temp, t)
   {
      t = arena_offset(i, temp, thing);
      t->a = 1;
      t->b = 2;
      t->c = 2;
   }
}

#if 0
static size thing_count(arena temp, size count)
{
   arena scratch = push(&temp, thing, count);
   size s = scratch_size(scratch) / sizeof(typeof(thing));
   size sum = 0;

   for(size i = 0; i < s; ++i)
      sum += i;

   return sum;
}
#endif

// scratch if by passed by value and arena if by pointer
static size perfect_work(arena temp, thing* p)
{
   size s = scratch_size(temp) / sizeof(*p);
   assert(implies(s > 0, p));

   size sum = 0;
   for(size i = 0; i < s; ++i)
   {
      sum += p->a;
      sum += p->b;
      sum += p->c;
      p[i].a += (int)sum;
   }

   return sum;
}

#if 0
static void arena_narrow(arena temp)
{
   size alloc_count = 3;
   assert(scratch_size(temp) / sizeof(typeof(thing)) >= alloc_count);

   size count = thing_count(temp, alloc_count);
   count += thing_count(temp, alloc_count);

   assert(count == 6);
}

static void do_work6(arena temp)
{
   arena s = temp;
   arena_iterate(temp, 3, int, t, { *t = 42; });

   temp = s;
   arena_narrow(temp);
}

static void do_work4(arena temp)
{
   arena s = temp;
   arena_iterate(s, 3, thing, t,
   {
      t++[i].a = 4;
   });

   s = temp;
   do_work6(s);
}
#endif

static arena arena_join(arena from, arena to)
{
   assert(from.beg < to.end);

   return (arena){from.beg, to.end};
}

typedef struct
{
   int32_t* data;
   ptrdiff_t len;
   ptrdiff_t cap;
} int32s;

static void push_objs_indexes(arena* a, size s, int v)
{
   for(size i = 0; i < s; ++i)
   {
      int* n = push(a, int);
      *n = v;
   }
}

int main()
{
   const size arena_size = 1ull << 46;

   void* base = VirtualAlloc(0, arena_size, MEM_RESERVE, PAGE_READWRITE);
   assert(base);

   arena a = arena_new(base, page_size);

   defer(a = arena_new(base, page_size), arena_decommit(&a))
   {
      const size total = page_size*3;
      int* pa = a.beg;
      push_objs_indexes(&a, total, 42);

      for(size i = 0; i < total; ++i)
         assert(pa[i] == 42);

      arena b = arena_new(a.end, page_size);
      int* pb = b.beg;
      push_objs_indexes(&b, total, 99);

      for(size i = 0; i < total; ++i)
         assert(pb[i] == 99);

      arena_decommit(&b);
   }

   return 0;
}