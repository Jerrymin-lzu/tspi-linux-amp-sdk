/**
  * Copyright (c) 2023 Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  */

#include "hal_base.h"
#include "list.h"

/*
 * initialize a list
 *
 * l: list to be initialized
 */
void list_init(list_t *l)
{
    l->next = l->prev = l;
}

/*
 * insert a node after a list
 *
 * l: list to insert it
 * n: new node to be inserted
 */
void list_insert_after(list_t *l, list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/*
 * insert a node before a list
 *
 * n: new node to be inserted
 * l: list to insert it
 */
void list_insert_before(list_t *l, list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/*
 * remove node from list.
 * n: the node to remove from the list.
 */
void list_remove(list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/*
 * tests whether a list is empty
 * l: the list to test.
 */
int list_isempty(const list_t *l)
{
    return l->next == l;
}

/*
 * get the list length
 * l: the list to get.
 */
unsigned int list_len(const list_t *l)
{
    unsigned int len = 0;
    const list_t *p = l;
    while (p->next != l)
    {
        p = p->next;
        len ++;
    }

    return len;
}

/*
 * initialize a single list
 *
 * l: the single list to be initialized
 */
void slist_init(slist_t *l)
{
    l->next = NULL;
}

void slist_append(slist_t *l, slist_t *n)
{
    struct slist_node *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = NULL;
}

void slist_insert(slist_t *l, slist_t *n)
{
    n->next = l->next;
    l->next = n;
}

unsigned int slist_len(const slist_t *l)
{
    unsigned int len = 0;
    const slist_t *list = l->next;
    while (list != NULL)
    {
        list = list->next;
        len ++;
    }

    return len;
}

slist_t *slist_remove(slist_t *l, slist_t *n)
{
    /* remove slist head */
    struct slist_node *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (slist_t *)0) node->next = node->next->next;

    return l;
}

slist_t *slist_first(slist_t *l)
{
    return l->next;
}

slist_t *slist_tail(slist_t *l)
{
    while (l->next) l = l->next;

    return l;
}

slist_t *slist_next(slist_t *n)
{
    return n->next;
}

int slist_isempty(slist_t *l)
{
    return l->next == NULL;
}
