#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT_SIZE 16
#define CHUNK_SIZE 65536

typedef struct Header {
   size_t size;
   struct Header *next;
   void *data;
   int free; /*free if asserted*/
} Header;

static Header *current = NULL;
static intptr_t *programBreak = NULL;
static int memLeft = CHUNK_SIZE;

void *myMalloc(size_t size) {
   int remainder = 0;
   int headerSize = 0;
   int dataSize = 0;
   int memRequired = 0;
   void *returnPtr = NULL;
   Header *traversing = NULL;
   Header header = {size, NULL, NULL};
   
   if (size == 0)
      return NULL;

   /*CHECK IF THIS IS THE FIRST TIME CALLED*/
   if (!programBreak) {
      programBreak = (intptr_t *)sbrk(CHUNK_SIZE);
      if (programBreak  == (void *)-1) {
         return NULL;
      }
      else {
         programBreak += ((unsigned int)programBreak % ALIGNMENT_SIZE); 
      }   
   }

   /*CHECK IF THERE IS ENGOUH MEMORY FOR HUNK REQUESTED*/
   headerSize = sizeof(Header) + (sizeof(Header) % ALIGNMENT_SIZE);
   dataSize = header.size + (header.size % ALIGNMENT_SIZE);
   memRequired = headerSize + dataSize;
   header.size = dataSize;

   if (( memLeft - headerSize - dataSize) < 0) {
      programBreak = (intptr_t *)sbrk(CHUNK_SIZE);
      memLeft = CHUNK_SIZE;
   }
   else {
      memLeft = memLeft - memRequired;
   }

   /*STORING THE HEADER INTO THE MEMORY*/
   if (!current) { /*first allocation of memory to the user*/
      memcpy(programBreak, &header, sizeof(Header));
      current = &header;
   }
   else {
      /*TRAVERSING THROUGHT THE LINKED LIST TO FIND WHERE TO STORE REQUEST*/
      traversing = programBreak; 
      while (traversing && traversing->next) {
         traversing = traversing->next;
         if (traversing.free && traversing.size >= memRequired) {
            memcpy(traversing, &header, sizeof(Header));
            break;
         }
      }
      
   }

   current->data = (unsigned int)current + headerSize;
   returnPtr = current->data;
   //current->next = (unsigned int)current->data + dataSize;

   return returnPtr;
}


void *mycalloc(size_t nmemb, size_t size) {
   char *arr = (char *)myMalloc(size*nmemb);
   int i = 0;
   for (i = 0; i < size*nmemb; i++) {
      arr[i] = 0;
   }
   
   return arr;
}

void free(void *ptr) {
}

void *realloc(void *ptr, size_t size) {
   if (ptr == 0 && size == 0)
      malloc(size);
   else if (size == 0)
      free(ptr);
   else if (ptr == 0)
      return NULL;
}
