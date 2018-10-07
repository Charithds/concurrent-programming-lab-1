#include "LinkedList.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define OP_MEMBER 0
#define OP_INSERT 1
#define OP_DEL 2

#define READ_WRITE_LOCK "RWL"
#define MUTEX "M"
#define SERIAL "S"
#define TEST_SAMPLE_SIZE 25

int random_array_length, total_ops, ops_per_thread, thread_count;
int * random_numbers_array, * random_array_current_pos, * op_seq;
    
// The linked list
struct node * head = NULL;

pthread_mutex_t lock;
pthread_rwlock_t rwlock;
pthread_t * thread_handles;

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
void init_linked_list(struct node **head, int length, int *random_numbers_array){
    for(int i = 0; i < length; i++){
        insert(head, random_numbers_array[i]);
    }
}

// Returns the next insert element 
int get_insert_element(){
    return random_numbers_array[(*random_array_current_pos)++];
}

// Returns the next delete element
int get_delete_element(){
    return random_numbers_array[rand()%((*random_array_current_pos)--)];
}

// Returns the next member element
int get_member_element(){
    return random_numbers_array[rand()%random_array_length];
}

void Usage (char* prog_name) {
   fprintf(stderr, "Usage: %s <mMember> <mInsert> <mDelete> <thread_count> <prog_type (M,RWL or S)>\n", prog_name);
   printf("For the serial program <thread_count> will be ignored.");
   exit(0);
}

