/***** Motor.h *****/
#include <Bela_I2C.h>

extern Bela_I2C i2c;

#define POSITION_INSTRUCTION 1
#define HOMING_INSTRUCTION 6
#define GO_INSTRUCTION 10

#define HOMING_VELOCITY 2000

class Motor{
	public:
		Motor():homing(false), status(-1){}
		
		bool setup(int address);
		
		void setStatus(int status);
		int getStatus();
		void requestStatus();
		
		bool home();
		bool homing;
		bool homed;
		int homingIterations;
		bool reHomeSeeking;
		
		void setPosition(int position);
		void go();
		
	private:
		int address;
		int status;
		int direction;
		
		char highest_byte(int);
		char higher_byte(int);
		char high_byte(int);
		char low_byte(int);
};