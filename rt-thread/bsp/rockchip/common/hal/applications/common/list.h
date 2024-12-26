/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#ifndef __LIST_H__
#define __LIST_H__

/*
 * container_of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/*
******************************************************************************
 * Double List define
******************************************************************************
*/
/*
 * Double List structure
 */
struct list_node
{
    struct list_node *next;                         /**< point to next node. */
    struct list_node *prev;                         /**< point to prev node. */
};
typedef struct list_node list_t;                    /**< Type for lists. */

/*
 * initialize a list object
 */
#define LIST_OBJECT_INIT(object) { &(object), &(object) }

/*
 * get the struct for this entry
 * node:   the entry point
 * type:   the type of structure
 * member: the name of list in structure
 */
#define list_entry(node, type, member) \
    container_of(node, type, member)

/*
 * list_for_each - iterate over a list
 * pos:  the list_t * to use as a loop cursor.
 * head: the head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/*
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * pos:  the list_t * to use as a loop cursor.
 * n:    another list_t * to use as temporary storage
 * head: the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/*
 * list_for_each_entry  -   iterate over list of given type
 * pos:    the type * to use as a loop cursor.
 * head:   the head for your list.
 * member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/*
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * pos:    the type * to use as a loop cursor.
 * n:      another type * to use as temporary storage
 * head:   the head for your list.
 * member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member), \
         n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/*
 * list_first_entry - get the first element from a list
 * ptr:    the list head to take the element from.
 * type:   the type of the struct this is embedded in.
 * member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/*
 * initialize a list
 *
 * l: list to be initialized
 */
void list_init(list_t *l);

/*
 * insert a node after a list
 *
 * l: list to insert it
 * n: new node to be inserted
 */
void list_insert_after(list_t *l, list_t *n);

/*
 * insert a node before a list
 *
 * n: new node to be inserted
 * l: list to insert it
 */
void list_insert_before(list_t *l, list_t *n);

/*
 * remove node from list.
 * n: the node to remove from the list.
 */
void list_remove(list_t *n);

/*
 * tests whether a list is empty
 * l: the list to test.
 */
int list_isempty(const list_t *l);

/*
 * get the list length
 * l: the list to get.
 */
unsigned int list_len(const list_t *l);

/*
******************************************************************************
 * Single List define
******************************************************************************
*/
/*
 * Single List structure
 */
struct slist_node
{
    struct slist_node *next;                        /**< point to next node. */
};
typedef struct slist_node slist_t;                  /**< Type for single list. */

/*
 * initialize a slist object
 */
#define SLIST_OBJECT_INIT(object) { NULL }

/*
 * get the struct for this single list node
 * node:   the entry point
 * type:   the type of structure
 * member: the name of list in structure
 */
#define slist_entry(node, type, member) \
    container_of(node, type, member)

/*
 * slist_for_each - iterate over a single list
 * pos:    the slist_t * to use as a loop cursor.
 * head:   the head for your single list.
 */
#define slist_for_each(pos, head) \
    for (pos = (head)->next; pos != NULL; pos = pos->next)

/*
 * slist_for_each_entry  -   iterate over single list of given type
 * pos:    the type * to use as a loop cursor.
 * head:   the head for your single list.
 * member: the name of the list_struct within the struct.
 */
#define slist_for_each_entry(pos, head, member) \
    for (pos = slist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (NULL); \
         pos = slist_entry(pos->member.next, typeof(*pos), member))

/*
 * slist_first_entry - get the first element from a slist
 * ptr:    the slist head to take the element from.
 * type:   the type of the struct this is embedded in.
 * member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define slist_first_entry(ptr, type, member) \
    slist_entry((ptr)->next, type, member)

/*
 * slist_tail_entry - get the tail element from a slist
 * ptr:    the slist head to take the element from.
 * type:   the type of the struct this is embedded in.
 * member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define slist_tail_entry(ptr, type, member) \
    slist_entry(slist_tail(ptr), type, member)

/*
 * initialize a single list
 *
 * l: the single list to be initialized
 */
void slist_init(slist_t *l);
void slist_append(slist_t *l, slist_t *n);
void slist_insert(slist_t *l, slist_t *n);
unsigned int slist_len(const slist_t *l);
slist_t *slist_remove(slist_t *l, slist_t *n);
slist_t *slist_first(slist_t *l);
slist_t *slist_tail(slist_t *l);
slist_t *slist_next(slist_t *n);
int slist_isempty(slist_t *l);

#endif
