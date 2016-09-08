/***** Axis.hpp *****/
#include <cstdlib>
#include <cmath>

#define NUM_MOTORS 20

Bela_I2C i2c;

#define ONE_TURN 200*32*36/14

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
void zeroAll(){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].zero();
	}
}
void holdStatusAll(int num){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].holdStatus(num);
	}
}
void resetAll(){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].reset();
	}
}

// movements
void setAllPosition(int pos){
	for (int i=0; i<NUM_MOTORS; i++){
		//motors[i].setAcceleration(200*32);
		motors[i].setPosition(0);
	}
}
void setRandomPosition(int min, int max){
	for (int i=0; i<NUM_MOTORS; i++){
		int pos = (int)(((float)(rand())/(float)(RAND_MAX))*(float)(max-min)) + min;
		if (!(i%2)){
			pos *= -1;
		}
		int vel = abs(pos);
		if (vel > 2000){
			motors[i].setMicrostepping(16);
			vel /= 2;
			pos /= 2;
		} else {
			motors[i].setMicrostepping(32);
		}
		rt_printf("%i: %i %i\n", i, pos, vel);
		motors[i].setAcceleration(vel*2);
		motors[i].setMaxVelocity(vel);
		motors[i].setPosition(pos);
	}
}
void setMultipleVelocity(int base){
	for (int i=0; i<NUM_MOTORS/2; i++){
		rt_printf("%i, vel: %i\n", i, base*(i+1));
		motors[i].setVelocity(base*(i+1));
	}
	for (int i=NUM_MOTORS/2; i<NUM_MOTORS; i++){
		rt_printf("%i, vel: %i\n", i, base*(NUM_MOTORS-i));
		motors[i].setVelocity(base*(NUM_MOTORS-i));
	}
}
void setSpiralPosition(int base, int velocity){
	for (int i=0; i<NUM_MOTORS/2; i++){
		motors[i].setMicrostepping(32);
		int pos = base*(i+1);
		motors[i].setAcceleration(velocity);
		motors[i].setMaxVelocity(velocity);
		motors[i].setPosition(pos);
	}
	for (int i=NUM_MOTORS/2; i<NUM_MOTORS; i++){
		int pos = base*(NUM_MOTORS-i);
		//rt_printf("%i, pos: %i\n", i, pos);
		motors[i].setAcceleration(velocity);
		motors[i].setMaxVelocity(velocity);
		motors[i].setPosition(pos);
	}
}
void setMultiplePosition(int base, int velocity){
	for (int i=0; i<NUM_MOTORS/2; i++){
		int vel = velocity*(i+1);
		int pos = base*(i+1);
		if (vel > 2000 && vel < 4000){
			motors[i].setMicrostepping(16);
			vel /= 2;
			pos /= 2;
		} else if (vel >= 4000){
			motors[i].setMicrostepping(8);
			vel /= 4;
			pos /= 4;
		} else {
			motors[i].setMicrostepping(32);
		}
		//rt_printf("%i, pos: %i, vel: %i\n", i, pos, vel);;
		motors[i].setAcceleration(vel);
		motors[i].setMaxVelocity(vel);
		motors[i].setPosition(pos);
	}
	for (int i=NUM_MOTORS/2; i<NUM_MOTORS; i++){
		int vel = velocity*(NUM_MOTORS-i);
		int pos = base*(NUM_MOTORS-i);
		if (vel > 2000){
			motors[i].setMicrostepping(16);
			vel /= 2;
			pos /= 2;
		} else {
			motors[i].setMicrostepping(32);
		}
		//rt_printf("%i, pos: %i\n", i, pos);
		motors[i].setAcceleration(vel);
		motors[i].setMaxVelocity(vel);
		motors[i].setPosition(pos);
	}
}
void setRatioVelocity(int base, float ratio){
	int pos;
	for (int i=0; i<NUM_MOTORS/2; i++){
		pos = base * powf(ratio, i);
		motors[i].setVelocity(pos);
		rt_printf("%i, vel: %i\n", i, pos);
	}
	//pos = base;
	for (int i=NUM_MOTORS/2; i<NUM_MOTORS; i++){
		pos = base * powf(ratio, NUM_MOTORS-1-i);
		motors[i].setVelocity(pos);
		rt_printf("%i, vel: %i\n", i, pos);
	}
}

int waveCount, wavePosition, waveIndex, waveInterations;
bool waveAlternating;
void startWaveMotion(int position, int iterations, bool alternating){
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setMicrostepping(32);
		motors[i].setAcceleration(200*32);
		motors[i].setMaxVelocity(800*32);
		motors[i].setStatus(-1);
		motors[i].waveInterations = 0;
		motors[i].finishedWave = false;
	}
	motors[0].setPosition(position);
	motors[0].go();
	motors[0].holdStatus(5);
	waveIndex = 0;
	waveCount = 0;
	wavePosition = position;
	waveInterations = iterations;
	waveAlternating = alternating;
}

void waveMotion(int interval){
	
	if (waveCount++ >= interval){
		if (waveIndex < NUM_MOTORS-1){
			waveIndex++;
			if (waveAlternating && (waveIndex%2)){
				motors[waveIndex].setPosition(-wavePosition);
			} else {
				motors[waveIndex].setPosition(wavePosition);
			}
			motors[waveIndex].go();
			motors[waveIndex].holdStatus(5);
			//rt_printf("going: %i %i %i\n", waveIndex, wavePosition, waveCount);
		}
		waveCount = 0;
	}

	for (int i=0; i<NUM_MOTORS; i++){
		if (waveIndex >= i && motors[i].getStatus() == 0 && !motors[i].finishedWave){
			if (motors[i].waveInterations >= waveInterations){
				motors[i].setPosition(0);
				motors[i].go();
				motors[i].finishedWave = true;
			} else {
				//rt_printf("setting %i to %i %i\n", i, motors[i].lastPosition * -1, motors[i].getStatus());
				motors[i].setPosition(motors[i].lastPosition * -1);
				motors[i].go();
				motors[i].waveInterations += 1;
			}
		}
	}
	
}

void edmark(){
	int turn = 200*32*36/14;
	rt_printf("%i\n", turn);
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].setPosition(((int)(turn*i*0.618))%((int)turn));	// 1.61803398875 ?
		//motors[i].setPosition(((int)(turn*(int)(i/2)*0.618))%((int)turn));
		//motors[i].setPosition(turn*i*0.618);
	}
	goAll();
}










