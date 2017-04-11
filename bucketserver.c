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
A UDP server that serves token 
using token bucket algorithm for IPv4 addresses
Ng Chiang Lin
April 2017
*/

#include "ratelimit.h"


/* 
Setup and binds the UDP server socket to a host and port
Takes a host, port string as well as a pointer to the
serversocket as parameters.  
*/
void bindSocket(const char* host, const char* port, int *serversocket)
{

  int status=0;
  struct addrinfo hints;
  struct addrinfo *serverip;
 
  memset(&hints,0, sizeof(struct addrinfo));
  hints.ai_family=AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ((status=getaddrinfo(host, port, &hints, &serverip)) != 0)
  {
     fprintf(stderr, "Unable to obtain ip information, getaddrinfo error: %s\n", 
     gai_strerror(status));
     exit(EXIT_FAILURE);
  }

  if(serverip->ai_family == AF_INET)
  {
       struct sockaddr_in *ipv4 =  (struct sockaddr_in *)serverip->ai_addr;
       void *addr = &(ipv4->sin_addr);
       char ipstr[INET6_ADDRSTRLEN];
       if( inet_ntop(serverip->ai_family, addr, ipstr, INET6_ADDRSTRLEN) == NULL)
       {
           fprintf(stderr, "Error converting ip address to string\n");
           exit(EXIT_FAILURE);   
       }

       printf("%s %s %s %s\n", "Using local ip:", ipstr, " UDP port:", port);
  }
  else
  {
      fprintf(stderr, "Program currently don't support Ipv6 yet !\n");
      exit(EXIT_FAILURE);
  }

  *serversocket = socket(serverip->ai_family, serverip->ai_socktype,
                serverip->ai_protocol);

  if (bind(*serversocket, serverip->ai_addr, serverip->ai_addrlen) == 0)
  {
       printf("Binded to address\n");
  }
  else
  {
      fprintf(stderr, "Unable to bind to address and port\n");
      exit(EXIT_FAILURE);
  }

  freeaddrinfo(serverip);

}


/* 
 Validates that a ip message string is valid
 Returns 0 for invalid ip string or
 the unsigned int representation 
 of the ip4 string message. 
*/  
unsigned int validateMessage(const char *msg)
{
  unsigned int ret=0;
  if(strlen(msg) > (IP4_CHAR_LEN - 1) )
  {
     fprintf(stderr, "Invalid ip message %s %zu\n", msg, strlen(msg)); 
     return ret;
  }

  ret=parseIP4(msg); 
  if(ret==0)
      fprintf(stderr, "Invalid key %s %u\n", msg, ret); 
         
  return ret; 
}

/* 
Creates a new ip bucket and add to
hashtable. Takes the unsigned int key k 
and ip message string as parameters. 
Returns 1 if successful,
0 otherwise
*/
size_t addNewBucket(unsigned int k, const char *msg)
{
  struct ip4bucket ip_bucket;
  size_t i, len;

  len=strlen(msg);
  
  if(msg == NULL || len >= IP4_CHAR_LEN )
     return 0;

  empty_ip4_bucket(&ip_bucket);
  ip_bucket.ipv4=k;
  ip_bucket.count=MAX_TOKENS - 1;

  for(i=0;i<len;i++)
    ip_bucket.addr[i] = msg[i];
  
  ip_bucket.addr[i] = '\0';
            
  return put(ip_bucket.ipv4, ip_bucket);

}



