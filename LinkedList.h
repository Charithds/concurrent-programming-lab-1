#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node{
    int data;
    struct node *next;
};
void printList(struct node *head);
void insert(struct node **head, int data);
bool isEmpty(struct node *head);
int length(struct node *head);
struct node* member(struct node *head, int data);
struct node* delete(struct node **head, int data);

#endif