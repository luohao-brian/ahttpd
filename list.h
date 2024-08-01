#ifndef __LIST_H
#define __LIST_H

// 链表节点结构体
typedef struct Node {
    char name[16];
    char addr[32];
    struct Node *next;
} Node;


// 创建链表
extern Node *createList();
// 创建新节点
extern Node *createNode(const char *name, const char *addr);
// 添加节点到链表头部
void addNode(Node ** head, const char *name, const char *addr);
// 根据名称删除节点
void removeNode(Node ** head, const char *name);
// 销毁链表
void destroyList(Node ** head);
// 打印链表
void printList(Node * head);

#endif
