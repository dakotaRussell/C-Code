#include "Report.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FD_LIMIT 12

typedef struct Files {
   int inFdL;
   int inFdR;
   int outFdL;
   int outFdR; 
   int parFd;
   int steps; 
   int fixedValue;
} Files;

void StepThrough(Report writeReport, Report readReport, Files myFiles);

int main(int argc, char *argv[]) {
   Files myFiles = {-1, -1, -1, -1, -1, 0, 0};
   char **curArg = argv;
   Report readReport = {0, 0, 0.0};
   Report writeReport = {0, 0, 0.0};

   while (*curArg) {
      switch (**curArg) {
      case 'S':
         myFiles.steps = atoi(*curArg + 1);
         curArg++;
         break;
      case 'I':
         if (myFiles.inFdR < 0) {
            myFiles.inFdR = atoi(*curArg + 1);
            if (myFiles.inFdR > FD_LIMIT)
               exit(EXIT_FAILURE);
         }
         else if (myFiles.inFdL < 0) {
            myFiles.inFdL = atoi(*curArg + 1);
            if (myFiles.inFdL > FD_LIMIT)
               exit(EXIT_FAILURE);
         }
         curArg++;
         break;
      case 'O':
         if (myFiles.parFd < 0) {
            myFiles.parFd = atoi(*curArg + 1);
         }
         else if (myFiles.outFdL < 0) {
            myFiles.outFdL = atoi(*curArg + 1);
            if (myFiles.outFdL > FD_LIMIT)
               exit(EXIT_FAILURE);
         }
         else if (myFiles.outFdR < 0) {
            myFiles.outFdR = atoi(*curArg + 1);
            if (myFiles.outFdR > FD_LIMIT)
               exit(EXIT_FAILURE);
         }
         curArg++;
         break;
      case 'V':
         writeReport.value = atof(*curArg + 1);
         myFiles.fixedValue = 1;
         curArg++;
         break;
      case 'D':
         writeReport.id = atoi(*curArg + 1);
         curArg++;
         break;
      default:
         curArg++; 
      }
   }
   
   StepThrough(writeReport, readReport, myFiles);
   exit(EXIT_SUCCESS);
}

void StepThrough(Report writeReport, Report readReport, Files myFiles) {
   int iterator, bytesRead;
   double total;

   for (iterator = 0; iterator <= myFiles.steps; iterator++) {
      writeReport.step = iterator;
      total = 0.0;
      bytesRead = 0;
      
      if (myFiles.outFdL >= 0)
         write(myFiles.outFdL, &writeReport, sizeof(Report)); 
      if (myFiles.outFdR >= 0)
         write(myFiles.outFdR, &writeReport, sizeof(Report));
      
      if (myFiles.parFd >= 0) 
         write(myFiles.parFd, &writeReport, sizeof(Report)); 
      else
         exit(EXIT_FAILURE);
      
      if (myFiles.inFdL >= 0) {
         bytesRead = read(myFiles.inFdL, &readReport, sizeof(Report));
         if (bytesRead != sizeof(Report))
            exit(EXIT_FAILURE);
         else {
            if (readReport.step != iterator)
               exit(EXIT_FAILURE);
            else
               total += readReport.value;
         }
      }
    
      bytesRead = 0;
      if (myFiles.inFdR >= 0) {
         bytesRead = read(myFiles.inFdR, &readReport, sizeof(Report));
         if (bytesRead != sizeof(Report))
            exit(EXIT_FAILURE);
         else {
            if (readReport.step != iterator)
               exit(EXIT_FAILURE);
            else 
               total += readReport.value;
         }
      }

      total /= 2;
      
      if (!(myFiles.fixedValue))
         writeReport.value = total;
   }
}
