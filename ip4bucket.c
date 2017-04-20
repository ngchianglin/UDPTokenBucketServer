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
 Functions for handling ip4bucket structures
 and to parse ip4 string into its integer
 representation 

 Ng Chiang Lin
 April 2017
*/

#include "ratelimit.h"


/*
 Copies the ip4bucket structures. 
 Takes a ip4bucket pointer as source
 parameter and another ip4bucket pointer 
 as destination.
 Copies the source data to destination data
 Note that that hash key ipv4 of the
 ip4bucket struct is not copied. This key is
 set by the hash table function.  
*/

void copy_ip4_bucket_data(struct ip4bucket *s, struct ip4bucket *d)
{

   size_t i;
   if(s== NULL || d == NULL)
     return;

   d->count = s->count;
   for(i=0;i<IP4_CHAR_LEN;i++)
     d->addr[i] = s->addr[i];
}

/*
Zeroed the ip4bucket structure
passed in as the parameter
*/
void empty_ip4_bucket(struct ip4bucket *s)
{
   size_t i;
   if(s==NULL)
      return;
 
   s->ipv4=0;
   s->count=0;
   for(i=0;i<IP4_CHAR_LEN;i++)
     s->addr[i] = '\0';

}



/*
 Parses an ipv4 string into 
 its integer representation
 Takes a string containing the ipv4
 string as parameter.
 Returns 0 if the ipv4 string is
 not valid. Returns unsigned int representation
 of the ipv4 string if successful. 
*/
unsigned int parseIP4(const char * p)
{

   char octets[4][4]; 
   size_t octnum=0 , index=0, i;
   char c;
   int val;
   unsigned int ret;
   

   while(*p != '\0')
   {
      c=*p;
      switch(c)
      {
           case '0':
           case '1':
           case '2':
           case '3':
           case '4':
           case '5':
           case '6':
           case '7':
           case '8':
           case '9':
             octets[octnum][index] = c;
             index++; 
             if(index == 4) // Invalid, each ipv4 octet max 3 digit
             {
                fprintf(stderr,"More digits than allowed %c\n", c);
                return 0; 
             }
             break;
           case '.':
             if(index == 0)
             {
                fprintf(stderr,"Empty octet \n");
                return 0; 
             }   
             octets[octnum][index] = '\0';
             octnum ++;
             index = 0; 
             if(octnum == 4) // Invalid each ipv4 max 4 octets 
             {
                fprintf(stderr,"More octets than allowed %c\n", c);
                return 0;
             }
             break;
           default:
                fprintf(stderr, "Invalid character %c\n", c);
                return 0; //Invalid format/char for ipv4
      }
 
      p++; 
  
   }

   if( !(octnum == 3 && index > 0) )
   {
       fprintf(stderr, "Less than 4 octets\n");
       return 0;    
   }
   
   octets[octnum][index] = '\0';

   ret = 0; 

   for(i=0;i<4;i++)
   {
      val = atoi(octets[i]);
      if(val > 255 || val < 0 )
      {
         fprintf(stderr, "Value not allowed for ipv4 octet\n");
         return 0;
      }

      ret = ret | val ;
      if(i<3) 
        ret = ret << 8;
   }
   
   return ret;

}

