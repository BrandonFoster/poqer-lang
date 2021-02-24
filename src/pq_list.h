/**
 * @file pq_list.h
 * @author Brandon Foster
 * @brief poqer-lang doubly linked list header.
 * the pq_list struct represents a doubly linked list.
 * create/destroy the list with the pq_new_* and pq_del_* functions.
 * 
 * @version 0.001
 * @date 11-5-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_LIST_H
#define _PQ_LIST_H

#include <stdlib.h>
#include <inttypes.h>

typedef struct pq_list_node
{
    void* item;
    struct pq_list_node* prev;
    struct pq_list_node* next;
} pq_list_node;

typedef struct pq_list
{
    pq_list_node* nil; //sentinel for a doubly linked list
    uint64_t size;
} pq_list;

static inline pq_list_node* pq_new_list_node()
{
    pq_list_node* node = (pq_list_node*)malloc(sizeof(pq_list_node));
    if(!node) return NULL;

    node->prev = NULL;
    node->next = NULL;
    node->item = NULL;

    return node;
}

static inline void pq_del_list_node(pq_list_node* node)
{
    if(!node) return;
    free(node);
}

static inline pq_list* pq_new_list()
{
    pq_list* list = (pq_list*)malloc(sizeof(pq_list));
    if(!list) return NULL;

    list->nil = (pq_list_node*)malloc(sizeof(pq_list_node));
    if(!list->nil)
    {
        free(list);
        return NULL;
    }

    list->nil->next = list->nil;
    list->nil->prev = list->nil;
    list->size = 0;
    return list;
}

static inline void pq_del_list(pq_list* list)
{
    if(!list) return;
    free(list);
}

static inline void pq_list_push_back(pq_list* list, void* item)
{
    pq_list_node* node = (pq_list_node*)malloc(sizeof(pq_list_node));
    node->item = item;
    node->prev = list->nil->prev;
    list->nil->prev = node;
    node->next = list->nil;
    list->size++;
}

static inline void pq_list_push_forward(pq_list* list, void* item)
{
    pq_list_node* node = (pq_list_node*)malloc(sizeof(pq_list_node));
    node->item = item;
    node->next = list->nil->next;
    list->nil->next = node;
    node->prev = list->nil;
    list->size++;
}

static inline void* pq_list_front(pq_list* list)
{
    return list->nil->next->item;
}

static inline void* pq_list_back(pq_list* list)
{
    return list->nil->prev->item;
}

#endif