/* 
Processing thread, consumes items from the input queue 
and process it. Takes the serversocket descriptor 
as threat argument. Sends udp response Ok if rate limit 
is not exceeded otherwise sends NOK. 
*/
void *processing(void *arg)
{
   struct queue_item *p;
   struct ip4bucket *ipb;
   int status; 
   int *serversocket; 
   unsigned int k;
   char *ok = "OK";
   char *nok = "NOK";
  
   serversocket = (int *) arg; 
    

   while(1)
   {
      p=dequeue(&input_queue);
      if(p!=NULL)
      {
         if( (k = validateMessage(p->msg)) == 0 )
            continue;      

         ipb = get(k);
         if(ipb == NULL)
         {//bucket not present in hash table
 
            if(addNewBucket(k,p->msg))
            {
               if(sendto(*serversocket, ok, 3, 0,
                 (struct sockaddr *) &p->peer_addr, p->peer_addr_len) != 3)
                     fprintf(stderr, "Error sending ok response\n");
            }
            else
            {
               fprintf(stderr, "Unable to add to hash table\n");
               if(sendto(*serversocket, nok, 4, 0,
                 (struct sockaddr *) &p->peer_addr, p->peer_addr_len) != 4)
                     fprintf(stderr, "Error sending nok response\n");
            }

         }
         else
         {//bucket already exists
             status=0;
             pthread_mutex_lock(&ht_datalock); //lock data
             if (ipb->count > 0 )
             {
                 ipb->count--;
                 status=1;
             }
             pthread_mutex_unlock(&ht_datalock); //unlock data

            if(status)
            {//within rate limit
                 if(sendto(*serversocket, ok, 3, 0,
                   (struct sockaddr *) &p->peer_addr, p->peer_addr_len) != 3)
                      fprintf(stderr, "Error sending ok response\n");
               
            }
            else
            {//exceeds rate limit

               if(sendto(*serversocket, nok, 4, 0,
                 (struct sockaddr *) &p->peer_addr, p->peer_addr_len) != 4)
                     fprintf(stderr, "Error sending nok response\n");
            }

         }

      }
   
   }

}


/*
Token Update thread
Loops through all the ipv4 buckets
in the hashtable and refill their
token according to the configured rate
*/
void *update(void *arg)
{
   struct ip4bucket *ipb;
   size_t i;
   int remove, rate;
   struct timespec ts;

   rate=TOKEN_REFILL;
   ts.tv_sec=SLEEP_INTERVAL;
   ts.tv_nsec=0; 

   while(1)
   {
       for(i=0;i<HASHSZ;i++)
       {
           ipb = getHashItem(i);
           remove=0;
           if(ipb != NULL)
           {
               pthread_mutex_lock(&ht_datalock); //lock data
               ipb->count+=rate;
               if(ipb->count > MAX_TOKENS )
                  remove=1;
               pthread_mutex_unlock(&ht_datalock); //unlock data
       
               if(remove)
               {
                 if(removeHashItem(ipb->ipv4) != 1)
                     fprintf(stderr, "Error removing item from hashtable\n"); 
               }

           }

       }

     nanosleep(&ts, NULL);

   }

}


int main(int argc, char* argv[])
{

  char *LISTEN_HOST="localhost";
  char *LISTEN_PORT="3211";
  int serversocket; 
  ssize_t num; 
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;
  char buf[BUFSZ];
  pthread_t tid1, tid2;
  struct queue_item qt;
  
  peer_addr_len=sizeof(peer_addr);

  printf("Initializing queues\n");
  initQueue(&input_queue);

  printf("Initializing hash tables\n");
  initHashTable();

  printf("Creating Token Bucket Rate Processing thread \n");

  if( pthread_create(&tid1, NULL, processing, (void *)&serversocket) != 0)
     fprintf(stderr, "Cannot create processing thread\n");

 
  printf("Creating Token Bucket Rate Refilling thread \n");
  if( pthread_create(&tid2, NULL, update, NULL) != 0 )
     fprintf(stderr, "Cannot create update thread\n");


  bindSocket(LISTEN_HOST,LISTEN_PORT , &serversocket);
  printf("Waiting for connections\n");


  while(1)
  {
    
    num = recvfrom(serversocket, buf, BUFSZ, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);

    if(num == -1)
    {
        perror("Network error");
        fprintf(stderr, "Network error: received %zd\n", num);
        continue;
    }
    
    if(num == 0)
       continue;   

    //The expected correct message string
    //should not be larger than BUFSZ
    buf[BUFSZ -1] = '\0';
         
    qt.peer_addr = peer_addr;
    qt.peer_addr_len = peer_addr_len;
    strncpy(qt.msg, buf, strlen(buf) + 1);

    if( enqueue(&input_queue, &qt) == -1)
      fprintf(stderr, "Unable to queue message \n"); 
   
  }

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
   
  return 0;

}
