#ifndef AXIS_H_INC
#define AXIS_H_INC

#include <Bela.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <vector>

#define NUM_MOTORS 20

// instruction byte codes:
// 1 means set position - go required
// 2 means set velocity - go required
// 3 means set max velocity - go required
// 4 means set acceleration - go required
// 5 means change microstepping - go NOT required
// 6 means home the motor, velocity is set - go NOT required
// 7 means set home offset - go required
// 10 means GO!

// payloads are always 16bit ints, high byte first

// modes:
// 0 = paused 
// 1 = seeking
// 2 = constantvelocity
// 3 = homing
// 4 = offset
// 5 = newMode
// 6 = justbooted

struct I2CMessage {
    int address;
    std::vector<char> buffer;
    I2CMessage(int addr) : address(addr){};
};

AuxiliaryTask createI2CSendTask();
AuxiliaryTask createI2CReadTask();
void i2cReadTask();
void i2cSendTask();

void setConstantVelocity(int velocity, bool sameDirection);

void setPosition(int motor, int position);
void setVelocity(int motor, int velocity);
void setMaxVelocity(int motor, int velocity);
void setAcceleration(int motor, int acceleration);
void setMicrostepping(int motor, int newMicroStepping);
void go(int motor);
void goAll();
int getStatus();

void createAuxTasks();
void scheduleTasks();

// initialize I2C bus, set up all slave devices
void setupI2C();

// run once in setup(). Checks if Tinys are awake and booted correctly
void bootCheck();

// set microstepping level. must be power of two max 32
void setMicrosteppingSync(int motor, int newstep);

// home the motors
void homeSync(int velocity);

// return the mode of the motor. returns -1 on error
int checkModeSync(int motor);

// set all motors to a constant velocity synchronously (not audio thread safe)
void setConstantVelocitySync(int velocity, bool alternating=false);
void setConstantPositionSync(int numRotations, int microstepping, int vel, bool alternating);
void rampToConstantVelocitySync(int velocity, bool alternating=false);
void setStaggeredVelocitySync(int velocity);
void rampToStaggeredVelocitySync(int velocity);
void staggeredPositionSync(int numRotations, int baseVelocity, bool alternating, bool reverse, bool inverted, bool twoHalves);
void setMicrosteppingSpeedSync(int baseVelocity, bool inverted);
void setConstantPositionDelaySync(int numRotations, int microstepping, int velocity, bool alternating, int delay, bool direction);

void longtest();

// set all motors to seek a position synchronously (not audio thread safe)
void seekSync(int position, bool alternating=false);

void seekRandSync(int position, bool alternating=false);
void setOffsetSync();

void reset();
void goAllSync();
void goSync(int motor);

// call in cleanuprender
void closeI2C();

#endif