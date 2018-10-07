<<<<<<< HEAD
#include "LinkedList.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define OP_MEMBER 0
#define OP_INSERT 1
#define OP_DEL 2

#define TEST_SAMPLE_SIZE 25

int random_array_length, total_ops, ops_per_thread, thread_count;
int * random_numbers_array, * random_array_current_pos, * op_seq;
    
// The linked list
struct node * head = NULL;

pthread_mutex_t lock;
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
void init_linked_list(struct node **head, int length, int *random_numbers_array)
{
    for(int i = 0; i < length; i++){
        insert(head, random_numbers_array[i]);
    }
}

// Returns the next insert element 
int get_insert_element()
{
    return random_numbers_array[(*random_array_current_pos)++];
}

// Returns the next delete element
int get_delete_element()
{
    return random_numbers_array[rand()%((*random_array_current_pos)--)];
}

// Returns the next member element
int get_member_element()
{
    return random_numbers_array[rand()%random_array_length];
}

void Usage (char* prog_name) {
   fprintf(stderr, "usage: %s <mMember> <mInsert> <mDelete> <thread_count>\n", prog_name);
   exit(0);
}

double get_elapsed_time(clock_t start, clock_t end){
    return (double)(end-start)/CLOCKS_PER_SEC;
}

void * linkedList_worker(void* args){
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
        pthread_create(&(thread_handles[i]), NULL, linkedList_worker, tid);
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
    thread_count = atoi(argv[4]);
    
    // to calculate the number of times to run, we need some stats
    double * elapsed_times, * test_stats, * stats;
    elapsed_times = malloc(TEST_SAMPLE_SIZE*sizeof(double));
    test_stats = malloc(2*sizeof(double));

    for(int i=0; i< TEST_SAMPLE_SIZE; i++){ 
        elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
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
        elapsed_times[i] = MutexMethod(n_init, m_member, m_ins, m_del, thread_count);
    }
    calculateStats(elapsed_times,runs_required, stats);
    printf("runs reqd: %d\n", runs_required);
    printf("elaped time mean: %f\n", stats[0]);
    printf("elaped time std: %f\n", stats[1]);
    return 0;
=======
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct list_node_s
{
 int data;
 struct list_node_s* next;
};
void *thread_function(void *);
int Member(int value);
int Insert(int value);
int Delete(int value);
int n;                 //number of values 
int m;                 //number of operations in each execution
float m_member_frac;   //member fraction
float m_insert_frac;   //insert fraction
float m_delete_frac;   //delete fraction
pthread_mutex_t mutex, mutex1;
int count, count_m, count_i, count_d = 0;
int m_member, m_insert, m_delete;
int thread_count;   //number of threads
struct list_node_s* head_p = NULL;
int* opr_values;

int main(int argc, char* argv[]){    //argc : no. of arguments and argv : vector of arguments
 long thread;
 pthread_t* thread_handles;
 clock_t start, end;
 double cpu_time_used;
 srand (time(NULL));
 int i, ins_value;

 if (argc != 7)
 {
        //error in input values
  fprintf(stderr, "error in input format. correct format: <thread_count> <n> <m> <m_member_frac> <m_insert_frac> <m_delete_frac>\n");
  exit(0);
 }
 thread_count = strtol(argv[1], NULL, 10);
 if (thread_count <= 0)
 {
  fprintf(stderr, "error. thread count should be greater than 0\n");

  exit(0);
 }
 n = (int) strtol(argv[2], NULL, 10);
 m = (int) strtol(argv[3], NULL, 10);
 m_member_frac = (float) atof(argv[4]);
 m_insert_frac = (float) atof(argv[5]);
 m_delete_frac = (float) atof(argv[6]);
 if(n < 0 || m < 0 || m_member_frac < 0 || m_insert_frac < 0 || m_delete_frac < 0 || m_member_frac+m_insert_frac+m_delete_frac != 1.0){
  //error
  fprintf(stderr, "error in input values.\n");
  fprintf(stderr, "n: no of initial values in the linkedlist.\n");
  fprintf(stderr, "m: no. of values for carrying out operations\n");
  fprintf(stderr, "m_member_frac: fraction of operations for member function\n");
  fprintf(stderr, "m_insert_frac: fraction of operations for insert function\n");
  fprintf(stderr, "m_delete_frac: fraction of operations for delete function\n");
  exit(0);
 }

 opr_values = malloc(m*sizeof(int));
 m_member = m * m_member_frac;
 m_insert = m * m_insert_frac;
 m_delete = m - (m_member + m_insert);

 for (i = 0; i < n; i++)
 {
  ins_value = rand() % 65535; //value should be between 2^16 - 1
  Insert(ins_value);
 }
 for (i = 0; i < m; i++)
 {
  opr_values[i] = rand() % 65535; //value should be between 2^16 - 1
 }

 thread_handles = malloc(thread_count*sizeof(pthread_t));
 start = clock();
 pthread_mutex_init(&mutex, NULL);
 for (thread = 0; thread < thread_count; thread++)
 {
  pthread_create(&thread_handles[thread], NULL, thread_function, (void*) thread);
 }
 for (thread = 0; thread < thread_count; thread++)
 {
  pthread_join(thread_handles[thread], NULL);
 }
 end = clock();
 cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
FILE *pFile;
 pFile=fopen("data-mutex.txt", "a");
 fprintf(pFile, "%f\n", cpu_time_used);
 printf("execution time is : %f\n", cpu_time_used);
 free(thread_handles);
 return 0;
}
void *thread_function(void* rank){ 
 
//int *opr_values = (int *) arg;
//long my_rank = (long)rank;

 pthread_mutex_lock( &mutex1 ); 
 int i = count++;
 pthread_mutex_unlock( &mutex1 ); 

 while(i < m){
  //only one thread can get access to the linked-list at a time
  if(count_m < sizeof(m_member)){
   pthread_mutex_lock( &mutex );    
   Member(opr_values[i]);
   pthread_mutex_unlock( &mutex ); 
  }

  else if(count_i < sizeof(m_insert) + sizeof(m_member)){
   pthread_mutex_lock( &mutex );    
   Insert(opr_values[i]);
   pthread_mutex_unlock( &mutex ); 
  }

  else{
   pthread_mutex_lock( &mutex );    
   Delete(opr_values[i]);
   pthread_mutex_unlock( &mutex );
  }

  pthread_mutex_lock( &mutex1 ); 
  i = count++;
  pthread_mutex_unlock( &mutex1 ); 
 }
 return NULL;
}
//Member function
int Member(int value){
 count_m++;
 struct list_node_s* curr_p = head_p;
 while(curr_p != NULL && curr_p->data < value){
  curr_p = curr_p->next;
 }
 if(curr_p == NULL || curr_p->data > value){
  return 0;
 }
 else{
  return 1;
 }
}
//insert function
int Insert(int value){
 count_i++;
 struct list_node_s** head_pp = &head_p;
 struct list_node_s* curr_p = *head_pp;
 struct list_node_s* pred_p = NULL;
 struct list_node_s* temp_p;
 while(curr_p != NULL && curr_p->data < value){
  pred_p = curr_p;
  curr_p = curr_p->next;
 }
 if(curr_p == NULL || curr_p->data > value){
  temp_p = malloc(sizeof(struct list_node_s));
  temp_p->data = value;
  temp_p->next = curr_p;
  if(pred_p == NULL) //new first node
   *head_pp = temp_p;
  else
   pred_p->next = temp_p; 
  return 1;
 }
 else{
  return 0; //value already in the list
 }
}

//delete function
int Delete(int value){
 count_d++;
 struct list_node_s** head_pp = &head_p;
 struct list_node_s* curr_p = *head_pp;
 struct list_node_s* pred_p = NULL;
 while(curr_p != NULL && curr_p->data < value){
  pred_p = curr_p;
  curr_p = curr_p->next;
 }
 if(curr_p != NULL && curr_p->data == value){
  if(pred_p == NULL){
   *head_pp = curr_p->next;  //deleting the first node in the list
   free(curr_p);
  }
  else{
   pred_p->next = curr_p->next;
   free(curr_p);
  }
  return 1;
 }
 else{
  return 0; //value not in the list
 }
>>>>>>> 3e78b01d9dd428871139c436deccbd55bfe73dd1
}