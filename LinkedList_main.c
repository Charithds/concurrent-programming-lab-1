#include "LinkedList.h"
#include <stdio.h>
#include <time.h>

#define OP_MEMBER 0
#define OP_INSERT 1
#define OP_DEL 2

int * random_numbers_ar, * number_ar_current_pos;

// A utility function to swap to integers 
void swap (int *a, int *b){ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
}

// A function to generate a random permutation of arr[] 
void randomize ( int arr[], int n ){ 
    srand ( time(NULL) ); 
    for (int i = n-1; i > 0; i--) 
    { 
        int j = rand() % (i+1); 
        swap(&arr[i], &arr[j]); 
    } 
}

// A funtion to generate shuffled operation sequence
void generate_operation_sequence(int * op_seq, int member, int insert, int del){
    int total_ops = member+insert+del;
    for(int i = 0; i < member; i++) op_seq[i] = OP_MEMBER;
    for(int i = member; i < member+insert; i++) op_seq[i] = OP_INSERT;
    for(int i = member+insert; i < total_ops; i++) op_seq[i] = OP_DEL;
    randomize(op_seq, total_ops);
}

// Initializes the linked list to given size
void init_linked_list(struct node **head, int length)
{
    for(int i = 0; i < length; i++){
        insert(head, random_numbers_ar[i]);
    }
}

// Returns the next insert element 
int get_insert_element()
{
    return random_numbers_ar[(*number_ar_current_pos)++];
}

// Returns the next delete element
int get_delete_element()
{
    return random_numbers_ar[rand()%((*number_ar_current_pos)--)];
}

// Returns the next member element
int get_member_element(int N)
{
    return random_numbers_ar[rand()%N];
}

int main (){
    srand(time(NULL));
    
    /*Parameters of the program*/
    int n_init = 1000;
    int m_ins = 50, m_del = 50, m_member = 9900;
    int total_ops = m_ins+m_del+m_member;

    /*  Size of the random array is 2X the required length.
        We can use the extra elements in "member" calls. 
        Also the current size of the linked list can be used accordingly.   */
    int random_ar_length = 2 * (n_init + m_ins);
    number_ar_current_pos = malloc(sizeof(int));
    *number_ar_current_pos=0;
    
    random_numbers_ar = malloc(sizeof(int)*random_ar_length);
    for(int i = 0; i < random_ar_length; i++){
        random_numbers_ar[i] = rand();
    }

    int * op_seq = malloc(total_ops*sizeof(int));
    generate_operation_sequence(op_seq, m_member, m_ins, m_del);

    // The linked list
    struct node * head = NULL;
    init_linked_list(&head, n_init);
    *number_ar_current_pos = 1000;
    
    // just a test var
    int test = 0;
    for(int i = 0; i < total_ops; i++){
        if(op_seq[i] == OP_MEMBER){
            struct node *element = member(head, get_member_element(random_ar_length));
            // if(element != NULL) printf("member %d\n", element->data);
            // else printf("NULL\n");
        } else if(op_seq[i] == OP_INSERT){
            insert(&head, get_insert_element());
            printf("insert\n");
            // printf("%d", OP_INSERT);
        }else if(op_seq[i] == OP_DEL){
            struct node *element = delete(&head, get_delete_element());
            if(element != NULL) printf("delete %d\n", element->data);
            else printf("NULL\n");
            // printf("%d", OP_DEL);
        }
        test++;
    }
    printf("%d",test);
    
    return 0;
}