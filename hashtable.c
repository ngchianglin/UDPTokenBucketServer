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
 A hash table implementation to store ip4 buckets
 It uses double hashing with the hash defines by the
 combination of the two auxiliary hash functions. 
 The hash table uses mutex to allow thread safe
 operation on the hash structure. 
 To read/change/modify hash data safely a data mutex 
 needs to be used by the modifier/readers.  
 
 Ng Chiang Lin
 April 2017
*/


#include "ratelimit.h"


static struct ip4bucket ht[HASHSZ];
static size_t hashsize;

/* hash lock for hash structure */
static pthread_mutex_t htlock = PTHREAD_MUTEX_INITIALIZER;

/* hash data lock for the data itself */
pthread_mutex_t ht_datalock = PTHREAD_MUTEX_INITIALIZER;

/* Auxiliary hash function 1 */
static size_t hash1(unsigned int ip)
{
   size_t ret = (size_t) (ip % HASHSZ);
   return  ret;
}

/* Auxiliary hash function 2 */
static size_t hash2(unsigned int ip)
{
   size_t ret = (size_t)  ( (ip % (HASHSZ -1)) + 1) ;
   return ret;
}

/* 
 Hash function for the key makes 
 use of hash function 1 and 2. 
 */
static size_t hash(unsigned int ip, size_t i)
{
  size_t ret = (size_t)(( hash1(ip) + i * hash2(ip) ) % HASHSZ );
  return ret;
}


/* Initializes the hash table */
void initHashTable(void)
{
  size_t i;
  hashsize=0;
  for(i=0;i<HASHSZ;i++)
    empty_ip4_bucket(&ht[i]);
}


/*
 Adds a ip4bucket item into the hash table 
 Takes an integer value as the hash key 
 and the ip4bucket struct to be added.
 Returns 1 if successful, 0 otherwise.
*/
size_t put(unsigned int k, struct ip4bucket v)
{
    size_t i, ret, index; 
   
    i=0; ret =0;
   
    if(k==0)
      return ret;


    pthread_mutex_lock(&htlock);
    while(i < HASHSZ)
    {
        index = hash(k, i);
        if (ht[index].ipv4 == 0 || ht[index].ipv4 == k )
        {//empty slot or duplicate
         //for duplicate old value is overwritten
         
          if(ht[index].ipv4 == 0 ) //new hash entry
          {      
               hashsize++;
               ht[index].ipv4 = k;
          }
          ret=1;

          pthread_mutex_lock(&ht_datalock); //lock data
          pthread_mutex_unlock(&htlock); //unlock hash

          copy_ip4_bucket_data(&v ,&ht[index]); //update data
          pthread_mutex_unlock(&ht_datalock); //unlock data
           
          return ret; 
        }
        else 
        {
           i++; 
        }
    }
    
   pthread_mutex_unlock(&htlock); //unlock hash
   return ret;     
}

/*
 Retrieves a ip4 bucket item
 from hash table at the specified
 index. Takes an size_t index value as
 parameter.
 Returns NULL if index exceeds
 the array size of the hash table 
 or hash item is empty. 
 Returns a pointer to the ip4bucket item
 in the hash table if not empty. 
*/
struct ip4bucket *getHashItem(size_t i)
{
   struct ip4bucket * ret= NULL; 
   pthread_mutex_lock(&htlock);
   if (i < HASHSZ  && ht[i].ipv4 != 0)
   {
      ret = &ht[i];
      pthread_mutex_unlock(&htlock);
      return ret;
   }

   pthread_mutex_unlock(&htlock);
   return ret; 
}

/*
 Retrieves an ip4bucket from
 the hash table using the specified key. 
 Takes unsigned int key as parameter.
 Returns the a pointer to ip4bucket
 item if found, NULL otherwise.   
*/
struct ip4bucket* get(unsigned int k)
{

   size_t i, index;
   i=0;

   if(k==0)
      return NULL;

   pthread_mutex_lock(&htlock);
   while(i < HASHSZ)
   {
      index = hash(k,i);
      if(ht[index].ipv4  != 0 && ht[index].ipv4 == k)
      {
        pthread_mutex_unlock(&htlock);
        return &ht[index];
      } 
      else
      {
        i++;
      }
   }

   pthread_mutex_unlock(&htlock);
   return NULL; 
}

/*
 Removes ip4bucket from hash table
 that has the specified key. 
 Takes an unsigned int key as argument
 Returns 0 if item is not found, 
 1 if item is deleted. 
*/

size_t removeHashItem(unsigned int k)
{
   size_t i, index, ret;
   i=0; ret=0;

    if(k==0)
      return ret;

   pthread_mutex_lock(&htlock);
   while(i < HASHSZ)
   {
      index=hash(k, i);
      if(ht[index].ipv4  != 0 && ht[index].ipv4 == k)
      {
          ret=1;
          hashsize--; 
          ht[index].ipv4 = 0;
          
          pthread_mutex_lock(&ht_datalock); //lock data
          pthread_mutex_unlock(&htlock); // unlock hash

          empty_ip4_bucket(&ht[index]); //empty data
          pthread_mutex_unlock(&ht_datalock); //unlock data
          
          return ret;
      }       
      else
      {
         i++;
      }

   }

  pthread_mutex_unlock(&htlock);
  return ret;
}


