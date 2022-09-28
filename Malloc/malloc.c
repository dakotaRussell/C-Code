#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT_SIZE 16
#define CHUNK_SIZE 65536

typedef struct Header {
   size_t dataSize;
   struct Header *next;
   void *data;
   int free; /*free if asserted*/
} Header;

static Header *programBreak = NULL;
static int memLeft = CHUNK_SIZE;

void *malloc(size_t size) {
   Header header = {size, NULL, NULL, 0};
   int memRequired = 0;
   int headerSize = 0;
   int headSet = 0; 
   Header *walkingPtr = NULL;
   void *returnPtr = NULL;

   if (size == 0)
      return NULL;

   /*CHECK IF THIS IS THE FIRST TIME CALLED*/
   if (!programBreak) {
      programBreak = (Header *)sbrk(CHUNK_SIZE);
      if (programBreak  == (void *)-1) {
         return NULL;
      }
      else {
         programBreak += ((unsigned int)programBreak % ALIGNMENT_SIZE); 
      }   
   }

   /*CHECK IF THERE IS ENGOUH MEMORY FOR HUNK REQUESTED*/
   header.dataSize += (header.dataSize % ALIGNMENT_SIZE);
   headerSize = sizeof(Header) + (sizeof(Header) % ALIGNMENT_SIZE);
   memRequired = headerSize + header.dataSize;

   if (memRequired > memLeft) {
      walkingPtr = programBreak; 
      while (walkingPtr && walkingPtr->next)
         walkingPtr = walkingPtr->next;
      walkingPtr = (intptr_t *)sbrk(CHUNK_SIZE);
      memLeft = CHUNK_SIZE;
   }
   else {
      memLeft = memLeft - memRequired;
   }

   /*STORING THE HEADER INTO THE MEMORY*/
   if (!headSet) { /*first allocation of memory to the user*/
      memcpy(programBreak, &header, sizeof(Header));
      headSet = 1;
      returnPtr = programBreak + headerSize;
   }
   else {
      /*TRAVERSING THROUGHT THE LINKED LIST TO FIND WHERE TO STORE REQUEST*/
      walkingPtr = programBreak; 
      while (walkingPtr && walkingPtr->next) {
         walkingPtr = walkingPtr->next;
         if (walkingPtr->free == 1 && memRequired <= (headerSize + 
           walkingPtr->dataSize)) {
            header.next = walkingPtr->next;
            break; 
         }
      }

      walkingPtr->next = walkingPtr + headerSize + walkingPtr->dataSize; 
      memcpy(walkingPtr->next, &header, sizeof(Header));
      returnPtr = walkingPtr->next + headerSize; 
   }


   return returnPtr;
}


void *calloc(size_t nmemb, size_t size) {
   char *arr = (char *)malloc(size*nmemb);
   int i = 0;
   for (i = 0; i < size*nmemb; i++) {
      arr[i] = 0;
   }
   
   return arr;
}

void free(void *ptr) {
   Header *myBlock = (Header *)ptr;
   Header *walkingPtr = programBreak; 

   myBlock->free = 1; 

   while (walkingPtr && walkingPtr->next) {
      if (walkingPtr->free && walkingPtr->next->free) {
         walkingPtr->next = walkingPtr->next->next;
      }
   }
}

void *realloc(void *ptr, size_t size) {
   Header *myBlock = (Header *)ptr;
   Header dummy = {0, NULL, NULL, 0};

   if (ptr == 0 && size == 0)
      malloc(size);
   else if (size == 0 && !ptr)
      free(ptr);
   else if (!ptr)
      return NULL;
   else {
      if (ptr >= (programBreak + CHUNK_SIZE) || ptr < programBreak) {
         return NULL;
      }
      else if (size < myBlock->dataSize) { /*shrinking*/
         if ((sizeof(Header) + (sizeof(Header) % ALIGNMENT_SIZE) + 
           ALIGNMENT_SIZE) <= (myBlock.dataSize - size)) { /*can the space be used for future calls*/ 
            myBlock->dataSize = size + (size % ALIGNMENT_SIZE); 
            dummy.next = myBlock->next;
            dummy.free = 1; 
            dummy.data = myBlock->data;
            myBlock->next = myBlock + sizeof(Header) + 
              (sizeof(Header) % ALIGNMENT_SIZE) + myBlock->dataSize;
            memcpy(myBlock->next, &dummy, sizeof(Header));
            return myBlock->data; 
         }
      }
      else { /*expanding*/
         if (myBlock->next && myBlock->next->free == 0) { /*cannot expand into adjacent blocks*/
            free(ptr);
            returnPtr = malloc(size);
         }
         else if (myBlock->next && myBlock->next->free && 
           (myBlock->next->next - myBlock->data) >= size) { /*expanding to adjacent blocks*/
            myBlock->next = myBlock->next->next;
            myBlock->dataSize = myBlock->next->next - myBlock->data;
            returnPtr = myBlock->data;
         }
         else { /*end of linked list, expand unimpeded*/
            myBlock.dataSize = size + (size % ALIGNMENT_SIZE);
            returnPtr = myBlock->data;
         }
      } 
   }

   return returnPtr; 
}
