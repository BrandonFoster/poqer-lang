/**
 * @file pq_syntax_tree.h
 * @author Brandon Foster
 * @brief poqer-lang syntax tree header.
 * the pq_syntax_tree struct is used to store the syntax tree of the poqer program.
 * create/destroy the syntax tree with the pq_new_* and pq_del_* functions.
 * 
 * @version 0.001
 * @date 11-6-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_SYNTAX_TREE_H
#define _PQ_SYNTAX_TREE_H
#include "pq_globals.h"
#include <stdlib.h>
#include <inttypes.h>

typedef struct pq_syntax_tree_node
{
    void* item;
    struct pq_syntax_tree_node* next;
    struct pq_syntax_tree_node* prev;
    struct pq_syntax_tree_node* children_nil;
} pq_syntax_tree_node;

/**
 * @brief The structure of a poqer-lang syntax tree.
 */
typedef struct pq_syntax_tree
{   //these variables should only be read externally, not modified.
    pq_syntax_tree_node* children_nil;
} pq_syntax_tree;

static inline pq_syntax_tree_node* pq_new_syntax_tree_node(void* item)
{
    pq_syntax_tree_node* node = (pq_syntax_tree_node*)malloc(sizeof(pq_syntax_tree_node));
    node->item = item;
    node->prev = NULL;
    node->next = NULL;
    node->children_nil = NULL;
    return node;
}

static inline pq_syntax_tree_node* pq_new_syntax_tree_nil_node()
{
    pq_syntax_tree_node* node = (pq_syntax_tree_node*)malloc(sizeof(pq_syntax_tree_node));
    node->item = NULL;
    node->prev = node;
    node->next = node;
    node->children_nil = NULL;
    return node;
}

/**
 * @brief Safe allocation for a pq_syntax_tree struct, initializes the syntax tree, then returns the pointer.
 * 
 * @return A pointer to the allocated pq_syntax_tree struct.
 */
static inline pq_syntax_tree* pq_new_syntax_tree()
{
    pq_syntax_tree* tree = (pq_syntax_tree*)malloc(sizeof(pq_syntax_tree));
    if(!tree) return NULL;
    tree->children_nil = pq_new_syntax_tree_nil_node();
    if(!tree->children_nil)
    {
        free(tree);
        return NULL;
    }
    return tree;
}

/**
 * @brief Safe deallocation of a pq_syntax_tree struct.
 * 
 * @param tree The syntax tree that will be deallocated.
 */
static inline void pq_del_syntax_tree(pq_syntax_tree* tree)
{
    if(!tree) return;
    free(tree);
}

static inline void pq_syntax_tree_add_left_sibling_node(pq_syntax_tree_node* node, pq_syntax_tree_node* sibling)
{
    if(!sibling) return;
    sibling->next = node;
    sibling->prev = node->prev;
    if(node->prev) node->prev->next = sibling;
    node->prev = sibling;
}

static inline void pq_syntax_tree_add_left_sibling(pq_syntax_tree_node* node, void* item)
{
    pq_syntax_tree_node* sibling = pq_new_syntax_tree_node(item);
    pq_syntax_tree_add_left_sibling_node(node, sibling);
}

static inline void pq_syntax_tree_add_right_sibling_node(pq_syntax_tree_node* node, pq_syntax_tree_node* sibling)
{
    if(!sibling) return;
    sibling->prev = node;
    sibling->next = node->next;
    if(node->next) node->next->prev = sibling;
    node->next = sibling;
}

static inline void pq_syntax_tree_add_right_sibling(pq_syntax_tree_node* node, void* item)
{
    pq_syntax_tree_node* sibling = pq_new_syntax_tree_node(item);
    pq_syntax_tree_add_right_sibling_node(node, sibling);
}

static inline void pq_syntax_tree_add_left_child_node(pq_syntax_tree_node* node, pq_syntax_tree_node* child)
{
    if(!child) return;
    if(!node->children_nil)
    {
        node->children_nil = pq_new_syntax_tree_nil_node();
        if(!node->children_nil) return;
    }
    child->prev = node->children_nil;
    child->next = node->children_nil->next;
    node->children_nil->next->prev = child;
    node->children_nil->next = child;
}

static inline void pq_syntax_tree_add_left_child(pq_syntax_tree_node* node, void* item)
{
    pq_syntax_tree_node* child = pq_new_syntax_tree_node(item);
    pq_syntax_tree_add_left_child_node(node, child);
}

static inline void pq_syntax_tree_add_right_child_node(pq_syntax_tree_node* node, pq_syntax_tree_node* child)
{
    if(!child) return;
    if(!node->children_nil)
    {
        node->children_nil = pq_new_syntax_tree_nil_node();
        if(!node->children_nil) return;
    }
    child->next = node->children_nil;
    child->prev = node->children_nil->prev;
    node->children_nil->prev->next = child;
    node->children_nil->prev = child;
}

static inline void pq_syntax_tree_add_right_child(pq_syntax_tree_node* node, void* item)
{
    pq_syntax_tree_node* child = pq_new_syntax_tree_node(item);
    pq_syntax_tree_add_right_child_node(node, child);
}

static inline PQbool pq_syntax_tree_is_leaf(pq_syntax_tree_node* node)
{
    return node->children_nil == NULL || node->children_nil->next == node->children_nil->prev;
}

static inline pq_syntax_tree_node* pq_syntax_tree_get_leftmost_leaf(pq_syntax_tree_node* node)
{
    if(!node) return NULL;
    if(!pq_syntax_tree_is_leaf(node))
    {
        node = node->children_nil->next;
    }
    return node;
}

#endif