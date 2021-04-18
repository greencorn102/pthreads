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


// for queue
struct q
{
 
    int data;
    struct q *next;
};
struct q *top = NULL;
struct q *bottom = NULL;

// for queue
void que(int item)
{
    struct q *qptr = malloc(sizeof(struct q));
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


int pick() // picks an element from the top of the queue
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
          free(temp);
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
void* func_thread(void *tid)
{
  
  while (true) {
  
    pthread_mutex_lock(&mutex5); // lock mutex5

    while((top == NULL) && (done == false)) 
    {
      pthread_cond_wait(&cond1, &mutex5);
      
    }


    if ((top == NULL) && (done == true)) 
    {
      pthread_mutex_unlock(&mutex5); // unlock mutex5
      pthread_exit(NULL);
    }


    int number = pick();   
    pthread_mutex_unlock(&mutex5); 
    pthread_cond_signal(&cond1);
      
    long the_square = number * number;
    sleep(number);

    //Critical Section 1
    pthread_mutex_lock(&mutex1); // locking mutex1
    sum += the_square;
    pthread_mutex_unlock(&mutex1); // unlocking mutex1

    if (number % 2 == 1) {
      //Critical Section 2
      pthread_mutex_lock(&mutex2); // locking mutex2
      odd++;                       
      pthread_mutex_unlock(&mutex2);// unlocking mutex2
    }
    if (number < min) {
      //Critical Section 3
      pthread_mutex_lock(&mutex3); // locking mutex3
      min = number;
      pthread_mutex_unlock(&mutex3); // unlocking mutex3
    }
    if (number > max) {
      //Critical Section 4
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
    printf("Usage: sumsq <infile>\n");
    exit(EXIT_FAILURE);
  }
  char *fn = argv[1]; // file name
  int tnum = atoi(argv[2]); // number of threads

  
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
    
    // print results
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

    // print results
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);
      
    // clean up and return
    return (EXIT_SUCCESS);
  }
}

