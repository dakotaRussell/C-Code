#include "Report.h"
#include <stdio.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_CELLS 50
#define MAX_LENGTH 15
#define CASE_THREE 3
#define PARAM_LENGTH 4

typedef struct Pipes {
   int parentPipe[2];
   int pipeOutRight[2];
   int pipeInRight[2];
   int pipeOutLeft[2];
   int pipeInLeft[2];
} Pipes;

typedef struct SetupInfo {
   int leftSet;
   int rightSet;
   int stepSet;
   int cellSet;
   int numCells;
   int stepCount;
   int childCount;
   double leftCell;
   double rightCell;
   int childrenPids[MAX_CELLS]; 
} SetupInfo;

typedef struct CellParams {
   char parentParam[PARAM_LENGTH];
   char idParam[PARAM_LENGTH];
   char stepParam[MAX_LENGTH];
   char fixedParam[MAX_LENGTH];
   char inFd1[PARAM_LENGTH];
   char inFd2[PARAM_LENGTH];
   char outFd1[PARAM_LENGTH];
   char outFd2[PARAM_LENGTH];
} CellParams;


void ParseArgs(char **curArg, SetupInfo *myArgs) {
   while (*curArg) {
      switch (**curArg) {
      case 'S':
         if (!myArgs->stepSet) {
            myArgs->stepCount = atoi(*curArg + 1);
            myArgs->stepSet = 1;
         }
         curArg++;
         break;
      case 'C':
         if (!myArgs->cellSet) {
            myArgs->numCells = atoi(*curArg + 1);
            myArgs->cellSet = 1;
         }
         curArg++;
         break;
      case 'L':
         if (!myArgs->leftSet) {
            myArgs->leftCell = atof(*curArg + 1);
            myArgs->leftSet = 1;
         }
         curArg++;
         break;
      case 'R':
         if (!myArgs->rightSet) {
            myArgs->rightCell = atof(*curArg + 1);
            myArgs->rightSet = 1;
         }
         curArg++;
         break;
      default:
         curArg++; 
      }
   }
   
   if (((myArgs->numCells < 2) && myArgs->rightSet && myArgs->leftSet) || 
    (myArgs->stepCount < 1) || (myArgs->numCells < 1) || 
    (myArgs->numCells > MAX_CELLS) || (myArgs->numCells == 1 && 
    myArgs->rightSet)) {
      fprintf(stderr, "Usage: LinearSim C S L R (in any order)\n");
      exit(EXIT_FAILURE);
   }
}

void ExecCommon(SetupInfo *myArgs, Pipes *pipes, int iterator,
 CellParams *params) {
   sprintf(params->idParam, "D%d", iterator);
   sprintf(params->stepParam, "S%d", myArgs->stepCount);
   sprintf(params->parentParam, "O%d", pipes->parentPipe[1]);
}

void ExecLeftCell(SetupInfo *myArgs, Pipes *pipes, int iterator,
 CellParams *params) {
   ExecCommon(myArgs, pipes, iterator, params);
   if (myArgs->leftSet)
      sprintf(params->fixedParam, "V%lf", myArgs->leftCell);
   
   if (myArgs->numCells == 1 || myArgs->numCells == 2) {
      if (myArgs->leftSet) {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->fixedParam, params->parentParam, NULL);
      }
      else {
         execl("./Cell", "./Cell", params->idParam, params->stepParam, 
          params->parentParam, NULL);
      }
   }
   else {
      sprintf(params->outFd1, "O%d", pipes->pipeOutRight[1]);
      if (myArgs->leftSet) {
         execl("./Cell", "./Cell", params->idParam, params->stepParam, 
          params->fixedParam, params->parentParam, params->outFd1, NULL);
      }
      else {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, params->outFd1, NULL);
      }
   }
      
}

void ExecRightCell(SetupInfo *myArgs, Pipes *pipes, int iterator,
 CellParams *params) {
   ExecCommon(myArgs, pipes, iterator, params);
   if (myArgs->rightSet)
      sprintf(params->fixedParam, "V%lf", myArgs->rightCell);
   
   if (myArgs->numCells == 2) {
      if (myArgs->rightSet) {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->fixedParam, params->parentParam, NULL);
      }
      else {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, NULL);
      }
   }
   else {
      sprintf(params->outFd1, "O%d", pipes->pipeOutLeft[1]);
      if (myArgs->rightSet) {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, params->fixedParam, params->outFd1, NULL);
      }
      else {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, params->outFd1, NULL);
      }
   }
}

void ExecMiddleCell(SetupInfo *myArgs, Pipes *pipes, int iterator, 
 CellParams *params) {
   ExecCommon(myArgs, pipes, iterator, params);
   sprintf(params->inFd1, "I%d", pipes->pipeInLeft[0]);
   sprintf(params->inFd2, "I%d", pipes->pipeInRight[0]);
   sprintf(params->outFd1, "O%d", pipes->pipeOutLeft[1]);
   sprintf(params->outFd2, "O%d", pipes->pipeOutRight[1]);

   if (iterator == 1) {
      if (myArgs->numCells == CASE_THREE) {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, params->inFd1, params->inFd2, NULL);
      }
      else {
         execl("./Cell", "./Cell", params->idParam, params->stepParam,
          params->parentParam, params->inFd1, params->inFd2, params->outFd2, 
          NULL);
      }
   }
   else if (iterator == (myArgs->numCells - 2)) {
      execl("./Cell", "./Cell", params->idParam, params->stepParam,
       params->parentParam, params->inFd1, params->inFd2, params->outFd1, NULL);
   }
   else {
      execl("./Cell", "./Cell", params->idParam, params->stepParam,
       params->parentParam, params->inFd1, params->inFd2, params->outFd1, 
       params->outFd2, NULL);
   }
}

