#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <pthread.h> 

long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false; // for file reading

static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex5 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;  


// structure for a linked list element
struct q
{ 
    int data;
    struct q *next;
};
struct q *top = NULL;
struct q *bottom = NULL;

// creating linked list
void que(int item)
{
    struct q *qptr = malloc(sizeof(struct q)); // allocates memory for a linked list element
    qptr->data = item;
    qptr->next = NULL;
    if (bottom == NULL)
    {
        top = qptr;
        bottom = qptr;
    }
    else
    {
        bottom->next = qptr;
        bottom = qptr;
    }
}


int pick() // picks an element from the top of the linked list
{         
    if (top == NULL)
    {
        printf("queue is empty \n");
        return 0;
    }
    else
      {
        struct q *temp;
	      int data;
      	data = top->data;
        temp = top;
        top = top->next;

	      if (top == NULL)
          {
	          bottom = NULL;
          }
          free(temp); // free ups memory for linked list element
          return data;
     }
}

// unedited original function for single thread
void calculate_square(long number)
{
  long the_square = number * number;
  sleep(number);
  sum += the_square;
  if (number % 2 == 1) {
    odd++;
  }
  if (number < min) {
    min = number;
  }
  if (number > max) {
    max = number;
  }
}


// function for multiple threads
void* func_thread(void *tid) // tid stands for thread ID
{
  
  while (true) {
  
    pthread_mutex_lock(&mutex5); // lock mutex5

    while((top == NULL) && (done == false)) 
    {
      pthread_cond_wait(&cond1, &mutex5); // conditional wait when linked list is empty and no file to read
      
    }


    if ((top == NULL) && (done == true)) 
    {
      pthread_mutex_unlock(&mutex5); // unlock mutex5
      pthread_exit(NULL);
    }


    int number = pick(); // picking element from the top of the queue  
    pthread_mutex_unlock(&mutex5); 
    pthread_cond_signal(&cond1); // signal for conditional variable
      
    long the_square = number * number;
    sleep(number);

    // critical section 1
    pthread_mutex_lock(&mutex1); // locking mutex1
    sum += the_square;
    pthread_mutex_unlock(&mutex1); // unlocking mutex1

    // critical section 2
    if (number % 2 == 1) {
      pthread_mutex_lock(&mutex2); // locking mutex2
      odd++;                       
      pthread_mutex_unlock(&mutex2);// unlocking mutex2
    }
    
    // critical section 3
    if (number < min) {
      pthread_mutex_lock(&mutex3); // locking mutex3
      min = number;
      pthread_mutex_unlock(&mutex3); // unlocking mutex3
    }
    
    // critical section 4
    if (number > max) {
      pthread_mutex_lock(&mutex4); // locking mutex4
      max = number;
      pthread_mutex_unlock(&mutex4); // unlocking mutex4
    }

  }  
}


int main(int argc, char* argv[])
{
  // check and parse command line options
  if (argc != 3) 
  {
        exit(EXIT_FAILURE);
  }
  char *fn = argv[1]; // reading file name
  int tnum = atoi(argv[2]); // reading number of threads

  
  if(tnum < 1)
  {
    exit(EXIT_FAILURE);
  }

  
  else if(tnum == 1)
  {
    FILE* fin = fopen(fn, "r");
    char action;
    long num;

    while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {
      if (action == 'p') {            
        calculate_square(num);
      } else if (action == 'w') {    
        sleep(num);
      } 
    }
    fclose(fin);
    
    // print outputs
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);
    
    // clean up and return
    return (EXIT_SUCCESS);
  }

 
  else
  {
    // declaring thread attributes 
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t thread_group[tnum]; // declaring group of threads
    
    for (int i = 0; i < tnum; i++)
    {
      pthread_create(&thread_group[i], NULL, func_thread, NULL); // creating pthreads
    }

  //Opening file 
  FILE* fin = fopen(fn, "r");
    char action;
    long num;

    while (fscanf(fin, "%c %ld\n", &action, &num) == 2) 
    {
      if (action == 'p') 
      {                            
      que(num);                
      } 
      else if (action == 'w') 
      {     
        sleep(num);
      } 
 
    }
    fclose(fin);
    
    done = true; 
    
    for (int i = 0; i < tnum; i++) 
    {
      pthread_cond_broadcast(&cond1); 
      pthread_join(thread_group[i], NULL); // joining pthreads
    }

    // print outputs
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);
      
    // clean up and return
    return (EXIT_SUCCESS);
  }
}

