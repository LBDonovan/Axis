/***** Axis.hpp *****/
#include <cstdlib>

#define NUM_MOTORS 20

Bela_I2C i2c;

Motor motors[NUM_MOTORS];

int offsets[NUM_MOTORS] = {1300, 7180, 960, -1530, -7980, -6060, -2510, 2890, -4376, 7790, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};

// utility functions
void goAll(){
	i2c.write(0, {10});
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setStatus(-1);
	}
}
bool allIdle(){
	for (int i=0; i<NUM_MOTORS; i++){
		if (motors[i].getStatus()){
			return false;
		}
	}
	return true;
}

// Utilities
void setRandomPosition(int scale){
	for (int i=0; i<NUM_MOTORS/2; i++){
		int pos = (int)(((float)(rand())/(float)(RAND_MAX))*(float)(scale)) - scale/2;
		motors[i].setPosition(pos);
	}
}
void setMultipleVelocity(int base){
	for (int i=0; i<NUM_MOTORS/2; i++){
		rt_printf("%i, vel: %i\n", i, base*(i+1));
		motors[i].setVelocity(base*(i+1));
	}
}
void setRatioVelocity(int velocity, float ratio){
	for (int i=0; i<NUM_MOTORS/2; i++){
		motors[i].setVelocity(velocity);
		velocity *= ratio;
	}
}

// Set all motors to turn at the same rate, but the plates to turn in alternating directions.
void setConstantVelocity_Alternating(int base) {
	for (int i = 0; i < NUM_MOTORS / 2; i++) {
		if (i % 2 == 0) {
			motors[i].setVelocity(base * -1);
		} else {
			motors[i].setVelocity(base);
		}
	}
} 

// Set all motors to turn at the same rate in the same direction.
void setConstantVelocity(int base) {
	for (int i = 0; i < NUM_MOTORS / 2; i++) {
		motors[i].setVelocity(base);
	}
}

// Set motors 0-9 in one direction, and 10-19 in the other 
void setRatioVelocity_AlternatingHalves(int velocity, float ratio) {
	int initialVelocity = velocity;
	for (int i = 0; i < NUM_MOTORS; i++) {
			motors[i].setVelocity(velocity * -1);
			motors[(NUM_MOTORS - 1) - i].setVelocity(velocity);
			velocity *= ratio;
		} 
}



void setRatioVelocity_AlternatingFromMiddle(int velocity, float ratio) {
	for (int i = 0; i < NUM_MOTORS / 2; i++) {
		int j = NUM_MOTORS - (i + 1);
			velocity *= -1;
			motors[i].setVelocity(velocity);
			motors[j].setVelocity(velocity);
			velocity *= ratio;
		} 
}
