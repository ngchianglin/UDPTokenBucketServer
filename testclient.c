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
A test client for the UDP server

For a good tutorial on network programming, refer to 
Beej's Guide to Network Programming
http://beej.us/guide/bgnet/ 


Ng Chiang Lin
April 2017
*/


#include "ratelimit.h"


#define BUFSZ 64
#define MAXTHREADS 20


struct threadparam
{
int clientsocket;
char *msg;
};


void *testthreads(void *arg)
{
   struct timespec ts;
   struct threadparam *param = NULL;
   char buf[BUFSZ]; 
   char sbuf[16];
   char *msg;
   int i, len;

   ts.tv_sec=1;
   ts.tv_nsec=0 * 1000000; //0 milliseconds 

   if(arg == NULL)
   {
      fprintf(stderr,"NULL thread parameter\n");
      exit(EXIT_FAILURE);
   }
   

   param = (struct threadparam *)arg; 
  
  
  i=1;
  while(1)
  {

     if(param->msg != NULL )
     {
         msg=param->msg;
     }
     else
     {
         snprintf(sbuf, 16, "%s%d", "192.168.10.", i); 
         i++;
         if(i==255)
           i=0; 

        msg=sbuf; 
     }
     
    len=strlen(msg) + 1;
     

     
     if( write(param->clientsocket, msg, len) != len )
     {
        perror("unable to write to socket successfully");
     }

     len = read(param->clientsocket, buf, BUFSZ);
     if (len == -1) 
     {
        perror("Error reading from socket");
     }
     else
     {
         buf[BUFSZ -1]='\0';
         printf("%s  reponse from server: %s\n",msg, buf); 
     }

     nanosleep(&ts, NULL);
  }


}



int main(int argc, char* argv[])
{

   int status=0, i, clientsocket;
   struct addrinfo hints;
   struct addrinfo *clientip;
   pthread_t tids[MAXTHREADS];
   struct threadparam tparams[MAXTHREADS];
   struct timeval tv;

   char argstr [8][90]={ {"0.0.0.0"}, 
                         {"."},
                         {"-1.-2.-3.-4"},
                         {"192.168."},
                         {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"},
                         {"B.BB.B.B"},
                         {"192.168.3.30"},
                         {"192.168.3.30"}             
                       };


   if(argc != 3)
   {
       fprintf(stderr,"Usage: %s host port\n", argv[0]);
       exit(EXIT_FAILURE);
   }

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_INET;    
   hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */


  if ((status=getaddrinfo(argv[1], argv[2], &hints, &clientip)) != 0)
  {
     fprintf(stderr, "Unable to obtain ip information, getaddrinfo error: %s\n", 
     gai_strerror(status));
     exit(EXIT_FAILURE);
  }

  for(i=0;i<MAXTHREADS;i++)
  {

      clientsocket= socket(clientip->ai_family, clientip->ai_socktype, clientip->ai_protocol);
      if(clientsocket == -1)
      {
          perror("error getting client socket");
          exit(EXIT_FAILURE);
      } 

      if ( connect(clientsocket, clientip->ai_addr, clientip->ai_addrlen) == -1)
      {
          perror("error connecting socket");
          exit(EXIT_FAILURE);
      }

      
      tv.tv_sec = 0;
      tv.tv_usec = 300000; //300 milliseconds time out

      if (setsockopt(clientsocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) 
      {
            perror("Error setting timeout for socket");
            exit(EXIT_FAILURE);
      }
        
      tparams[i].clientsocket = clientsocket;
      if(i<8)
         tparams[i].msg=argstr[i]; 
      else
         tparams[i].msg=NULL;
  
     printf("%d Msg: %s clientsocket: %d\n", i, tparams[i].msg , tparams[i].clientsocket); 

     if( pthread_create(&tids[i], NULL, testthreads, (void *)&tparams[i] ) != 0 )
     {
        perror("Cannot thread");
        exit(EXIT_FAILURE);
     }

  }
  


 for(i=0;i<MAXTHREADS;i++)
    pthread_join(tids[i], NULL);

 return 0;

}
















