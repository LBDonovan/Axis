#include <Bela.h>
#include <Bela_I2C.h>
#include <Motor.h>
#include <cstdlib>
#include <Scope.h>

// from bottom of i2c connector:
// black (GND), white (SDA), yellow (SCL), orange (3.3v)
// P9 01 (top left) black (GND)
// p9 05 (3rd from top left) red (5V power jack only (not USB) to ATTINY 5v)

// ratio: 14/36

#define NUM_MOTORS 20

Bela_I2C i2c;
Scope scope;

Motor motors[NUM_MOTORS];
int offsets[NUM_MOTORS] = {0};
int positions[NUM_MOTORS];

int gState = 0;
int requestCount = 0;

void receiveCallback (int address, std::vector<char> buffer){
	//rt_printf("%i: %i\n", address, (int)buffer[0]);
	motors[address-1].setStatus((int)buffer[0]);
}
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

bool setup(BelaContext *context, void *userData)
{
	i2c.setup(1);
	scope.setup(1, context->audioSampleRate, NUM_MOTORS);
	for (int i=0; i<NUM_MOTORS; i++){
		scope.setSlider(i, -8300, 8300, 1, 50);
		positions[i] = 50;
		offsets[i] = 50;
	}
	
	i2c.onReceive(receiveCallback);
	
	for (int i=0; i<NUM_MOTORS; i++){
		if (!motors[i].setup(i+1)){
			//return false;
		}
	}
	
	return true;
}

void render(BelaContext *context, void *userData)
{
	for (unsigned int n=0; n<context->audioFrames; n++){
       scope.log(0.0f);
		if (requestCount++ > 4410){
			requestCount = 0;
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].requestStatus();
			}
		}
		
		if (gState == 0){
			bool homed = true;
			for (int i=0; i<NUM_MOTORS; i++){
				if (!motors[i].home()){
					homed = false;
				}
			}
			if (homed){
				rt_printf("HOMED!\n");
				for (int i=0; i<NUM_MOTORS; i++){
					motors[i].doneHoming();
				}
				gState += 1;
			}
		} else if (gState == 1){
			rt_printf("SETTING OFFSETs\n");
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].setOffset(offsets[i]);
			}
			gState += 1;
		} else if (gState == 2){
			if (allIdle()){
				gState += 1;
				rt_printf("READY\n");
			}
		} else if (gState == 3){
			for (int i=0; i<NUM_MOTORS; i++){
				if (scope.getSliderValue(i) != positions[i]){
					motors[i].setPosition(scope.getSliderValue(i));
					motors[i].go();
					positions[i] = scope.getSliderValue(i);
				}
			}
		}
	}
} 

void cleanup(BelaContext *context, void *userData)
{
	i2c.cleanup();
}