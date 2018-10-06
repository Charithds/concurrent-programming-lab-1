#include "LinkedList.h"

// struct node *head = NULL;

//display the list
void printList(struct node *head)
{
    struct node *ptr = head;
    printf("\n[ ");

    //start from the beginning
    while (ptr != NULL)
    {
        printf("%d ", ptr->data);
        ptr = ptr->next;
    }

    printf(" ]");
}

//insert link at the first location
void insert(struct node **head, int data)
{
    //create a link
    struct node *link = (struct node *)malloc(sizeof(struct node));

    link->data = data;
    // printf(" %d ", link->data);

    //point it to old first node
    link->next = *head;

    //point first to new first node
    *head = link;
    // printf(" %d ", head);
}

//is list empty
bool isEmpty(struct node *head)
{
    return head == NULL;
}

int length(struct node *head)
{
    int length = 0;
    struct node *current;

    for (current = head; current != NULL; current = current->next)
    {
        length++;
    }

    return length;
}

//member a link with given key
struct node *member(struct node *head, int data)
{

    //start from the first link
    struct node *current = head;

    //if list is empty
    if (head == NULL)
    {
        return NULL;
    }

    //navigate through list
    while (current->data != data)
    {

        //if it is last node
        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            //go to next link
            current = current->next;
        }
    }

    //if data found, return the current Link
    return current;
}

//delete a link with given key
struct node *delete (struct node **head, int data)
{

    //start from the first link
    struct node *current = *head;
    struct node *previous = NULL;

    //if list is empty
    if (*head == NULL)
    {
        return NULL;
    }

    //navigate through list
    while (current->data != data)
    {

        //if it is last node
        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if (current == *head)
    {
        //change first to point to next link
        *head = (*head)->next;
    }
    else
    {
        //bypass the current link
        previous->next = current->next;
    }

    return current;
}

// delete the first item
struct node* deleteFirst(struct node **head) {
    //save reference to first link
    struct node *tempLink = *head;

    //mark next to first link as first
    *head = (*head)->next;
    
    //return the deleted link
    return tempLink;
}

/*
void main()
{
    struct node *head = NULL;

    insert(&head, 10);
    insert(&head, 20);
    insert(&head, 30);
    insert(&head, 1);
    insert(&head, 40);
    insert(&head, 56);

    printf("Original List: %d ", length(head));

    //print list
    printList(head);
    while(!isEmpty(head)) {            
        struct node *temp = deleteFirst(&head);
        printf("\nDeleted value:");
        printf("(%d,%d) ",temp->data);
   }  
    printf("\nList after deleting all items: ");
    printList(head);
    insert(&head, 10);
    insert(&head, 20);
    insert(&head, 30);
    insert(&head, 1);
    insert(&head, 40);
    insert(&head, 56);

    printf("\nRestored List: ");
    printList(head);
    printf("\n");

    struct node *foundLink = member(head, 30);

    if (foundLink != NULL)
    {
        printf("Element found: ");
        printf("%d ", foundLink->data);
        printf("\n");
    }
    else
    {
        printf("Element not found.");
    }

    delete (&head, 30);
    printf("List after deleting an item: ");
    printList(head);
    printf("\n");
    foundLink = member(head, 30);

    if (foundLink != NULL)
    {
        printf("Element found: ");
        printf("%d ", foundLink->data);
        printf("\n");
    }
    else
    {
        printf("Element not found.");
    }

}
*/