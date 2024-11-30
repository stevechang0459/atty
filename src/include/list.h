#ifndef LIST_H
#define LIST_H

#include "global.h"

// // #include <stddef.h>
// #include <stdio.h>
// #include <stdlib.h>

// #include "types.h"
// #include "error.h"
// #include "utility.h"
// #include "menu.h"

// #define CONFIG_LIST_DEBUG_MESSAGE       (TRUE)

// #if (CONFIG_LIST_DEBUG_MESSAGE)
//     #define LIST_ASSERT(cond)   ASSERT(cond)
// #else
//     #define LIST_ASSERT(cond)
// #endif

typedef struct _swlist_t {
	u16 head;
	u16 tail;
	u16 cnt;
} swlist_t;

typedef struct list_head {
	struct list_head *next, *prev;
} list_t;

#define LIST_HEAD_INIT(name)    {&(name), &(name)}

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(
        struct list_head *new,
        struct list_head *prev,
        struct list_head *next)
{
	prev->next = new;
	new->prev = prev;
	new->next = next;
	next->prev = new;
}

// stack
static inline void list_add(
        struct list_head *new,
        struct list_head *head)
{
	__list_add(new, head, head->next);
}

// queue
static inline void list_add_tail(
        struct list_head *new,
        struct list_head *head
)
{
	__list_add(new, head->prev, head);
}

static inline void __list_del(
        struct list_head *prev,
        struct list_head *next
)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(struct list_head *ptr)
{
	__list_del(ptr->prev, ptr->next);
	ptr->next = NULL;
	ptr->prev = NULL;
}

static inline int list_is_last(
        const struct list_head *list,
        const struct list_head *head
)
{
	return list->next == head;
}

static inline int list_is_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline int list_is_singular(const struct list_head *head)
{
	return !list_is_empty(head) && (head->next == head->prev);
}

#define _offsetof(type, member) ((size_t)&((type *)0)->member)

#define _container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - _offsetof(type, member)))

#define list_entry(ptr, type, member) \
    _container_of(ptr, type, member)

#if 0
#define _offsetof(type, member)  ((size_t)&((type *)0)->member)

#define _container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - _offsetof(type, member) );})

#define list_entry(ptr, type, member) \
    _container_of(ptr, type, member)

#define list_entry(ptr, type, member) \
    ((type *)( (char *)(ptr) - ((size_t)&((type *)0)->member) ))

// use head to get the first/last entry
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

// use current entry to get next/prev entry
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(ptr, head) \
    for (ptr = (head)->next; ptr != (head); ptr = ptr->next)

#define list_for_each_prev(ptr, head) \
    for (ptr = (head)->prev; ptr != (head); ptr = ptr->prev)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry( head, typeof(*pos), member ); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

#define list_for_each_entry_prev(pos, head, member) \
    for (pos = list_last_entry( head, typeof(*(pos)), member ); \
         &pos->member != (head); \
         pos = list_prev_entry(head, member))

#define list_for_each_entry_cont(pos, head, member) \
    for (pos = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

#define list_for_each_entry_prev_cont(pos, head, member) \
    for (pos = list_prev_entry(pos, member); \
         &pos->member != (head); \
         pos = list_prev_entry(pos,member))

// ret_code menu_func_list(int argc, char **argv);

// void list_test(void);
#endif

#endif // LIST_H