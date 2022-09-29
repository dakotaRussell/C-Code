#define GRAVITY 9.8 /* meters per second squared */
#define POUNDS_TO_KG 0.4536 /*conversion factor*/
#define KG_PER_TOMATO 0.1 /*weight in kg*/
#define KG_PER_PIE 1.0 /*weight in kg*/
#define KG_PER_ROCK 3.0 /*weight in kg*/

/*Prototypes*/

double getWeight(char objectChoice);
double getVelObject(int distance);
double getVelSkater(int skaterWeight, double objectWeight, double objectVel);