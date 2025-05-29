#pragma once

// List macros.

#define OffsetOf(type, field) (size_t)&((type*)0)->field
#define ContainerOf(ptr, type, field) (type*)(((char*)(ptr)) - OffsetOf(type, field))
#define ListEntry(ptr, type, field) ContainerOf(ptr, type, field)

#define ListCreate(list_head) (list_head)->previous = (list_head)->next = (list_head); 

#define ListForEach(cursor, list_head, type, field) \
		for (type* (cursor) = ListEntry((list_head)->next, type, field); \
			&(cursor)->field != (list_head); \
			(cursor) = ListEntry((cursor)->field.next, type, field))


typedef struct list_head
{
   struct list_head* previous;
   struct list_head* next;
} list_head;

