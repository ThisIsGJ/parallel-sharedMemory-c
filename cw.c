#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

pthread_mutex_t lock;
pthread_barrier_t   barrier;
pthread_cond_t condvar;

int dimension;
double** array;
int flag;
double precision;
int lineNo;
int counter_thread;
int threadNo;

//initial the square array
double** setArray(int dimension)
{
  static double **r;
  srand(time(NULL));
	
  r = malloc(dimension*sizeof(double *));
  for (int i = 0; i < dimension; ++i)
  {
  	r[i] = malloc(dimension * sizeof(double));
  }

  for (int i = 0; i < dimension; i++)
  {
		for (int j = 0; j < dimension; j++) 
		{	
			r[i][j] = rand()%100;
		}
     
  }
  return r;
}

// the main operation for the array: to get average for each numnber
void *getAverage(void* thread){
	//the thread number of the thread 
	int threadN = (intptr_t) thread;
	//line_No is the number of the rows which the thread need to operate
	int line_No = lineNo;
	pthread_mutex_unlock(&lock);
	pthread_barrier_wait (&barrier);

	//if (dimension-2)/threadNo is not a int, I will let the last thread to 
	//operate remain rows of the array.
	if (threadN == threadNo-1)
	{	
		line_No = dimension-2-lineNo*(threadNo-1);
	}

	double** midArray;
	double **copyArray;
	// The thread's start row number
	int startRowNo = threadN*lineNo+1;
	
	//malloc space to copyArray	
	copyArray = malloc((line_No+2)*sizeof(double *));
	for (int i = 0; i < line_No+2; ++i)
	 {
  		copyArray[i] = malloc(dimension * sizeof(double));
 	}

	//malloc space to midArray
	midArray = malloc(line_No*sizeof(double *));

	for (int i = 0; i < line_No; ++i)
	 {
  		midArray[i] = malloc(dimension * sizeof(double));
 	}	

//flag == 1 mean the precision requirement for the arrat is not finish yet
	while (flag == 1)
	{		
		//after each turn finished, I need to make sure the last finished thread could
		//set the flag to 0 aganin
		// and after that let all the thread start at the same time
		pthread_mutex_lock(&lock);	
		counter_thread++;
		if(counter_thread == threadNo)
		{		
			//You can check each process by using checkAnswer belowåå
			//checkAnswer(array);
			flag = 0;
			counter_thread = 0;
			pthread_cond_broadcast(&condvar);
		}else {					
			pthread_cond_wait(&condvar, &lock);
		}
		pthread_mutex_unlock(&lock);							

		//copy required rows of the array to copyArray	
		for (int i = 0; i < line_No+2; i++) 
		{
			for (int j = 0; j < dimension; j++) 
			{	 
				*(*(copyArray+i)+j) = *(*(array+i+startRowNo-1)+j);
			}
		}

		for (int i = 1; i < line_No+1; i++ )
	 	{	
			for (int j = 1; j < dimension-1; j++) {		
				double newValue = (copyArray[i+1][j] + copyArray[i-1][j]
								+ copyArray[i][j+1] + copyArray[i][j-1])/4;
				
				//check if it get the precision
				if(flag == 0){
					if(fabs(copyArray[i][j] - newValue) >= precision)
					{	
						pthread_mutex_lock(&lock);
						flag = 1;
						pthread_mutex_unlock(&lock);
					}	
				
				}
				//give new value to midArray
				//midArray stores the update the averaged number 
				*(*(midArray+i-1)+j-1) = newValue;		
			}  				     
	  }
	
		pthread_barrier_wait (&barrier);
		//update the original array by using the midArray
		for (int i = 0; i < line_No; i++) 
		{
			for (int j = 1; j < dimension-1; j++) 
			{
				*(*(array+i+startRowNo)+j) = midArray[i][j-1];
			}	
		}
		pthread_barrier_wait (&barrier);
	}
	free(copyArray);
	free(midArray);	
	return NULL;
}

//Used to print the new array and check the answer
int checkAnswer(double **array){	
	printf("\n");
	for (int i = 0; i < dimension; i++ )
	  {	
		for (int j = 0; j < dimension; j++) {
				printf( "%f\t", *(*(array+i)+j));
			}       
			printf("\n");
	  }
	  return 0;
}

int main()
{
//The 4 input is initialized below 
	//initial the dimension of the square array
	dimension = 5;
	//initial the number of thread
	threadNo = 2;
	//initial the square array 
	array = setArray(dimension);
	//intial the precsion of my project
	precision = 1;
	//if the number of thread is greater than the dimension, I let the 
	//thread number equal to the dimension. 
	if ((dimension-2) < threadNo)
	{
		threadNo = dimension-2;
	}

	pthread_t* threadID;
	pthread_attr_t attr;
	//allocate the space to threadID
	threadID = malloc(threadNo * sizeof(pthread_t));	
	pthread_barrier_init (&barrier, NULL, threadNo);
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init (&condvar, NULL);
	pthread_attr_init(&attr);
  	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  	//counter_thread is used to find the last thread and let the last thread 
  	//to make set the flag to 0 - used in the getAverage() function
	counter_thread = 0;

	//0 means finish precision work 
	//1 means didn't precision work
	flag = 1;  

	// lineNo is the number of rows of the array operated by each thread
	//the last thread make operate the different number of rows which 
	//calculate in gatAverage() function 
	lineNo = (dimension-2)/threadNo;
	
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
	    return 1;
	 }	
	
	printf("The initialized Array:\n");
	checkAnswer(array);

	//creat thread 
	for (int i = 0; i < threadNo; i++) {
		pthread_mutex_lock(&lock);
		pthread_create(&threadID[i], &attr,getAverage,(void *)(intptr_t)i);
	}
		
	for(int j = 0; j < threadNo; j++)
	{
	   pthread_join(threadID[j], NULL);
	 }

	pthread_attr_destroy(&attr);
  	pthread_mutex_destroy(&lock);
  	pthread_cond_destroy(&condvar);
  	pthread_barrier_destroy(&barrier);
	
	printf("This is the final answer:\n");
	checkAnswer(array);
  	printf("Get Final Answer\n");
	return 0;
}

