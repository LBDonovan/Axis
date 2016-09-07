/***** Axis.hpp *****/
#include <cstdlib>

#define NUM_MOTORS 20

Bela_I2C i2c;

Motor motors[NUM_MOTORS];
int offsets[NUM_MOTORS] = {1300, 7180, 960, -1530, -7980, -6060, -2510, 2890, -4376, 7790, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};

// utility functions
void go(int motor){
	i2c.write(motor+1, {GO_INSTRUCTION});
	motors[motor].setStatus(-1);
}
void goAll(){
	i2c.write(0, {GO_INSTRUCTION});
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setStatus(-1);
	}
}
void stopAll(){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setVelocity(0);
	}
	goAll();
}
bool allIdle(){
	for (int i=0; i<NUM_MOTORS; i++){
		if (motors[i].getStatus()){
			return false;
		}
	}
	return true;
}

// movements
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
void setRatioVelocity(int base, float ratio){
	int pos = base;
	for (int i=0; i<NUM_MOTORS/2; i++){
		motors[i].setVelocity(pos);
		pos *= ratio;
	}
}

int waveCount, wavePosition;
void startWaveMotion(int motor, int position){
	motors[motor].setPosition(position);
	go(motor);
	waveCount = 0;
	wavePosition = position;
}

void waveMotion(int interval){
	if (waveCount++ >= interval){
		for (int i=0; i<NUM_MOTORS; i++){
			if (motors[i].getStatus() != 0 && motors[i].getStatus() != -1){	// motor is not idle
				rt_printf("%i not idle\n", i, waveCount);
				if(i != 0){
					if (motors[i-1].getStatus() == 0){	// motor before is idle
						motors[i-1].setPosition(wavePosition);
						go(i-1);
						rt_printf("motor before (%i) is idle, setting its position to %i\n", i-1, wavePosition);
					}
				}
				if (i != (NUM_MOTORS-1)){
					if (motors[i+1].getStatus() == 0){	// motor after is idle
						motors[i+1].setPosition(wavePosition);
						go(i+1);
						rt_printf("motor after (%i) is idle, setting position %i\n", i+1, wavePosition);
					}
				}
			}
		}
		waveCount = 0;
	}
}












