/*
* MIT License
*
* Copyright (c) 2017 Ng Chiang Lin
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/*
 A simple queue implementation
 It uses mutex to enable thread safe
 operation. A conditional wait is used to make
 dequeue operation blocking where there is no
 data in the queue. 

 Ng Chiang Lin
 April 2017
*/

#include "ratelimit.h"


/*Initializes a queue */
void initQueue(struct queue * q)
{
  if(q == NULL)
    return;

  memset(q,0, sizeof(struct queue));
  pthread_mutex_init(&q->lock, NULL);
  pthread_cond_init(&q->qready,NULL);
}


/*
Enqueues a queue item
Takes a pointer to a queue and
the queue item as parameters. 
Returns 1 if success, -1 otherwise

*/
int enqueue( struct queue * q, struct queue_item* item)
{
   
   if(q == NULL || item == NULL)
     return -1; 

   pthread_mutex_lock( &q->lock );

   if( q->size >= QUEUESZ)
   {
       pthread_cond_signal(&q->qready);
       pthread_mutex_unlock(&q->lock); 
       return -1;
   }

   q->size++; 
   q->qarray[q->end] = *item;
   q->end++;
   if(q->end == QUEUESZ)
         q->end = 0; 

   pthread_mutex_unlock(&q->lock); 
   pthread_cond_signal(&q->qready);
   return 1;

}


/*
Dequeues a queue item, blocks
if there is no data in queue
Takes a pointer to a queue 
and return a pointer to a queue_item 
if successful, NULL otherwise
*/

struct queue_item* dequeue(struct queue * q )
{
   struct queue_item * ret=NULL;

   if(q == NULL)
      return NULL;

    pthread_mutex_lock(&q->lock );
    while(q->size == 0)
    { //conditional block when no data in queue
       pthread_cond_wait(&q->qready, &q->lock);
    }
   
    q->size--;
    ret = &q->qarray[q->front]; 
    q->front++; 
    if(q->front == QUEUESZ)
          q->front=0; 
    pthread_mutex_unlock(&q->lock); 
    
    return ret;
  
}
