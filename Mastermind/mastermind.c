/* MASTERMIND
 *
 * Author: Dakota Dehn 
   Project 4
   Instructor: Julie 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#define MAX_DIM 6
#define MIN_DIM 3
#define MAX_GUESSES 12
#define MAXCHAR 'F'

typedef struct
{
   char letters[MAX_DIM+1]; /* +1 for the null char */
   int exact;
   int inexact;
} Guess;

/* This function already implemented.
 * Compile with "gcc -Wall -ansi -pedantic mastermind.c util.o" to use it.
 * Otherwise, implement the function yourself and just compile with "gcc -Wall -ansi -pedantic  mastermind.c"
 */
int calcMatches(char answer[], int dim, Guess guess[], int index);

/* Prototypes for functions you must implement */
char getRandChar(char maxChar);
void getGuess(char letters[], int dim, char maxChar);
void printBoard(Guess guesses[], int tries, int dim);

int main(void)
{
   /* TODO - declare variables here */
   char maxChoice, answer= 'Y', answerArray[MAX_DIM+1];
   int seed, dim=0, guessesAllowed, i=1, totalGames=0, totalTries=0, gameWon=0, index;
   Guess guesses[MAX_GUESSES];

   /* TODO - get the max letter */
   printf("Enter max letter: ");
   scanf(" %c", &maxChoice);
   while (maxChoice > MAXCHAR)
   {
      printf("Max letter must be between A and F.\n");
      printf("Enter max letter: ");
      scanf(" %c", &maxChoice);
   }

   /* TODO - get the dimension */
   printf("Enter game dimension: ");
   scanf("%d", &dim);
   while(dim < MIN_DIM || dim > MAX_DIM)
   {
      printf("Dimension must be between 3 and 6.\n");
      printf("Enter game dimension: ");
      scanf("%d", &dim);
   }

   
   /* TODO - get the seed and set up the random number generator */
   printf("Enter the seed: ");
   scanf("%d", &seed);
   srand(seed);



   /* TODO - determine maximum guesses based on max letter and dimension */
   guessesAllowed = ceil((double)dim*(maxChoice -'A'+1)/3);
   
   /* TODO - main game loop here */
   while(answer == 'Y' || answer == 'y')
   {
      for (index=0; index < dim; index++)
      {
         answerArray[index] = getRandChar(maxChoice);
      }

      answerArray[index+1]= '\0';
      printf("\nStarting game...");
      totalGames++;
      while (guessesAllowed > 0)
      {  
         if (guessesAllowed == 1)
         {
            printf("\nLAST GUESS!  Guess wisely!\n");
         }
         else
         {
            printf("\nYou have %d guesses left!\n", guessesAllowed);
         }   
         getGuess(guesses[i-1].letters, dim, maxChoice); 
         totalTries++;     
         if (calcMatches(answerArray, dim, guesses, i-1))
         {
            guessesAllowed = 0;
            gameWon = 1;
            printBoard(guesses,i,dim);
            printf("\nYou win!!\n");
            printf("Pattern found in %d attempts!\nCurrent average:  %.3f\n", i, ((double)totalTries/totalGames));
            printf("\nAnother game [Y/N]? ");
            scanf(" %c", &answer);
         }
         else
         {
            printBoard(guesses, i, dim);
         }
         i++;
         guessesAllowed--;   
      }

      if (guessesAllowed == 0 && gameWon==0)
      {
         printf("\nYou lose!!\n");
         printf("Current average:  %.3f\n", ((double)totalTries/totalGames));
         printf("\nAnother game [Y/N]? ");
         scanf(" %c", &answer);
      }

      i=1;
      gameWon=0;
      guessesAllowed = ceil((double)dim*(maxChoice -'A'+1)/3);
   } 
	
   return 0;
}

/* TODO - Implement this function.
 * Use the rand() function to return a random character
 *    inbetween 'A' and the maxChar.
 */
char getRandChar(char maxChar)
{
   return (rand() % (maxChar-'A'+1)) + 'A';  /* CHANGE THIS */
}

/* TODO - Implement this function.
 * Prompt the user for characters to fill in the letters array.
 * Make sure all input characters are between 'A' and maxChar.
 * "dim" indicates how many letters to get.
 */
void getGuess(char letters[], int dim, char maxChar)
{
   int correct, i;
   printf("Enter Guess (%d chars): ", dim);
   
   do
   {
      correct = 1;
      scanf(" %s", letters);
      for (i=0; i<dim; i++)
      {
         if (letters[i] > maxChar || letters[i] < 'A')
         {
            i=dim; 
            printf("One or more chars out of range A-%c, try again: ", maxChar);
            correct = 0;
         }
      }
   } while (correct == 0);
}
 
/* TODO - Implement this function.
 * Display the mastermind board according to the following example:
 *
 *       XXXX
 *       ----
 * (0,2) ABCD
 * (0,3) ABBD
 * (3,0) BEDE
 *
 * "tries" indicates how many guessess are in the Guess array.
 * "dim" indicates how many letters are in each Guess.
 */
void printBoard(Guess guesses[], int tries, int dim)
{
   int i;
   printf("\n");
   for (i=0; i<dim; i++)
   {
      if (i==0)
      {
         printf("      ");
      }
      printf("X");
   }
   printf("\n");
   for (i=0; i<dim; i++)
   {
      if (i==0)
      {
         printf("      ");
      }
      printf("-");
   }
   printf("\n");
   
   for (i=0; i<tries; i++)
   {
      printf("(%d,%d) %s\n", guesses[i].exact, guesses[i].inexact, guesses[i].letters);
   }
}

int calcMatches(char answer[], int dim, Guess guess[], int index)
{
   int i, j;
   char dumAnswer[7], dumGuess[7];
   strcpy(dumAnswer, answer);
   strcpy(dumGuess, guess[index].letters);
   guess[index].exact = 0;
   guess[index].inexact = 0;
   /*looking for exact*/
   for (i=0; i<dim; i++)
   {
      if (dumAnswer[i] == dumGuess[i])
      {
         dumAnswer[i] = 'X';
         dumGuess[i] = 'Y';
         (guess[index].exact)++;
      }
      
   }

   if (guess[index].exact == dim)
   {
      return 1;
   }

   for (i=0; i<dim; i++)
   {
      for(j=0; j<dim; j++)
      {
         if (dumAnswer[i] == dumGuess[j])
         {
            dumAnswer[i] = 'X';
            dumGuess[j] = 'Y'; 
            (guess[index].inexact)++;
         }
      }
   }

   return 0;

}





