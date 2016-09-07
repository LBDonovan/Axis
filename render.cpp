#include <Bela.h>
#include <Bela_I2C.h>
#include <Motor.h>
#include <Axis.hpp>

// from bottom of i2c connector:
// black (GND), white (SDA), yellow (SCL), orange (3.3v)
// P9 01 (top left) black (GND)
// p9 05 (3rd from top left) red (5V power jack only (not USB) to ATTINY 5v)

// ratio: 14/36

#define PHI 1.6180339887
#define phi 0.6180339887

extern int NUM_MOTORS;
int gState = 0;
int requestCount = 0;
int gWaitCount = 0;
int gSecondsToWait;
int gNextState;
int gLastState;
int gMvt1State = 0;
int gTicker = 0;

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
		
		if (requestCount++ > 4410){
			requestCount = 0;
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].requestStatus();
			}
		}
		
		// State 0: Checking to see if it's homed. If not, homes them. If it is, it iterates to State 1.
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
		// State 1: Sets offsets.
		} else if (gState == 1){ 
			rt_printf("SETTING OFFSETS\n");
			for (int i=0; i<NUM_MOTORS; i++){
				motors[i].setOffset(offsets[i]);
			}
			gState += 1;
		// State 2: Waits for the offsets to finish.
		} else if (gState == 2){
			if (allIdle()){
				
				waitCount = 0;
				// These two variables dictate what happens during the Wait State, and where it goes after:
				gSecondsToWait = 2;
				// Right now it's just going to state 4; it could go anywhere.
				gLastState = gState;
				gNextState = 4;
				gState = 3;
			}
		// State 3: WAIT STATE.
		// Waits gSecondsToWait seconds, then moves to gNextState state.
		// These must be set in the last state before entering this one.
		} else if (gState == 3){
		    if (waitCount++ > 44100*gSecondsToWait){
		        gState = gNextState;
		    }
		// State 4: Sets velocities, sets plates in motion.
		} else if (gState == 4){
			rt_printf("SETTING VELOCITIES!\n");
			setRatioVelocity(100, 1.618);
			goAll();
			gWaitCount = 0;
			// Set wait interval, and following state
			gSecondsToWait = 60;
			gNextState = 6;
			// Send to Wait State
			gState = 3;
		}
		// THESE ARE THE MOVEMENT STATES


		// STATE 5: Movement 1
		// Direction: Same
		// Velocity: Same
		// Start: Delay
		else if (gState == 5) {
			
			if (gMvt1State = 0) {
				for (int i = 0; i < NUM_MOTORS / 2; i++) {
					motors[i].setVelocity(100);
				}
				gMvt1State = 1;	
				gWaitCount = 0;
			} 
			else {
				if (gWaitCount < NUM_MOTORS / 2) {
					motor[counter].go();
					gWaitCount += 1;
					gNextState = 5; // return to this state next time
					gLastState = gState; // store this state as the last one
					gSecondsToWait = 0.25; // wait 11025 samples, 1/4 of a second
					gState = 3; // Go to wait state
				} else {
					// If the counter has addressed and started all motors, let it spin for 60 seconds, then pause.
					// After pause, go to State 6.
					gSecondsToWait = 60;
					gLastState = gState;
					gNextState = gState + 1;
					gState = 3;
				}
			}
		}

		// STATE 6: Movement 2
		// Direction: Same
		// Velocity: Ratio
		// Start: Simultaneous
		else if (gState == 6) {
			setRatioVelocity(100, phi);
			goAll();
			// When does it stop, and how do they all stop at the right place?
		}

		// STATE 7: Movement 3
		// Direction: alternating
		// Velocity: Same
		// Start: Simultaneous
		else if (gState == 7) {
			setConstantVelocity_Alternating(100);
			goAll();
			// Wait for 90 seconds
			// Make all stop
		}

		// STATE 8: Movement 4:
		// Direction: Half 1, half -1
		// Velocity: Multiple
		// Start: Simultaneous
		// Inside ones the fastest, outer the slowest
		else if (gState == 8) {
			setRatioVelocity_AlternatingHalves(100, phi);
			goAll();
			// Make it stop!!
		}

		// STATE 9: MOVEMENT 5
		// Direction: Alternating
		// Velocity: ratio
		// Start: simultaneous
		else if (gState == 9) {	
			setConstantVelocity_Alternating(100, phi);
			goAll();
			// OH GOD MAKE IT STOP
		}

		// STATE 10: MOVEMENT 6
		// Direction: same
		// Velocity: ratio
		// Start: simultaneous
		else if (gState == 10) {
			setConstantVelocity(100);
			goAll();
			// WE SHALL NEVER BE STILL AGAIN
		}
		// State 99: BURN IT ALL DOWN (aka stop everything)
		else if (gState == 99) {
			for (int i = 0; i < NUM_MOTORS; i++) {
				motors[i].stopNow();
			}
			// Wait for half a second, then go to gNextState.
			gNextState = gLastState + 1;
			gLastState = gState;
			gSecondsToWait = 2;
			gState = 3;
		}

		
	} // End audio for loop
}  // End render()

void cleanup(BelaContext *context, void *userData)
{
	for (int i=0; i<NUM_MOTORS; i++){
		motors[i].stopNow();
	}
	i2c.cleanup();
}