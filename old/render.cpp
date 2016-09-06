#include <Bela.h>
#include <cmath>
#include <Axis.h> 

// from bottom of i2c connector:
// black (GND), white (SDA), yellow (SCL), orange (3.3v)
// P9 01 (top left) black (GND)
// p9 05 (3rd from top left) red (5V power jack only (not USB) to ATTINY 5v)

// ratio: 14/36

int count = 0;
int state = 0;
int numStates = 7;
bool firstTime = true;

// status array - initialised to -1
int status[NUM_MOTORS+1] = {-1};

bool setup(BelaContext *context, void *userData)
{
    
    createAuxTasks();
    
    setupI2C();
    bootCheck();
    
    reset();
    
    //seekRandSync(500*32, true);
    
    usleep(1000);
    
    homeSync(2000);
    
    setOffsetSync();
    
    usleep(100000);
    
    // setConstantPositionSync(int numRotations, int microstepping, int vel, bool alternating)
    //setConstantPositionSync(3, 4, 500, true);

    // void staggeredPositionSync(int numRotations, int baseVelocity, bool alternating, bool clockwise, bool inverted);
    //staggeredPositionSync(3, 16, false, false, true);

	return true;
}

void render(BelaContext *context, void *userData)
{
    for (unsigned int n=0; n<context->audioFrames; n++){
        
        if (count++ == 22050){
            count = 0;
            if (firstTime){
                firstTime = false;
                scheduleTasks();
            } else {
                //rt_printf("status: %i\n", getStatus());
                scheduleTasks();
                if (getStatus() == 0){
                    rt_printf("state: %i\n", state);
                    if (state == 0){
                        reset();
                        // void staggeredPositionSync(int numRotations, int baseVelocity, bool alternating, bool clockwise, bool inverted, bool twoHalves);
                        // setConstantPositionSync(1, 8, 500, true);
                        setConstantPositionDelaySync(3, 8, 500, false, 500000, true);
                    } else if (state == 1){
                        reset();
                        setConstantPositionDelaySync(3, 8, 500, false, 500000, false);
                    } else if (state == 2){
                        reset();
                        // fast in middle, halves in same direction
                        staggeredPositionSync(3, 16, false, false, false, false);
                    }else if (state == 3){
                        reset();
                        staggeredPositionSync(3, 16, false, true, true, false);
                        // fast on outside
                        
                    } else if (state == 4){
                        reset();
                        setConstantPositionSync(10, 8, 500, true);
                        
                    } else if (state == 5){
                        reset();
                        staggeredPositionSync(3, 16, true, false, false, true);
                        // alternating directions fast in middle
                        
                        
                    } else if (state == 6){
                        reset();
                        staggeredPositionSync(3, 16, false, true, false, true);
                        // fast on middle, halves in opposite directions
                    }
                    state = (state+1)%numStates;
                }
            }
        }
        
    }

}

void cleanup(BelaContext *context, void *userData)
{
    closeI2C();
}