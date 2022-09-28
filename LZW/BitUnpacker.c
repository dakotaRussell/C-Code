/*
 * Copyright Software Innovations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UINT_SIZE 32
#define UINT_MASK 0xFFFFFFFF

typedef unsigned int UInt;

typedef struct {
   UInt curData;
   UInt nextData;
   int bitsLeft;
   int validNext;
} BitUnpacker;


void BuInit(BitUnpacker *bu) {
   bu->curData = 0;          
   bu->nextData = 0;        
   bu->bitsLeft = 0;       
   bu->validNext = 0;     
}                              

void BuTakeData(BitUnpacker *bu, UInt val) {
   bu->nextData = val;                     
   bu->validNext = 1;                     
}                                            

int BuUnpack(BitUnpacker *bu, int size, UInt *data) {           
   int num = 0;
                                                
   if (bu->bitsLeft == 0) {                                   
      if (bu->validNext) {                                   
         bu->curData = bu->nextData;                        
         bu->validNext = 0;                                
         bu->bitsLeft = UINT_SIZE;                        
      }                                                     
   }                                                        
                                                            
   if (bu->bitsLeft >= size) {                           
      if (size == UINT_SIZE)                            
         *data++ = bu->curData;                        
      else                                            
         *data++ = (bu->curData >> (bu->bitsLeft - size))
          & (~(UINT_MASK << size));                         
      bu->bitsLeft -= size;                                     
      num = 1;                                                 
   }                                                        
   else {                                                     
      *data = bu->curData & ((1 << bu->bitsLeft) - 1);       
      if (bu->validNext) {                                  
         bu->curData = bu->nextData;                               
         bu->validNext = 0;                                       
         *data = *data << (size - bu->bitsLeft) |                
          bu->curData >> (UINT_SIZE - size + bu->bitsLeft); 
         bu->bitsLeft = UINT_SIZE - (size - bu->bitsLeft);         
         num = 1;                                                 
      }                                                     
   }                                                        
                                                            
   return num;                                                   
}                                                           

