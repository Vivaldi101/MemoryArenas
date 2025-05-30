#include "list.h"
#include "arena.h"
#include <stdbool.h>
#include <stdio.h>

#define implies(p, q) (!(p) || (q))
#define iff(p, q) (implies(p, q) && implies(q, p))

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

int32s fibonacci(int32_t max, arena* perm)
{
   static int32_t init[] = {0, 1};
   int32s fib = {0};
   fib.data = init;
   fib.len = fib.cap = countof(init);

   for(;;)
   {
      int32_t a = fib.data[fib.len - 2];
      int32_t b = fib.data[fib.len - 1];
      if(a + b > max)
         return fib;
      //*push(&fib, perm) = a + b;
   }
}

static void iterate_objs_indexes(arena a, size s)
{
   for(size i = 0; i < s; ++i)
   {
      int* n = push(&a, int, 1);
      *n = 42;
   }
}

int main()
{
   const size arena_size = 1ull << 46;
   const size ints_count = 4;

   void* base = VirtualAlloc(0, arena_size, MEM_RESERVE, PAGE_READWRITE);
   assert(base);

   arena a1 = arena_new(base, 4096);
#if 0
   int* pa1 = push(&a1, int, ints_count);
   int* q = pa1;
   for(int i = 0; i < ints_count; ++i)
      pa1[i] = i;

   // TODO: expand inside push
   size expand_count = 10*sizeof(int);
   int* w = pa1 = push(&a1, int, ints_count);

   for(int i = 0; i < ints_count; ++i)
      pa1[i] = i*2;

   pa1 = push(&a1, int, ints_count);

   for(int i = 0; i < ints_count; ++i)
      pa1[i] = i*4;

   int* r = pa1 = push(&a1, int, ints_count);

   for(int i = 0; i < ints_count; ++i)
      pa1[i] = i*8;

   for(int i = 0; i < ints_count-1; ++i)
      assert(pa1[i+1] == 4*w[i+1]);

   int* n = push(&a1, int, ints_count);
#endif

   size s = 100000000;
   iterate_objs_indexes(a1, s);

   int* n = a1.beg;

   return 0;
}