/***** Axis.hpp *****/
#include <cstdlib>

#define NUM_MOTORS 20

Bela_I2C i2c;

Motor motors[NUM_MOTORS];
int offsets[NUM_MOTORS] = {1300, 7180, 960, -1530, -7980, -6060, -2510, 2890, -4376, 7790, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};

// utility functions
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

int waveCount, wavePosition, waveIndex;
void startWaveMotion(int position){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setStatus(-1);
	}
	motors[0].setPosition(position);
	motors[0].go();
	waveIndex = 0;
	waveCount = 0;
	wavePosition = position;
}

void waveMotion(int interval){
	
	if (waveCount++ >= interval){
		if (waveIndex < NUM_MOTORS-1){
			waveIndex++;
			motors[waveIndex].setPosition(wavePosition);
			motors[waveIndex].go();
			// rt_printf("going: %i\n", waveIndex);
		}
		waveCount = 0;
	}

	for (int i=0; i<NUM_MOTORS; i++){
		if (waveIndex >= i && motors[i].getStatus() == 0){
			// rt_printf("setting %i\n", i);
			motors[i].setPosition(motors[i].lastPosition * -1);
			motors[i].go();
		}
	}
	
}












