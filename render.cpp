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
	if (buffer.size() != 1){
		rt_printf("AAARGH %i", address);
	}
	/*if (gState == 6){
		printf("%i: %i\n", address-1, (int)buffer[0]);
	}*/
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

long mainCount;
void render(BelaContext *context, void *userData)
{
	for (unsigned int n=0; n<context->audioFrames; n++){
		
		mainCount++;
		
		if (requestCount++ > 4410){
			requestCount = 0;
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].requestStatus();
			}
		}
		
		if (gState == 0){
			resetAll();
			zeroAll();
			waitCount = 0;
			gState += 1;
		} else if (gState == 1){
		    if (waitCount++ > 4410){
		        gState += 1;
		    }
		} else if (gState == 2){
			bool homed = true;
			for (int i=0; i<NUM_MOTORS; i++){
				if (!motors[i].home()){
					homed = false;
					if (!(mainCount%44100)){
						//rt_printf("not homed: %i\n", i);
					}
				}
			}
			if (homed){
				rt_printf("HOMED!\n");
				for (int i=0; i<NUM_MOTORS; i++){
					motors[i].doneHoming();
				}
				gState += 1;
			}
		} else if (gState == 3){
			rt_printf("SETTING OFFSETs\n");
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].setOffset(offsets[i]);
			}
			gState += 1;
		} else if (gState == 4){
			if (allIdle()){
				gState = 15;	// STATE!
				waitCount = 0;
			}
		} else if (gState == 5){
		    if (waitCount++ > 44100){
		        gState += 1;
		    }
		} else if (gState == 6){
			rt_printf("EDMARK!\n");
			edmark();
			holdStatusAll(10);
			gState += 1;
		} else if (gState == 7){
			if (allIdle()){
				gState += 1;
				waitCount = 0;
				zeroAll();
			}
		} else if (gState == 8){
		    if (waitCount++ > 44100){
		        gState += 1;
		    }
		} else if (gState == 9){
			rt_printf("setMultiplePosition!\n");
			setMultiplePosition(ONE_TURN, 500);
			goAll();
			holdStatusAll(1);
			gState += 1;
		} else if (gState == 10){
			if (allIdle()){
				gState += 1;
				waitCount = 0;
				resetAll();
			}
		} else if (gState == 11){
		    if (waitCount++ > 4410){
		        gState += 1;
		        zeroAll();
		        waitCount = 0;
		    }
		} else if (gState == 12){
		    if (waitCount++ > 4410){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 13){
			rt_printf("STARTING WAVE!\n");
			startWaveMotion(ONE_TURN/2, 3, true);
			gState += 1;
		} else if (gState == 14){
			waveMotion(44100/8);
			if (allIdle()){
				gState = 0;
			} else {
				if (!(mainCount%44100)){
					for (int i=0; i<NUM_MOTORS; i++){
						rt_printf("%i: %i\n", i, motors[i].getStatus());
					}
				}
			}
		} else if (gState == 15){
		    if (waitCount++ > 4410){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 16){
		    setRandomPosition(ONE_TURN/32, ONE_TURN/2);
		    goAll();
		    holdStatusAll(10);
		    gState += 1;
		} else if (gState == 17){
		    if (allIdle()){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 18){
		    if (waitCount++ > 3*44100/2){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 19){
		    setAllPosition(0);
		    goAll();
		    holdStatusAll(10);
		    gState += 1;
		    waitCount = 0;
		} else if (gState == 20){
		    if (allIdle()){
		        gState += 1;
		    }
		} else if (gState == 21){
		    if (waitCount++ > 3*44100/2){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 22){
		    setSpiralPosition(200*32/12, 200*32);
		    goAll();
		    holdStatusAll(10);
		    gState += 1;
		} else if (gState == 23){
		    if (allIdle()){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 24){
		    if (waitCount++ > 3*44100/2){
		        gState += 1;
		        waitCount = 0;
		    }
		} else if (gState == 25){
		    setAllPosition(ONE_TURN);
		    goAll();
		    holdStatusAll(10);
		    gState += 1;
		    waitCount = 0;
		} else if (gState == 26){
		    if (allIdle()){
		        gState += 1;
		    }
		} else if (gState == 27){
		    if (waitCount++ > 3*44100/2){
		        gState = 16;
		        waitCount = 0;
		    }
		}
		
	}
} 

void cleanup(BelaContext *context, void *userData)
{
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].stopNow();
		usleep(1000);
	}
	i2c.cleanup();
}