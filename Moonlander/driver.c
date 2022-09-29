/**
 * Moonlander function driver program.
 *
 * The purpose of this program is to test the functions in landerFuncs.c.
 * I have partially implemented some test cases for you.  Implement many
 * more test cases!  (And flesh out the test cases that I have given you.)
 */

#include "landerFuncs.h"
#include <stdio.h>

#define GRAVITY 1.62

void testshowWelcome();
void testGetAltitude();
void testGetFuel();
void testdisplayLMState();
void testUpdateVelocity();
void testgetFuelRate();
void testupdateAcceleration();
void testupdateAltitude();
void testupdateFuel();
void testdisplayLMLandingStatus();
int compareDoubles(double d1, double d2, double epsilon);

int main()
{ 
   testshowWelcome();
   testGetAltitude();
   testGetFuel(); 
   testgetFuelRate();
   testdisplayLMState();
   testupdateAcceleration();
   testupdateAltitude();
   testUpdateVelocity();
   testupdateFuel();
   testdisplayLMLandingStatus();
   printf("\n");
   return 0;

}

void testshowWelcome()
{
   printf("*** Testing showWelcome...***\n\n");
   showWelcome();
}

void testGetAltitude()
{
   int altitude;

   printf("*** Testing getAltitude...***\n\n");

   /* Test 1: Enter a valid number */
   printf("TEST 1: When prompted, input a number greater than 0.  You should not see an error.\n\n");
   altitude = getAltitude();
   printf("\nAltitude returned was: %d\n", altitude);

   /* Test 2: First enter 0, then a negative number, then a valid number */
   printf("\nTEST 2: When prompted, input 0.  You should see an error and then be prompted again.\n");
   printf("Then enter a negative number.  You should see the same error message and be \n");
   printf("prompted again.  Finally, enter a number greater than 0.\n\n");
   altitude = getAltitude();
   printf("\nAltitude returned was: %d\n\n", altitude);

}

void testGetFuel()
{
   int fuel;

   printf("*** Testing getFuel... ***\n\n");

   /* Test 1: Enter a valid number */
   printf("TEST 1: When prompted, input a number greater than 0.  You should not see an error.\n\n");
   fuel = getFuel();
   printf("\nFuel returned was: %d\n", fuel);

   /* Test 2: First enter 0, then a negative number, then a valid number */
   printf("\nTEST 2: When prompted, input 0.  You should see an error and then be prompted again.\n");
   printf("Then enter a negative number.  You should see the same error message and be \n");
   printf("prompted again.  Finally, enter a number greater than 0.\n\n");
   fuel = getFuel();
   printf("\nFuel returned was: %d\n\n", fuel);
}

void testgetFuelRate()
{
   int testingfuelRate;
   printf("*** Testing getFuelRate... ***\n\n\n");

   /*Test 1: Enter a number that is not valid. */
   printf("TEST 1: When prompted, input a negative number. You should see an error.\n");
   printf("After the error message, enter a valid number between 0 and 5 inclusively.\n");
   testingfuelRate = getFuelRate(5);
   printf("\nFuel Rate returned: %d\n", testingfuelRate);


   /* Test 2: Enter a number greater than current fuel amount */
   printf("\n\nTEST 2: When prompted, input a valid fuel rate greater than five.\n\n");
   testingfuelRate = getFuelRate(5);
   printf("\n\nExpected result: 5");
   printf("\nActual result: %d\n", testingfuelRate);
   if (testingfuelRate == 5)
   {
      printf("\n     PASS\n");
   }
   else
   {
      printf("\n     FAIL\n");
   }

}

void testdisplayLMState()
{
   printf("\n\n*** Testing displayLMState...***\n\n");
   /*Test 1:*/
   displayLMState(5,6,7,8,9);

   /*Test 2: */
   displayLMState(10,11,12,13,14);

   /*Test 3: */
   displayLMState(1,2,3,4,5);

   /*Test 4*/
   displayLMState(90, 91, 92, 93, 94);
   
   /*Test 5*/
   displayLMState(2345, 9678, 4326, 5213, 4568);
}

