/**
 * @file pq_binary_tree.h
 * @author Brandon Foster
 * @brief poqer-lang binary tree header.
 * the pq_binary_tree struct is used to store the binary tree of the poqer program.
 * create/destroy the binary tree with the pq_new_* and pq_del_* functions.
 * 
 * @version 0.001
 * @date 11-10-2020
 * @copyright Brandon Foster (c) 2020-2021
 */

#ifndef _PQ_BINARY_TREE_H
#define _PQ_BINARY_TREE_H
#include "pq_globals.h"
#include <stdlib.h>

typedef struct pq_binary_tree_node
{
    void* item;
    struct pq_binary_tree_node* left;
    struct pq_binary_tree_node* right;
} pq_binary_tree_node;

typedef struct pq_binary_tree
{   //these variables should only be read externally, not modified.
    pq_binary_tree_node* root;
} pq_binary_tree;

static inline pq_binary_tree_node* pq_new_binary_tree_node(void* item)
{
    pq_binary_tree_node* node = (pq_binary_tree_node*)malloc(sizeof(pq_binary_tree_node));
    node->item = NULL;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static inline pq_binary_tree* pq_new_binary_tree(void* item)
{
    pq_binary_tree* tree = (pq_binary_tree*)malloc(sizeof(pq_binary_tree));
    tree->root = pq_new_binary_tree_node(item);
}

#endif