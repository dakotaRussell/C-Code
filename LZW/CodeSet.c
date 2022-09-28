#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "SmartAlloc.h"
#include "CodeSet.h"                                                    
                                                                      
/* One entry in a dictionary of codes.                                
 *  * |data| points to the sequence of bytes the code represents.        
 *   * |size| is the sequence length.                                     
 *    */                                                                   
                                                                      
typedef struct CodeEntry {                                            
   unsigned char *fullCode;                                           
   unsigned short size;                                               
   unsigned char suffix;                                              
   int getCodeCount;                                                  
   struct CodeEntry *prefix;                                          
} CodeEntry;                                                          
                                                                      
typedef struct CodeSet {                                              
   int length;                                                        
   int ndx;                                                           
   CodeEntry *array;                                                  
} CodeSet;                                                            
                                                                      
/* Allocate, initialize, and return a CodeSet object, via void *      
 *  * The CodeSet will have room for |numCodes| codes, though it will    
 *   * initially be empty. */                                             
                                                                      
void *CreateCodeSet(int numCodes) {                                          
   CodeSet *codeSet = calloc(1, sizeof(CodeSet));                            
  
   codeSet->length = numCodes;                                               
   codeSet->array = (CodeEntry *)calloc(numCodes, sizeof(CodeEntry));        
   return codeSet;                                                           
}                                                                     
                                                                      
/* Add a new 1-byte code to |codeSet|, returning its index, with      
 *  * the first added code having index 0.  The new code's byte is       
 *   * equal to |val|.  Assume (and assert if needed) that there          
 *    * is room in the |codeSet| for a new code. */                        
                                                                      
int NewCode(void *codeSet, char val) {                                      
   CodeSet *myCodeSet = (CodeSet *)codeSet;                                  
                                                                      
   (myCodeSet->array + myCodeSet->ndx)->suffix = (unsigned char)val;        
   (myCodeSet->array + myCodeSet->ndx)->size = 1;                            
   return (myCodeSet->ndx)++;                                               
}                                                                     
                                                                      
/* Create a new code by copying the existing code at index            
 *  * |oldCode| and extending it by one zero-valued byte.  Any           
 *   * existing code might be extended, not just the most recently        
 *    * added one. Return the new code's index.  Assume |oldCode|          
 *     * is a valid index and that there is enough room for a new code. */  
                                                                      
int ExtendCode(void *codeSet, int oldCode) {                               
   CodeSet *myCodeSet = (CodeSet *)codeSet;                               
                                                                      
   (myCodeSet->array + myCodeSet->ndx)->prefix =                    
    myCodeSet->array + oldCode;                                       
   (myCodeSet->array + myCodeSet->ndx)->size =                       
    (myCodeSet->array + oldCode)->size + 1;                           
                                                                      
   return (myCodeSet->ndx)++;                                                
}                                                                     
                                                                      
/* Set the final byte of the code at index |code| to |suffix|.        
 *  * This is used to override the zero-byte added by ExtendCode.        
 *   * If the code in question has been returned by a GetCode call,       
 *    * and not yet freed via FreeCode, then the changed final byte        
 *     * will also show in the Code data that was returned from GetCode.*/  
                                                                      
void SetSuffix(void *codeSet, int code, char suffix) {                      
   CodeSet *myCodeSet = (CodeSet *)codeSet;                          
   
   CodeEntry *temp = myCodeSet->array + code;                       
  
   temp->suffix = (unsigned char)suffix;                           
   if (temp->getCodeCount)                                        
      *(temp->fullCode + temp->size - 1) = suffix;
}                                                                     
                                                                      
/* Return the code at index |code| */                                 
Code GetCode(void *codeSet, int code) {                         
   CodeSet *myCodeSet = (CodeSet *)codeSet;                    
   CodeEntry *current = myCodeSet->array + code;              
   CodeEntry *temp = current->prefix;                        
   Code entry;                                                        
   unsigned char *ptr = NULL;
   int size = entry.size = current->size;
   
   if (++(current->getCodeCount) == 1) {                   
      current->fullCode = calloc(size, sizeof(unsigned char));     
      ptr = current->fullCode + size - 1;          
                                                                      
      memset(ptr--, current->suffix, 1);                         
      while (temp && temp->getCodeCount == 0) {                 
         *ptr-- = temp->suffix;                              
         temp = temp->prefix;                               
      }                                                               
                                                                      
      if (++ptr != current->fullCode)                               
         memcpy(current->fullCode, temp->fullCode, temp->size);    
   }                                                                  
   entry.data = current->fullCode;                                
                                                                      
   return entry;                                                 
}                                                                     
                                                                      
/* NOT USED FOR THIS IMPLEMENTATION                                   
 * * Mark the code at index |code| as no longer needed, until a new     
 * * GetCode call is made for that code.                                
 * */                                                                   
void FreeCode(void *codeSet, int code) {                        
   CodeSet *myCodeSet = (CodeSet *)codeSet;                    
   CodeEntry *temp = myCodeSet->array + code;                 
  
   if (--(temp->getCodeCount) == 0) {                        
      free(temp->fullCode);                                 
   }                                                                  
}

/* Free all dynamic storage associated with |codeSet| */              
void DestroyCodeSet(void *codeSet) {
   CodeSet *myCodeSet = (CodeSet *)codeSet;                           
   CodeEntry *temp = myCodeSet->array;

   while (temp < myCodeSet->ndx + myCodeSet->array) {
      if (temp->getCodeCount) 
         free(temp->fullCode);
      temp++;
   }

   free(myCodeSet->array);   
   free(myCodeSet);
}     