void testupdateAcceleration()
{
   double acceleration;
   printf("\n\n*** Testing updateAcceleration...***\n\n");

   /*Test 1: */
   printf("TEST 1: Inputing fuel rate of 0.\n");
   acceleration = updateAcceleration(GRAVITY, 0);
   printf("         Expect: %f\n", -GRAVITY);
   printf("         Got: %f\n", acceleration);
   if (compareDoubles(acceleration, -GRAVITY, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }

   /*Test 2:*/
   printf("TEST 2: Inputing fuel rate of 5.\n");
   acceleration = updateAcceleration(GRAVITY, 5);
   printf("         Expect: %d\n", 0);
   printf("         Got: %f\n", acceleration);
   if (compareDoubles(acceleration, 0, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }

   /*Test 3 */
   printf("TEST 3: Inputing fuel rate of 9.\n");
   acceleration = updateAcceleration(GRAVITY, 9);
   printf("         Expect: %f\n", 1.296);
   printf("         Got: %f\n", acceleration);
   if (compareDoubles(acceleration, 1.296, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }
}

void testupdateAltitude()
{
   double altitude;

   printf("*** Testing updateAltitude...***\n\n");

   /*Test1: Inputing a altitude that should equal 0 */
   printf("TEST 1: Inputing an altitude of zero\n");
   altitude = updateAltitude(0, 0, 0);
   printf("         Expect: %d\n", 0);
   printf("         Got: %f\n", altitude);
   if (compareDoubles(altitude, 0, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }

    /*Test2: Inputing a altitude greater than zero */
   printf("TEST 2: Inputing an altitude bigger than zero. There should be no error.\n");
   altitude = updateAltitude(7, 8, 9);
   printf("         Expect: %f\n", 19.5);
   printf("         Got: %f\n", altitude);
   if (compareDoubles(altitude, 19.5, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }

 /*Test3: Inputing a altitude that will be less than zero */
   printf("TEST 1: Inputing an altitude that will result in a negative value\n");
   altitude = updateAltitude(1, -5, 1);
   printf("         Expect: %d\n", 0);
   printf("         Got: %f\n", altitude);
   if (compareDoubles(altitude, 0, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }


}

void testUpdateVelocity()
{
   double velocity;

   printf("*** Testing updateVelocity... ***\n\n");
   
   /* Test 1: -1.0 and -0.5 */
   printf("TEST 1: Inputing -1 and -0.5\n");
   velocity = updateVelocity(-1.0, -0.5);
   printf("      Expect: %f\n", -1.5);
   printf("      Got: %f\n", velocity);
   /* Compare to see if output velocity and the expected are equal.
    * See the description of the compareDoubles function below. */
   if (compareDoubles(velocity, -1.5, .000001))
   {
      printf("      Pass\n");
   }
   else
   {
      printf("      FAIL\n");
   }

   /* Test 2: 0.0 and 0.0 */
   printf("TEST 2: Inputing 0.0 and 0.0\n");
   velocity = updateVelocity(0.0, 0.0);
   printf("      Expect: %f\n", 0.0);
   printf("      Got: %f\n", velocity);
   if (compareDoubles(velocity, 0.0, .000001))
   {
      printf("      Pass\n");
   }
   else
   {
      printf("      FAIL\n");
   }

   /* Test 3: -100.23 and 1.1 */
   printf("TEST 3: Inputing -100.23 and 1.1\n");
   velocity = updateVelocity(-100.23, 1.1);
   printf("      Expect: %f\n", -99.13);
   printf("      Got: %f\n", velocity);
   if (compareDoubles(velocity, -99.13, .000001))
   {
      printf("      Pass\n");
   }
   else
   {
      printf("      FAIL\n");
   }
}

/* You cannot compare two doubles using == because they are not stored exactly.
 * Use this function to compare doubles to see if they are equivalent to a certain
 * "close enough" epsilon.
 * 
 * For example to compare two doubles (x and y) and see if there are equivalent to
 * 0.000001, you would call the function like this: 
 * compareDoubles(x, y, 0.000001);
 *
 * Return Value: 0 or 1 (a boolean value)
 */
int compareDoubles(double d1, double d2, double epsilon)
{
   return (d1-d2 > -epsilon && d1-d2 < epsilon);
}

void testupdateFuel()
{
   int updatefuel;
   printf("***Testing update Fuel...***\n");

   printf("Test1: When fuel and fuel Rate are both positive.");
   updatefuel = updateFuel(9, 5);
   printf("         Expect: %d\n", 4);
   printf("         Got: %d\n", updatefuel);
   if (compareDoubles(updatefuel, 4, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }


   printf("Test2: When fuel and fuel Rate are both positive, but fuel rate is greater than fuel amount.");
   updatefuel = updateFuel(5, 9);
   printf("         Expect: %d\n", -4);
   printf("         Got: %d\n", updatefuel);
   if (compareDoubles(updatefuel, -4, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }


   printf("Test3: When fuel and fuel Rate are both zero.");
   updatefuel = updateFuel(0, 0);
   printf("         Expect: %d\n", 0);
   printf("         Got: %d\n", updatefuel);
   if (compareDoubles(updatefuel, 0, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }


   printf("Test4: When fuel and fuel Rate are both negative and result with negative answer.");
   updatefuel = updateFuel(-9, -5);
   printf("         Expect: %d\n", -4);
   printf("         Got: %d\n", updatefuel);
   if (compareDoubles(updatefuel, -4, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }


   printf("Test5: When fuel and fuel Rate are both negative and return a positive answer.");
   updatefuel = updateFuel(-5, -9);
   printf("         Expect: %d\n", 4);
   printf("         Got: %d\n", updatefuel);
   if (compareDoubles(updatefuel, 4, .000001))
   {
      printf("         PASS\n");
   }
   else
   {
      printf("         FAIL\n");
   }
}   

void testdisplayLMLandingStatus()
{
   printf("\n**** Testing displayLMLandingStatus...***\n\n");

   /*Test 1*/
   printf("Test1: If velocity is between 0 and -1 inclusively.\n");
   displayLMLandingStatus(0.0);
   displayLMLandingStatus(-0.5);
   displayLMLandingStatus(-1.0);

   printf("Test2: If velocity is between -1 and -10 exclusively.\n");
   displayLMLandingStatus(-5.0);

   printf("Test3: If velocity is less than or equal to -10.\n");
   displayLMLandingStatus(-10);
   displayLMLandingStatus(-15);
}

