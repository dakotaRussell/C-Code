/* Header file for Project 2, Moonlander
 */
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

