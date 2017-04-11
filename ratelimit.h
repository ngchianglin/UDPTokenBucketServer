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
 Header file to define the parameters
 and structures for a udp ip4 token
 bucket server program.  

 Ng Chiang Lin
 April 2017
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>


/* Rate definitions */
#define MAX_TOKENS 50

/* 1 token refill per 1 second */
#define TOKEN_REFILL 1
#define SLEEP_INTERVAL 1


/* Queue Definitions*/

#define QUEUESZ 256
#define MSGSZ 64

struct queue_item
{
 struct sockaddr_storage peer_addr;
 socklen_t peer_addr_len;
 char msg[MSGSZ];  
};


struct queue
{
 size_t size;
 size_t front;
 size_t end;
 pthread_mutex_t lock;
 pthread_cond_t qready;
 struct queue_item qarray[QUEUESZ];
};

void initQueue(struct queue * q);
int enqueue( struct queue * q, struct queue_item * item);
struct queue_item* dequeue(struct queue * q );

struct queue input_queue;

/* IPv4 Bucket definitions */
#define IP4_CHAR_LEN 16

/* 
 IP Bucket structure 
 The first member ipv4 
 serves as the key for the hash
 table that is used later. 
 count and addr are considered
 as the actual hash data
*/
struct ip4bucket
{
 unsigned int ipv4;
 int count;
 char addr[IP4_CHAR_LEN];
};


unsigned int parseIP4(const char * p);
void copy_ip4_bucket_data(struct ip4bucket *s, struct ip4bucket *d);
void empty_ip4_bucket(struct ip4bucket *s);


/* Hash table definitions */

/*
 Use a prime number for hash table size 
 The hash table itself is a static array 
 defined in hashtable.c 
*/
#define HASHSZ 4093

extern pthread_mutex_t ht_datalock;

void initHashTable(void);
size_t put(unsigned int k, struct ip4bucket v);
struct ip4bucket *getHashItem(size_t i);
struct ip4bucket * get(unsigned int k);
size_t removeHashItem(unsigned int k);

#define BUFSZ 64


