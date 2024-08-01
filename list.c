#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"


// 创建链表
Node *createList()
{
    return NULL;
}

// 创建新节点
Node *createNode(const char *name, const char *addr)
{
    Node *newNode = (Node *) malloc(sizeof(Node));

    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    strcpy(newNode->name, name);
    strcpy(newNode->addr, addr);
    newNode->next = NULL;
    return newNode;
}

// 添加节点到链表头部
void addNode(Node ** head, const char *name, const char *addr)
{
    Node *newNode = createNode(name, addr);

    newNode->next = *head;
    *head = newNode;
}

// 根据名称删除节点
void removeNode(Node ** head, const char *name)
{
    Node *current = *head;
    Node *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Node with name '%s' not found!\n", name);
}

// 销毁链表
void destroyList(Node ** head)
{
    Node *current = *head;
    Node *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

// 打印链表
void printList(Node * head)
{
    Node *current = head;

    while (current != NULL) {
        printf("%-8s\t%-16s \n", current->name, current->addr);
        current = current->next;
    }
}

/*
int main()
{
    Node *head = createList();

    addNode(&head, "John", "123 Main St");
    addNode(&head, "Alice", "456 Elm Ave");

    printf("Original List:\n");
    printList(head);

    removeNode(&head, "John");

    printf("List after removing 'John':\n");
    printList(head);

    destroyList(&head);

    return 0;
}
*/
