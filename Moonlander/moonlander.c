/* Project 2 
* 
* Name: Dakota Dehn
* Instructor: Julie Workman 
*/

#include <stdio.h>
#include "landerFuncs.h"

#define GRAVITY 1.62

void showWelcome(void);
int getFuel(void);
int getAltitude(void);
void displayLMState(int time, double altitude, double velocity, int fuel, int fuelRate);
int getFuelRate(int fuel);
double updateAcceleration(double gravity, int fuelRate);
double updateAltitude(double altitude, double velocity, double acceleration);
double updateVelocity(double velocity, double acceleration);
int updateFuel(int fuel, int rate);
void displayLMLandingStatus(double velocity);


int main()
{
	int fuel, fuelRate = 0, elapsedTime = 0;
	double altitude, velocity = 0, acceleration;
	showWelcome();
	altitude = getAltitude();
	fuel = getFuel();
	printf("\nLM state at retrorocket cutoff\n");
   
	do
	{	
	   if (fuel > 0)
      {
         displayLMState(elapsedTime, altitude, velocity, fuel, fuelRate);
		   printf("\n");
		   fuelRate = getFuelRate(fuel);
      }
      else
      {
         printf("OUT OF FUEL - Elapsed Time:%4d Altitude: %7.2f Velocity: %7.2f\n", elapsedTime, altitude, velocity);
         fuelRate = 0;
      }
		elapsedTime++;
		acceleration = updateAcceleration(GRAVITY, fuelRate);
		altitude = updateAltitude(altitude, velocity, acceleration);
		velocity = updateVelocity(velocity, acceleration);
		fuel = updateFuel(fuel, fuelRate);
	} while (altitude > 0);

   printf("\nLM state at landing/impact\n");
   displayLMState(elapsedTime, altitude, velocity, fuel, fuelRate);
   printf("\n");
	displayLMLandingStatus(velocity);


	return 0;
}