void LeftCellSetup(SetupInfo *myArgs, Pipes *pipes, int iterator,
 CellParams *params) {
   if (!(myArgs->childrenPids[iterator] = fork())) {
      close(pipes->parentPipe[0]);
      close(pipes->pipeOutRight[0]);
      ExecLeftCell(myArgs, pipes, iterator, params);
   }
   else {
      myArgs->childCount++;
      close(pipes->pipeOutRight[1]);
      pipes->pipeInLeft[0] = pipes->pipeOutRight[0];
   }
}

void RightCellSetup(SetupInfo *myArgs, Pipes *pipes, int iterator,
 CellParams *params) {
   if (!(myArgs->childrenPids[iterator] = fork())) {
      close(pipes->parentPipe[0]);
      close(pipes->pipeOutRight[0]);
   
      ExecRightCell(myArgs, pipes, iterator, params);
   }

   else {
      myArgs->childCount++;
      close(pipes->pipeOutLeft[1]);
      close(pipes->pipeInLeft[0]);
   }
}

void MiddleCellSetup(SetupInfo *myArgs, Pipes *pipes, int iterator, 
 CellParams *params) {
   pipe(pipes->pipeOutRight);
   pipe(pipes->pipeInRight);
   
   if (!(myArgs->childrenPids[iterator] = fork())) {
      close(pipes->parentPipe[0]);
      close(pipes->pipeInRight[1]);
      close(pipes->pipeOutRight[0]);
   
      if (iterator == (myArgs->numCells - 2))
         close(pipes->pipeOutRight[1]);
   
      ExecMiddleCell(myArgs, pipes, iterator, params);
   }
   else {
      myArgs->childCount++;
      close(pipes->pipeInRight[0]);
      close(pipes->pipeOutRight[1]);
   
      if (iterator != 1)
         close(pipes->pipeOutLeft[1]);
   
      close(pipes->pipeInLeft[0]);
      pipes->pipeOutLeft[1] = pipes->pipeInRight[1];
      pipes->pipeInLeft[0] = pipes->pipeOutRight[0];
   }
}

void CellSetup(SetupInfo *myArgs, Pipes *pipes, CellParams *params) {
   int iterator = 0;

   pipe(pipes->parentPipe);

   for (; iterator < myArgs->numCells; iterator++) {
      if (myArgs->numCells > 2) {
         if (iterator == 0) {
            pipe(pipes->pipeOutRight);
            LeftCellSetup(myArgs, pipes, iterator, params);
         }
         else if (iterator == (myArgs->numCells - 1)) {
            RightCellSetup(myArgs, pipes, iterator, params);
         }
         else {
            MiddleCellSetup(myArgs, pipes, iterator, params);
         }
      }
      else {
         if (!(myArgs->childrenPids[iterator] = fork())) {
            close(pipes->parentPipe[0]);
            if (iterator  == 0)
               ExecLeftCell(myArgs, pipes, iterator, params);
            else
               ExecRightCell(myArgs, pipes, iterator, params);
         }
         else
            myArgs->childCount++;
      }
   }
}

void ReadParentPipe(SetupInfo *myArgs, Pipes *pipes, int *ret) {
   int cellArray[myArgs->numCells];
   int ndx = 0, tooLow = 0, tooHigh = 0;
   int bytesRead = 0;
   Report curReport = {0, 0, 0.0};

   for (; ndx < myArgs->numCells; ndx++)
      cellArray[ndx] = 0;

   bytesRead = read(pipes->parentPipe[0], &curReport, sizeof(Report));
   while (bytesRead != 0) {
      if (bytesRead == sizeof(Report))
         cellArray[curReport.id]++;
      fprintf(stdout, "Result from %d, step %d: %.3f\n", curReport.id, 
       curReport.step, curReport.value);
      bytesRead = read(pipes->parentPipe[0], &curReport, sizeof(Report));
   }

   for (ndx = 0; ndx < myArgs->numCells; ndx++) {
      if (cellArray[ndx] < (myArgs->stepCount + 1)) {
         tooLow++;
      }
      if (cellArray[ndx] > (myArgs->stepCount + 1)) {
         tooHigh++;
      }
   }

   if (tooLow) {
      fprintf(stderr, "Error: %d cells reported too few reports\n", tooLow);
      *ret = 1;
   }

   if (tooHigh) {
      fprintf(stderr, "Error: %d cells reported too many reports\n", tooHigh);
      *ret = 1;
   }
}

int GetID(int pid, SetupInfo *myArgs) {
   int ndx = 0;

   for (; ndx < myArgs->numCells; ndx++) {
      if (myArgs->childrenPids[ndx] == pid)
         return ndx;
   }

   return -1; 
}

void WaitForChildren(SetupInfo *myArgs, int *ret) {
   int waitCount = myArgs->childCount;
   int pid = 0, status = 0; 
   
   while (waitCount-- > 0) {
      pid = wait(&status);
      if (status) {
         fprintf(stderr, "Error: Child %d exited with %d\n", GetID(pid,
          myArgs), 1);
         *ret = 1;
      }
   }
}

int main(int argc, char *argv[]) {
   char **curArg = argv;
   SetupInfo myArgs = {0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, {0}};
   Pipes myPipes = { {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };
   CellParams myParams = { { 0 } };
   int ret = 0; 

   ParseArgs(curArg, &myArgs);
   CellSetup(&myArgs, &myPipes, &myParams);
   close(myPipes.parentPipe[1]);
   ReadParentPipe(&myArgs, &myPipes, &ret);
   WaitForChildren(&myArgs, &ret);

   if (ret)
      return EXIT_FAILURE;
   else
      return EXIT_SUCCESS;   
}
