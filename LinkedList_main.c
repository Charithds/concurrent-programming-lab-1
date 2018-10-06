#include "LinkedList.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

#define OP_MEMBER 0
#define OP_INSERT 1
#define OP_DEL 2

#define TEST_SAMPLE_SIZE 25


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
void init_linked_list(struct node **head, int length, int *random_numbers_array)
{
    for(int i = 0; i < length; i++){
        insert(head, random_numbers_array[i]);
    }
}

// Returns the next insert element 
int get_insert_element(int * random_numbers_array, int *random_array_current_pos)
{
    return random_numbers_array[(*random_array_current_pos)++];
}

// Returns the next delete element
int get_delete_element(int * random_numbers_array, int *random_array_current_pos)
{
    return random_numbers_array[rand()%((*random_array_current_pos)--)];
}

// Returns the next member element
int get_member_element(int *random_numbers_array, int N)
{
    return random_numbers_array[rand()%N];
}

void Usage (char* prog_name) {
   fprintf(stderr, "usage: %s <mMember> <mInsert> <mDelete> <thread_count>\n", prog_name);
   exit(0);
}

double get_elapsed_time(clock_t start, clock_t end){
    return (double)(end-start)/CLOCKS_PER_SEC;
}

double SerialMethod(struct node *head, int *random_numbers_array, int * op_seq, int * random_array_current_pos, int n_init, int m_member, int m_ins, int m_del){
    int random_array_length = 2 * (n_init + m_ins);
    int total_ops = m_ins+m_del+m_member;

    random_array_current_pos = malloc(sizeof(int));
    *random_array_current_pos=0;
    
    random_numbers_array = malloc(sizeof(int)*random_array_length);
    for(int i = 0; i < random_array_length; i++){
        random_numbers_array[i] = rand();
    }

    op_seq = malloc(total_ops*sizeof(int));
    generate_operation_sequence(op_seq, m_member, m_ins, m_del);
    
    init_linked_list(&head, n_init, random_numbers_array);
    *random_array_current_pos = 1000;
    
    clock_t start, end;
    start = clock();
    for(int i = 0; i < total_ops; i++){
        if(op_seq[i] == OP_MEMBER){
            struct node *element = member(head, get_member_element(random_numbers_array, random_array_length));
        } else if(op_seq[i] == OP_INSERT){
            insert(&head, get_insert_element(random_numbers_array, random_array_current_pos));
        }else if(op_seq[i] == OP_DEL){
            struct node *element = delete(&head, get_delete_element(random_numbers_array, random_array_current_pos));
            if(!element){
                // printf("im here");
                element = deleteFirst(&head);
            }
        }
    }
    end = clock();
    // printf("final length of the linked list is %d\n", length(head));
    double elapsed_time = get_elapsed_time(start, end);
    
    // free all the memory allocated
    free(random_numbers_array);
    free(random_array_current_pos);
    free(op_seq);
    struct node *curr;
    while ((curr = head) != NULL) { // set curr to head, stop if list empty.
        head = head->next;          // advance head to next element.
        free (curr);                // delete saved pointer.
    }

    return elapsed_time*1000;
}

void calculateStats(double * elapsedTimes, int count, double *stats){
    double sum=0, variance_sum=0, mean=0, std=0;
    for(int i = 0; i < count; i++) sum += elapsedTimes[i];
    
    mean = sum/count;
    
    for(int i = 0; i < count; i++) variance_sum += pow(elapsedTimes[i]-mean, 2);

    std = sqrt(variance_sum/count);
    stats[0] = mean;
    stats[1] = std;
}

int main (int argc, char* argv[]){
    //printf("%d ", argc);
    if(argc != 5) Usage(argv[0]);
    srand(time(NULL));
    
    /*Parameters of the program*/
    int n_init = 1000;
    int m_ins = atoi(argv[2]), m_del = atoi(argv[3]), m_member = atoi(argv[1]);

    /*  Size of the random array is 2X the required length.
        We can use the extra elements in "member" calls. 
        Also the current size of the linked list can be used accordingly.
        op_seq defines the operation sequence.   
    */
    int * random_numbers_array, * random_array_current_pos, * op_seq;
    
    // The linked list
    struct node * head = NULL;
    
    // to calculate the number of times to run, we need some stats
    double * elapsed_times, * test_stats, * stats;
    elapsed_times = malloc(TEST_SAMPLE_SIZE*sizeof(double));
    test_stats = malloc(2*sizeof(double));

    for(int i=0; i< TEST_SAMPLE_SIZE; i++){ 
        elapsed_times[i] = SerialMethod(head, random_numbers_array, op_seq, random_array_current_pos, n_init, m_member, m_ins, m_del);
    }
    calculateStats(elapsed_times, TEST_SAMPLE_SIZE, test_stats);
    // printf("elaped time mean: %f\n", test_stats[0]);
    // printf("elaped time std: %f\n", test_stats[1]);
    double run_expression = 100*1.96*test_stats[1]/(5*test_stats[0]);
    int runs_required = ceil(pow(run_expression,2));
    runs_required = (runs_required > TEST_SAMPLE_SIZE) ? runs_required:100;
    // printf("runs reqd: %d\n", runs_required);

    free(elapsed_times);
    elapsed_times = malloc(runs_required*sizeof(double));
    stats = malloc(2*sizeof(double));
    for(int i=0; i< runs_required; i++){ 
        elapsed_times[i] = SerialMethod(head, random_numbers_array, op_seq, random_array_current_pos, n_init, m_member, m_ins, m_del);
    }
    calculateStats(elapsed_times,runs_required, stats);
    printf("runs reqd: %d\n", runs_required);
    printf("elaped time mean: %f\n", stats[0]);
    printf("elaped time std: %f\n", stats[1]);
    return 0;
}