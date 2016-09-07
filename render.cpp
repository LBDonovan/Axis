#include <Bela.h>
#include <Bela_I2C.h>
#include <Motor.h>
#include <Axis.hpp>

// from bottom of i2c connector:
// black (GND), white (SDA), yellow (SCL), orange (3.3v)
// P9 01 (top left) black (GND)
// p9 05 (3rd from top left) red (5V power jack only (not USB) to ATTINY 5v)

// ratio: 14/36

int gState = 0;
int requestCount = 0;
int waitCount = 0;

void receiveCallback (int address, std::vector<char> buffer){
	//rt_printf("%i: %i\n", address, (int)buffer[0]);
	motors[address-1].setStatus((int)buffer[0]);
}

bool setup(BelaContext *context, void *userData)
{
	i2c.setup(1);
	
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
		
		if (requestCount++ > 441){
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
				waitCount = 0;
			}
		} else if (gState == 3){
		    if (waitCount++ > 44100*2){
		        gState += 1;
		    }
		} else if (gState == 4){
			rt_printf("STARTING WAVE!\n");
			startWaveMotion(8300/2);
			gState += 1;
		} else if (gState == 5){
			waveMotion(44100/8);
		}
		
	}
} 

void cleanup(BelaContext *context, void *userData)
{
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].stopNow();
	}
	i2c.cleanup();
}