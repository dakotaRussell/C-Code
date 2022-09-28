#include <stdio.h>
#include "CodeSet.h"
#include "MyLib.h"
#include "LZWExp.h"
#include "SmartAlloc.h"

void PrintOutput(void *unknown, unsigned char *data, int numBytes) {
   while (numBytes--) {
      printf("%c", *data++);
   }
}

int main(int argc, char **argv) {
   LZWExp myLZWExp;
   int recycleCode = DEFAULT_RECYCLE_CODE;
   UInt data;
   int status = 0;

   if (argc > 1)
      recycleCode = *argv[2];

   LZWExpInit(&myLZWExp, &PrintOutput, NULL, recycleCode);   
   
   while (scanf("%X", &data) != EOF && status == 0) {
      status = LZWExpDecode(&myLZWExp, data);
   }
  
   if (status == BAD_CODE)
      printf("Bad code\n");
   else
      status = LZWExpStop(&myLZWExp);

   if (status == MISSING_EOD)
      printf("Missing EOD\n");

   LZWExpDestruct(&myLZWExp);
  
   if (report_space())
      printf("%lu\n", report_space()); 
   return 0;   
}
