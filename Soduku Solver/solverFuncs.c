/*Dakota Dehn & Ansley Chen*/
/* Project 3 Part 1 */

#include <stdio.h>
#ifndef SOLVER_H
#define SOLVER_H

void readSudoku(int puzzle[][9]);
int checkValid (int puzzle[][9], int row, int col, int num);
void printSudoku(int puzzle[][9]);
int singleCandidate(int puzzle[][9], int row, int col);
void bruteForce(int puzzle[][9], int *attempts, int *backTracks);
int isSolved(int puzzle[][9]);

#endif

void readSudoku(int puzzle[][9])
{
	int i, j;
	for (i=0; i < 9; i++)
	{
		printf("Enter line %d: ", i+1);
		for (j=0; j < 9; j++)
		{
			scanf("%1d", &puzzle[i][j]);
		}
	}
	
}

int checkValid(int puzzle[][9], int row, int col, int num)

{
	int r, c, i, j, startVBox=0, startHBox=0;
	startHBox = (row/3)*3;
	startVBox = (col/3)*3;

	if (puzzle[row][col] != 0)
	{
		return -1;
	}

	/*test a row*/
	for (c=0; c<9; c++)
	{
		if (puzzle[row][c] == num)
		{
			return 0;
		}
	}

	/*test a column*/
	for (r=0; r<9; r++)
	{
		if (puzzle[r][col] == num)
		{
			return 0;
		}
	}

	/*test box*/
	for (i= startHBox; i< startHBox + 3; i++)
	{
		for (j = startVBox; j< startVBox + 3; j++)
		{
			if (puzzle[i][j] == num)
			{
				return 0;
			}
		}
	}

	return 1;
	

}

void printSudoku(int puzzle[][9])
{
	int i, j;
	for (i=0; i<9; i++)
	{

		for(j=0; j<9; j++)
		{	
			
			if (j == 3 || j== 6)
			{
				printf(" | ");
			}
			

			printf("%d", puzzle[i][j]);
		}
		if (i == 2 || i== 5)
			{
				printf("\n---------------");
			}

		printf("\n");

	}
}


int singleCandidate(int puzzle[][9], int row, int col)
{
	int i, count=0, location = 0;
  	/*try numbers 1-9*/
  	for(i=1; i<10; i++)
  	{
		if(checkValid(puzzle, row, col, i) == 1)
     	{
        	count++;
			location = i;
     	}
 	}

	if(count == 1)
		{
			puzzle[row][col] = location;
			return 1;
  		}

	return 0;	
	
}

void bruteForce(int puzzle[][9], int *attempts, int *backTracks)
{
	int i, j, num, count, dummy[9][9]; 
	for(i=0; i<9; i++)
	{
		/*columns*/
		for(j=0; j<9; j++)
		{
			dummy[i][j] = puzzle[i][j];
		}
	}

	/*rows*/
	for(i=0; i<9; i++)
	{
		/*columns*/
		for(j=0; j<9; j++)
		{
			if (dummy[i][j] == 0)
			{
				count = 0;
				num=puzzle[i][j]+1;
				for(puzzle[i][j]=0; num<10; num++)
				{	
					if (checkValid(puzzle, i, j, num) == 1)
					{
						count++; 
						if (count == 1)
						{
							puzzle[i][j] = num;
							num = 10;
						}
					}
					else
					{
						(*attempts)++;
					}
				}

				if (count == 0)
				{
					(*backTracks)++;
					do
					{
						j--;
						if (j<0)
						{
							i--; 
							j=8;
						}
						if (i<0)
						{
							return;
						}
					} while (dummy[i][j] != 0);

					j--;
					if (j<0)
					{
						i--;
						j=8;
					}			
				}
			}
		}
	}
}		


int isSolved(int puzzle[][9])
{
	int i, j;
	for(i=0; i<9; i++)/*rows*/
	{
		for(j=0; j<9; j++)/*columns*/
		{
			if(puzzle[i][j] == 0)
			{
				return 0;
			}
		}
	}

	return 1; 
}





	