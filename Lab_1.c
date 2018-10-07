#include "LinkedList.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define OP_MEMBER 0
#define OP_INSERT 1
#define OP_DEL 2
#define MAX_NUM 65536

#define READ_WRITE_LOCK "R"
#define MUTEX "M"
#define SERIAL "S"
#define TEST_SAMPLE_SIZE 20

int random_array_length, total_ops, ops_per_thread, thread_count;
int * random_numbers_array, * random_array_current_pos;
int n_init, m_ins, m_del, m_member;
    
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

// Initializes the linked list to given size
void init_linked_list(struct node **head, int length, int *random_numbers_array){
    for(int i = 0; i < length; i++){
        insert(head, random_numbers_array[i]);
    }
}

void Usage (char* prog_name) {
   fprintf(stderr, "Usage: %s <total ops> <mMember> <mInsert> <mDelete> <thread_count> <prog_type (M,R or S)>\n", prog_name);
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

double SerialMethod(){
    srand(time(NULL));
    
    random_array_current_pos = malloc(sizeof(int));
    *random_array_current_pos=0;
    
    random_numbers_array = malloc(sizeof(int)*random_array_length);
    for(int i = 0; i < random_array_length; i++){
        random_numbers_array[i] = rand()%MAX_NUM;
    }
    
    init_linked_list(&head, n_init, random_numbers_array);
    *random_array_current_pos = 1000;
    
    clock_t start, end;
    start = clock();
    for(int i = 0; i < total_ops; i++){
        int prob = rand()%total_ops;
        if(prob < m_member){
            struct node *element = member(head, rand()%MAX_NUM);
        } else if(prob < m_member+m_ins){
            insert(&head, random_numbers_array[(*random_array_current_pos)++]);
        }else{
            struct node *element = delete(&head, random_numbers_array[rand()%((*random_array_current_pos)--)]);
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
    struct node *curr;
    while ((curr = head) != NULL) { // set curr to head, stop if list empty.
        head = head->next;          // advance head to next element.
        free (curr);                // delete saved pointer.
    }

    return elapsed_time*1000;
}

void * linkedList_worker_mutex(void* args){
    for(int i = 0; i < ops_per_thread; i++){
        int prob = rand()%total_ops;
        if(prob < m_member){
            pthread_mutex_lock(&lock);
            struct node *element = member(head, rand()%MAX_NUM);
            pthread_mutex_unlock(&lock);
        } else if(prob < m_member+m_ins){
            pthread_mutex_lock(&lock);
            insert(&head, random_numbers_array[(*random_array_current_pos)++]);
            pthread_mutex_unlock(&lock);
        }else{
            pthread_mutex_lock(&lock);
            struct node *element = delete(&head, random_numbers_array[rand()%((*random_array_current_pos)--)]);
            if(!element){
                element = deleteFirst(&head);
            }
            pthread_mutex_unlock(&lock);
        }
    }
}

double MutexMethod(){
    srand(time(NULL));

    random_array_current_pos = malloc(sizeof(int));
    *random_array_current_pos=0;
    
    random_numbers_array = malloc(sizeof(int)*random_array_length);
    for(int i = 0; i < random_array_length; i++){
        random_numbers_array[i] = rand()%MAX_NUM;
    }
    
    init_linked_list(&head, n_init, random_numbers_array);
    *random_array_current_pos = 1000;
    
    ops_per_thread = total_ops/thread_count;
    // printf("ops_p_t :%d\n", ops_per_thread);
    pthread_mutex_init(&lock, NULL);
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    clock_t start, end;
    start = clock();
    for(int i=0;i<thread_count;i++){
        pthread_create(&(thread_handles[i]), NULL, linkedList_worker_mutex, NULL);
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
    struct node *curr;
    while ((curr = head) != NULL) { // set curr to head, stop if list empty.
        head = head->next;          // advance head to next element.
        free (curr);                // delete saved pointer.
    }
    pthread_mutex_destroy(&lock);

    return elapsed_time*1000;
}

void * linkedList_worker_rwlock(void* args){
    for(int i = 0; i < ops_per_thread; i++){
        int prob = rand()%total_ops;
        if(prob < m_member){
            pthread_rwlock_rdlock(&rwlock);
            struct node *element = member(head, rand()%MAX_NUM);
            pthread_rwlock_unlock(&rwlock);
        } else if(prob < m_member + m_ins){
            pthread_rwlock_wrlock(&rwlock);
            insert(&head, random_numbers_array[(*random_array_current_pos)++]);
            pthread_rwlock_unlock(&rwlock);
        }else{
            pthread_rwlock_wrlock(&rwlock);
            struct node *element = delete(&head, random_numbers_array[rand()%((*random_array_current_pos)--)]);
            if(!element){
                element = deleteFirst(&head);
            }
            pthread_rwlock_unlock(&rwlock);
        }
    }
}

double RWLockMethod(){
    srand(time(NULL));

    random_array_current_pos = malloc(sizeof(int));
    *random_array_current_pos=0;
    
    random_numbers_array = malloc(sizeof(int)*random_array_length);
    for(int i = 0; i < random_array_length; i++){
        random_numbers_array[i] = rand()%MAX_NUM;
    }
    
    init_linked_list(&head, n_init, random_numbers_array);
    *random_array_current_pos = 1000;
    
    ops_per_thread = total_ops/thread_count;
    // printf("ops_p_t :%d\n", ops_per_thread);
    pthread_rwlock_init(&rwlock, NULL);
    thread_handles = malloc(thread_count*sizeof(pthread_t));

    clock_t start, end;
    start = clock();
    for(int i=0;i<thread_count;i++){
        pthread_create(&(thread_handles[i]), NULL, &linkedList_worker_rwlock, NULL);
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
    if(argc != 7) Usage(argv[0]);
    
    /*Parameters of the program*/
    n_init = 1000;
    total_ops = atoi(argv[1]);
    m_ins = (int)(total_ops * atof(argv[3])), m_del = (int)(total_ops * atof(argv[4])), m_member = (int)(total_ops * atof(argv[2]));
    thread_count = atoi(argv[5]);
    char *program = argv[6];

    random_array_length = 2 * (n_init + m_ins);
    total_ops = m_ins+m_del+m_member;
    
    // to calculate the number of times to run, we need some stats
    double * elapsed_times, * test_stats, * stats;
    elapsed_times = malloc(TEST_SAMPLE_SIZE*sizeof(double));
    test_stats = malloc(2*sizeof(double));

    for(int i=0; i< TEST_SAMPLE_SIZE; i++){
        if(strcmp(program, READ_WRITE_LOCK)==0){
            elapsed_times[i] = RWLockMethod(n_init, m_member, m_ins, m_del, thread_count);
            // printf("R\n");
        }else if(strcmp(program, MUTEX)==0){
            elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
            // printf("M\n");
        }else if(strcmp(program, SERIAL)==0){
            // printf("S\n");
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
        if(strcmp(program, READ_WRITE_LOCK)==0){
            // printf("R\n");
            elapsed_times[i] = RWLockMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, MUTEX)==0){
            // printf("M\n");
            elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
        }else if(strcmp(program, SERIAL)==0){
            // printf("S\n");
            elapsed_times[i] = SerialMethod(n_init, m_member, m_ins, m_del);
        }
    }
    calculateStats(elapsed_times,runs_required, stats);
    printf("runs required: %d\n", runs_required);
    printf("mean: %f ", stats[0]);
    printf("std: %f \n", stats[1]);
    return 0;
}
