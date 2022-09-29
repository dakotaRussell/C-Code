/* Project 1
*
* Name: Dakota Dehn
* Instructor: J. Workman
* Section: 01
*/

#include <stdio.h>
#include <math.h>
#include "skaterUtil.h"


int main()
{
	/*Declaring Variables*/
	double objectWeight, objectVel, skaterVel;
	char objectChoice;
	int skaterWeight, distance;

	/* Prompt for Input */
	printf("How much do you weigh (pounds)? ");
	scanf("%d", &skaterWeight);
	printf("How far away is your professor (meters)? ");
	scanf("%d", &distance);
	printf("Will you throw a rotten (t)omato, banana cream (p)ie, (r)ock, or (o)ther? ");
	scanf(" %c", &objectChoice);

	objectWeight = getWeight(objectChoice);
	objectVel = getVelObject(distance);
	skaterVel = getVelSkater(skaterWeight, objectWeight, objectVel);

	/*Output*/
	printf("\nNice throw!  ");


	if (objectWeight <= 0.1)
	{
		printf("You're going to get an F!");
	}

	else if(objectWeight > 0.1 && objectWeight <= 1.0)
	{
		printf("Make sure your professor is OK.");
	}

	else
	{
		if (distance < 20)
		{
			printf("How far away is the hospital?");
		}

		else
		{
			printf("RIP professor.");
		}
	}

	printf("\nVelocity of skater: %.3f m/s\n", skaterVel);

	if (skaterVel < 0.2)
	{
		printf("My grandmother skates faster than you!\n");
	}

	else if (skaterVel >= 1.0)
	{
		printf("Look out for that railing!!!\n");
	}

	return 0;
}

