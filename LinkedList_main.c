#include "LinkedList.h"
#include <stdio.h>
#include <time.h>

void init_linked_list(struct node **head, int * number_ar, int length)
{
    for(int i = 0; i < length; i++){
        insert(head, number_ar[i]);
    }
}

int main (){
    // create a list of random numbers
    // n = 1,000 and m = 10,000, mMember = 0.99, mInsert = 0.005, mDelete = 0.005
    // 9900 50 50
    // n = 1,000 and m = 10,000, mMember = 0.90, mInsert = 0.05, mDelete = 0.05
    // 9000 500 500
    // n = 1,000 and m = 10,000, mMember = 0.50, mInsert = 0.25, mDelete = 0.25
    // 5000 2500 2500 

    // init list should initialize with 1000 random numbers
    // shuffling of ops
    /*
        if op == 1 insert or something like that
    */
    struct node * head = NULL;
    
    srand(time(NULL));
    int n_init = 1000;
    int m_ins = 50;
    int m_del = 50;
    int N = n_init + m_ins;

    int * numbers_ar = malloc(sizeof(int)*N);
    for(int i = 0; i < N; i++){
        numbers_ar[i] = rand();
    }
    
    init_linked_list(&head, numbers_ar, n_init);
    printList(head);
}