/***** Axis.h *****/
#include <Bela_I2C.h>

#define NUM_MOTORS 20

class Axis{
	public:
		Axis():homing(false){}
		
		bool setup();
		bool home();
		void cleanup();
		void requestAll();
		
		static int status[NUM_MOTORS];
		
	private:
		Bela_I2C i2c;
		static void receiveCallbackRender(int address, std::vector<char> buffer);
		
		bool homing;
		int homingIterations;
		int homeStatus[NUM_MOTORS];
		void rehome(int address);
		
		void sendHome();
		char highest_byte(int);
		char higher_byte(int);
		char high_byte(int);
		char low_byte(int);
};