/*Dakota Dehn and Ansley Chen*/

#include <stdio.h>
#include "sudokusolver.h"

int main()
{
	int puzzle[9][9], i, j, scount, amtChange=0;
	int attempts=0, backTracks=0;

	/*user inputting the puzzle*/
	readSudoku(puzzle); 

	/*printing input puzzle*/
	printf("\n\nInput puzzle:\n\n");
	for (i=0; i<9; i++)
	{
		for (j=0; j<9; j++)
		{
			printf("%d", puzzle[i][j]);
		}
		printf("\n");
	}

	/*trying single candidate*/
	do
	{
		scount=0;
		for (i=0; i<9; i++)/*rows*/
		{
			for (j=0; j<9; j++)/*columns*/
			{
				if (puzzle[i][j] == 0)
				{
					if (singleCandidate(puzzle, i, j) == 1)
					{
						amtChange++;
						scount++;
					}
					
				} 
			}
		}
	} while (scount > 0 && !isSolved(puzzle));

	if (scount== 0)
	{
		bruteForce(puzzle, &attempts, &backTracks);
	}

	printf("\n");
	if (!isSolved(puzzle))
	{
		printf("Puzzle is not solvable.\n");
	}

	else if(scount>0)
	{
		printf("Solved by single candidate technique.\n");
	}
	
	else if(backTracks == 0) /*(!isSolved(puzzle))*/
	{
		printf("Solved by brute force with no backtracking.\n");
	}

	else if(backTracks != 0)
	{
		printf("Solved by brute force with backtracking.\n");
	}
	/*printing solved puzzle*/
	
	

	printf("Single candidates found: %d\n", amtChange);
	printf("Failed attempts: %d\n", attempts);
	printf("Backtracks: %d\n", backTracks);
	if(isSolved(puzzle)==1)
	{
		printf("\nSolution:\n\n");
	printSudoku(puzzle);
	}
	

	/*success or failure*/
	


	

	return 0;
}