double get_elapsed_time(clock_t start, clock_t end){
    return (double)(end-start)/CLOCKS_PER_SEC;
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

double SerialMethod(int n_init, int m_member, int m_ins, int m_del){
    srand(time(NULL));
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

void * linkedList_worker_mutex(void* args){
    // printf("IM here.\n");
    int tid = *((int*) args);
    int start = tid*ops_per_thread;
    int end = (tid==thread_count-1) ? total_ops:(start+ops_per_thread);
    // printf("worker %d", tid);
    for(int i = start; i < end; i++){
        if(op_seq[i] == OP_MEMBER){
            pthread_mutex_lock(&lock);
            struct node *element = member(head, get_member_element());
            pthread_mutex_unlock(&lock);
        } else if(op_seq[i] == OP_INSERT){
            pthread_mutex_lock(&lock);
            insert(&head, get_insert_element());
            pthread_mutex_unlock(&lock);
        }else if(op_seq[i] == OP_DEL){
            pthread_mutex_lock(&lock);
            struct node *element = delete(&head, get_delete_element());
            if(!element){
                element = deleteFirst(&head);
            }
            pthread_mutex_unlock(&lock);
        }
    }
}

double MutexMethod(int n_init, int m_member, int m_ins, int m_del, int thread_count){
    srand(time(NULL));
    /*  Size of the random array is 2X the required length.
        We can use the extra elements in "member" calls. 
        Also the current size of the linked list can be used accordingly.
        op_seq defines the operation sequence.   
    */
    random_array_length = 2 * (n_init + m_ins);
    total_ops = m_ins+m_del+m_member;

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
    
    ops_per_thread = total_ops/thread_count;
    // printf("ops_p_t :%d\n", ops_per_thread);
    pthread_mutex_init(&lock, NULL);
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    clock_t start, end;
    start = clock();
    for(int i=0;i<thread_count;i++){
        // printf("thread");
        int *tid = malloc(sizeof(int));
        *tid = i;
        pthread_create(&(thread_handles[i]), NULL, linkedList_worker_mutex, tid);
    }

    for(int i=0;i<thread_count;i++){
        pthread_join(thread_handles[i], NULL);
    }
    end = clock();
    // printf("final length of the linked list is %d\n", length(head));
    double elapsed_time = get_elapsed_time(start, end);
    
    // free all the memory allocated
    free(thread_handles);
    free(random_numbers_array);
    free(random_array_current_pos);
    free(op_seq);
    struct node *curr;
    while ((curr = head) != NULL) { // set curr to head, stop if list empty.
        head = head->next;          // advance head to next element.
        free (curr);                // delete saved pointer.
    }
    pthread_mutex_destroy(&lock);

    return elapsed_time*1000;
}

void * linkedList_worker_rwlock(void* args){
    // printf("IM here.\n");
    int tid = *((int*) args);
    int start = tid*ops_per_thread;
    int end = (tid==thread_count-1) ? total_ops:(start+ops_per_thread);
    // printf("worker %d", tid);
    for(int i = start; i < end; i++){
        if(op_seq[i] == OP_MEMBER){
            pthread_rwlock_rdlock(&rwlock);
            member(head, get_member_element());
            pthread_rwlock_unlock(&rwlock);
        } else if(op_seq[i] == OP_INSERT){
            pthread_rwlock_wrlock(&rwlock);
            insert(&head, get_insert_element());
            pthread_rwlock_unlock(&rwlock);
        }else if(op_seq[i] == OP_DEL){
            pthread_rwlock_wrlock(&rwlock);
            struct node *element = delete(&head, get_delete_element());
            if(!element){
                element = deleteFirst(&head);
            }
            pthread_rwlock_unlock(&rwlock);
        }
    }
}

double RWLockMethod(int n_init, int m_member, int m_ins, int m_del, int thread_count){
    srand(time(NULL));
    /*  Size of the random array is 2X the required length.
        We can use the extra elements in "member" calls. 
        Also the current size of the linked list can be used accordingly.
        op_seq defines the operation sequence.   
    */
    random_array_length = 2 * (n_init + m_ins);
    total_ops = m_ins+m_del+m_member;

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
    
    ops_per_thread = total_ops/thread_count;
    // printf("ops_p_t :%d\n", ops_per_thread);
    pthread_rwlock_init(&rwlock, NULL);
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    clock_t start, end;
    start = clock();
    for(int i=0;i<thread_count;i++){
        // printf("thread");
        int *tid = malloc(sizeof(int));
        *tid = i;
        pthread_create(&(thread_handles[i]), NULL, linkedList_worker_rwlock, tid);
    }

    for(int i=0;i<thread_count;i++){
        pthread_join(thread_handles[i], NULL);
    }
    end = clock();
    // printf("final length of the linked list is %d\n", length(head));
    double elapsed_time = get_elapsed_time(start, end);
    
    // free all the memory allocated
    free(thread_handles);
    free(random_numbers_array);
    free(random_array_current_pos);
    free(op_seq);
    struct node *curr;
    while ((curr = head) != NULL) { // set curr to head, stop if list empty.
        head = head->next;          // advance head to next element.
        free (curr);                // delete saved pointer.
    }
    pthread_rwlock_destroy(&rwlock);

    return elapsed_time*1000;
}

int main (int argc, char* argv[]){
    //printf("%d ", argc);
    if(argc != 6) Usage(argv[0]);
    
    /*Parameters of the program*/
    int n_init = 1000;
    int m_ins = atoi(argv[2]), m_del = atoi(argv[3]), m_member = atoi(argv[1]);
    thread_count = atoi(argv[4]);
    char *program = argv[5];
    
    // to calculate the number of times to run, we need some stats
    double * elapsed_times, * test_stats, * stats;
    elapsed_times = malloc(TEST_SAMPLE_SIZE*sizeof(double));
    test_stats = malloc(2*sizeof(double));

    for(int i=0; i< TEST_SAMPLE_SIZE; i++){
        if(strcmp(program, READ_WRITE_LOCK)){
            elapsed_times[i] = RWLockMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, MUTEX)){
            elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, SERIAL)){
            elapsed_times[i] = SerialMethod(n_init, m_member, m_ins, m_del);
        }else{
            printf("Wrong program argument");
            exit(0);
        }
    }
    calculateStats(elapsed_times, TEST_SAMPLE_SIZE, test_stats);
    // printf("elaped time mean: %f\n", test_stats[0]);
    // printf("elaped time std: %f\n", test_stats[1]);
    double run_expression = 100*1.96*test_stats[1]/(5*test_stats[0]);
    int runs_required = ceil(pow(run_expression,2));
    runs_required = (runs_required > TEST_SAMPLE_SIZE) ? runs_required:TEST_SAMPLE_SIZE;
    // printf("runs reqd: %d\n", runs_required);

    free(elapsed_times);
    elapsed_times = malloc(runs_required*sizeof(double));
    stats = malloc(2*sizeof(double));
    for(int i=0; i< runs_required; i++){ 
        if(strcmp(program, READ_WRITE_LOCK)){
            elapsed_times[i] = RWLockMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, MUTEX)){
            elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, SERIAL)){
            elapsed_times[i] = SerialMethod(n_init, m_member, m_ins, m_del);
        }else{
            printf("Wrong program argument");
            exit(0);
        }
    }
    calculateStats(elapsed_times,runs_required, stats);
    printf("runs required: %d\n", runs_required);
    printf("elapsed time mean: %f milliseconds\n", stats[0]);
    printf("elapsed time std: %f milliseconds\n", stats[1]);
    return 0;
}
