#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Intrusive doubly linked list node.
 *
 * Designed to be embedded directly within data structures (e.g. OsTaskControlBlock)
 * to allow zero-allocation membership in multiple lists simultaneously.
 */
typedef struct LinkedListNode {
    struct LinkedListNode* next;  /**< Pointer to the next node in the list. */
    struct LinkedListNode* prev;  /**< Pointer to the previous node in the list. */
} LinkedListNode;

/**
 * @brief Intrusive circular doubly linked list root container.
 *
 * Utilizes a circular sentinel node ('root') where root.next points to the
 * first element and root.prev points to the last element. An empty list
 * has root.next == &root and root.prev == &root.
 */
typedef struct LinkedList {
    LinkedListNode root;  /**< Sentinel anchor node. */
    size_t count;         /**< Total number of elements currently stored. */
} LinkedList;

/**
 * @brief Computes the address of the parent structure containing an intrusive LinkedListNode.
 *
 * @param ptr Pointer to the embedded LinkedListNode member.
 * @param type The type of the parent structure.
 * @param member The name of the LinkedListNode member within the parent structure.
 * @return Pointer to the enclosing parent structure of type (type*).
 */
#define LINKED_LIST_CONTAINER_OF(ptr, type, member) \
    ((type*)(void*)((char*)(ptr) - offsetof(type, member)))

/**
 * @brief Direct unchecked insertion at the tail of the list.
 *
 * Executes in strictly 4 store instructions without function call overhead.
 * For use in performance-critical kernel dispatcher code where parameters are pre-verified.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the unlinked LinkedListNode to insert.
 */
static inline __attribute__((no_instrument_function)) void linked_list_insert_tail_direct(LinkedList* list, LinkedListNode* node)
{
    node->prev = list->root.prev;
    node->next = &list->root;
    list->root.prev->next = node;
    list->root.prev = node;
    list->count++;
}

/**
 * @brief Direct unchecked insertion at the head of the list.
 *
 * Executes in strictly 4 store instructions without function call overhead.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the unlinked LinkedListNode to insert.
 */
static inline __attribute__((no_instrument_function)) void linked_list_insert_head_direct(LinkedList* list, LinkedListNode* node)
{
    node->prev = &list->root;
    node->next = list->root.next;
    list->root.next->prev = node;
    list->root.next = node;
    list->count++;
}

/**
 * @brief Direct unchecked unlinking of a node.
 *
 * Executes in strictly 4 store instructions without function call overhead.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the linked LinkedListNode to remove.
 */
static inline __attribute__((no_instrument_function)) void linked_list_remove_direct(LinkedList* list, LinkedListNode* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
    list->count--;
}

/**
 * @brief Removes and returns the node at the head of the list.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @return Pointer to the popped LinkedListNode, or NULL if list is empty or NULL.
 */
static inline __attribute__((no_instrument_function)) LinkedListNode* linked_list_pop_head(LinkedList* list)
{
    if ((list == NULL) || (list->count == 0U)) {
        return NULL;
    }
    LinkedListNode* head = list->root.next;
    head->prev->next = head->next;
    head->next->prev = head->prev;
    head->next = NULL;
    head->prev = NULL;
    list->count--;
    return head;
}

/**
 * @brief Removes and returns the node at the tail of the list.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @return Pointer to the popped LinkedListNode, or NULL if list is empty or NULL.
 */
static inline __attribute__((no_instrument_function)) LinkedListNode* linked_list_pop_tail(LinkedList* list)
{
    if ((list == NULL) || (list->count == 0U)) {
        return NULL;
    }
    LinkedListNode* tail = list->root.prev;
    tail->prev->next = tail->next;
    tail->next->prev = tail->prev;
    tail->next = NULL;
    tail->prev = NULL;
    list->count--;
    return tail;
}

/**
 * @brief Returns the node at the head of the list without removing it.
 *
 * @param list Pointer to the LinkedList instance.
 * @return Pointer to head LinkedListNode, or NULL if list is empty or NULL.
 */
static inline __attribute__((no_instrument_function)) LinkedListNode* linked_list_peek_head(const LinkedList* list)
{
    if ((list == NULL) || (list->count == 0U)) {
        return NULL;
    }
    return list->root.next;
}

/**
 * @brief Returns the node at the tail of the list without removing it.
 *
 * @param list Pointer to the LinkedList instance.
 * @return Pointer to tail LinkedListNode, or NULL if list is empty or NULL.
 */
