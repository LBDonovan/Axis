/***** Motor.h *****/
#include <Bela_I2C.h>

extern Bela_I2C i2c;

#define POSITION_INSTRUCTION 1
#define VELOCITY_INSTRUCTION 2
#define MAX_VELOCITY_INSTRUCTION 3
#define ACCELERATION_INSTRUCTION 4
#define MICROSTEPPING_INSTRUCTION 5
#define HOMING_INSTRUCTION 6
#define HOMING_OFFSET_INSTRUCTION 7
#define GO_INSTRUCTION 10

#define HOMING_VELOCITY 2000

class Motor{
	public:
		Motor():homing(false), status(-1), statusHolds(0){}
		
		bool setup(int address);
		
		void setStatus(int status);
		void holdStatus(int numTimes);
		int getStatus();
		void requestStatus();
		
		bool home();
		bool homing;
		bool homed;
		int homingIterations;
		bool reHomeSeeking;
		void doneHoming();
		
		void setPosition(int position);
		void setVelocity(int velocity);
		void setMaxVelocity(int velocity);
		void setAcceleration(int acceleration);
		void setMicrostepping(int microstepping);
		void setOffset(int offset);
		
		void go();
		void stopNow();
		void reset();
		void zero();
		
		int lastPosition;
		int waveInterations;
		bool finishedWave;
		
	private:
		int address;
		int status;
		int statusHolds;
		int direction;
		
		char highest_byte(int);
		char higher_byte(int);
		char high_byte(int);
		char low_byte(int);
};