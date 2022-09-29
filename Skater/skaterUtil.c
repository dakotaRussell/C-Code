#include <stdio.h>
#include <math.h>
#include "skaterUtil.h"

/*Defining Functions*/
double getWeight(char objectDecision)
{
	double weight;

	if (objectDecision == 't')
	{
		return KG_PER_TOMATO;
	}

	else if (objectDecision == 'p')
	{
		return KG_PER_PIE;
	}

	else if (objectDecision == 'r')
	{
		return KG_PER_ROCK;
	}

	else
	{
		printf("Enter the weight of the object in KG: ");
		scanf("%lf", &weight);
	}

	return weight; 
}


double getVelObject(int distance)
{
	return sqrt((GRAVITY*distance)/2);
}

double getVelSkater(int skaterWeight, double objectWeight, double objectVel)
{
	return objectWeight*objectVel/((double)skaterWeight*POUNDS_TO_KG);
}