static inline __attribute__((no_instrument_function)) LinkedListNode* linked_list_peek_tail(const LinkedList* list)
{
    if ((list == NULL) || (list->count == 0U)) {
        return NULL;
    }
    return list->root.prev;
}

/**
 * @brief Checks if the list contains zero elements.
 *
 * @param list Pointer to the LinkedList instance.
 * @return true if empty or list is NULL, false otherwise.
 */
static inline __attribute__((no_instrument_function)) bool linked_list_is_empty(const LinkedList* list)
{
    return ((list == NULL) || (list->count == 0U));
}

/**
 * @brief Returns the total number of elements currently stored in the list.
 *
 * @param list Pointer to the LinkedList instance.
 * @return Total element count, or 0 if list is NULL.
 */
static inline __attribute__((no_instrument_function)) size_t linked_list_count(const LinkedList* list)
{
    return (list != NULL) ? list->count : 0U;
}

/**
 * @brief Initializes a doubly linked list container.
 *
 * Configures the internal sentinel root node into an empty circular state
 * and resets the item count to zero.
 *
 * @param list Pointer to the LinkedList instance to initialize.
 * @return true if initialization succeeded, false if list is NULL.
 */
bool linked_list_init(LinkedList* list);

/**
 * @brief Initializes an individual list node.
 *
 * Sets the node's internal pointers to NULL, marking it as unlinked.
 *
 * @param node Pointer to the LinkedListNode instance to initialize.
 * @return true if initialization succeeded, false if node is NULL.
 */
bool linked_list_node_init(LinkedListNode* node);

/**
 * @brief Checks if a list node is currently linked to any list.
 *
 * @param node Pointer to the LinkedListNode to inspect.
 * @return true if the node is linked, false if unlinked or node is NULL.
 */
bool linked_list_node_is_linked(const LinkedListNode* node);

/**
 * @brief Inserts a node at the head (beginning) of the list with defensive checks.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the LinkedListNode to insert.
 * @return true if inserted, false if parameters are NULL or node is already linked.
 */
bool linked_list_insert_head(LinkedList* list, LinkedListNode* node);

/**
 * @brief Inserts a node at the tail (end) of the list with defensive checks.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the LinkedListNode to insert.
 * @return true if inserted, false if parameters are NULL or node is already linked.
 */
bool linked_list_insert_tail(LinkedList* list, LinkedListNode* node);

/**
 * @brief Inserts a node immediately before a specified target node.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @param target Pointer to the existing node in the list before which node is inserted.
 * @param node Pointer to the LinkedListNode to insert.
 * @return true if inserted, false if parameters are NULL or node is already linked.
 */
bool linked_list_insert_before(LinkedList* list, LinkedListNode* target, LinkedListNode* node);

/**
 * @brief Inserts a node immediately after a specified target node.
 *
 * Operation executes in strict O(1) deterministic time.
 *
 * @param list Pointer to the LinkedList instance.
 * @param target Pointer to the existing node in the list after which node is inserted.
 * @param node Pointer to the LinkedListNode to insert.
 * @return true if inserted, false if parameters are NULL or node is already linked.
 */
bool linked_list_insert_after(LinkedList* list, LinkedListNode* target, LinkedListNode* node);

/**
 * @brief Removes a node from its containing list with defensive checks.
 *
 * Operation executes in strict O(1) deterministic time.
 * Resets the removed node's pointers to NULL.
 *
 * @param list Pointer to the LinkedList instance.
 * @param node Pointer to the LinkedListNode to remove.
 * @return true if removed, false if parameters are NULL, list is empty, or node is not linked.
 */
bool linked_list_remove(LinkedList* list, LinkedListNode* node);

/**
 * @brief Returns the node immediately following the current node in the list.
 *
 * @param list Pointer to the LinkedList instance.
 * @param current Pointer to the current node.
 * @return Pointer to the next LinkedListNode, or NULL if current is the tail or invalid.
 */
LinkedListNode* linked_list_next(const LinkedList* list, const LinkedListNode* current);

/**
 * @brief Returns the node immediately preceding the current node in the list.
 *
 * @param list Pointer to the LinkedList instance.
 * @param current Pointer to the current node.
 * @return Pointer to the previous LinkedListNode, or NULL if current is the head or invalid.
 */
LinkedListNode* linked_list_prev(const LinkedList* list, const LinkedListNode* current);

/**
 * @brief Unlinks all nodes and resets the list to an empty state.
 *
 * @param list Pointer to the LinkedList instance.
 */
void linked_list_clear(LinkedList* list);

#endif /* LINKED_LIST_H */
