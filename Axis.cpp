/***** Axis.cpp *****/
#include <Axis.h>

int Axis::status[NUM_MOTORS] = {-1};

void Axis::receiveCallbackRender(int address, std::vector<char> buffer){
	if (!buffer.size()){
		Axis::status[address] = -1;
	} else {
	    Axis::status[address] = (int)buffer[0];
	}
	if (!address){
		rt_printf("%i received %i\n", address, buffer[0]);
	}
}

bool Axis::setup(){
	// open I2C bus 1 (/dev/i2c-1)
    i2c.setup(1);
    
    for (int i=0; i<NUM_MOTORS; i++){
        i2c.addSlave(i+1);
    }
    
    i2c.onReceive(Axis::receiveCallbackRender);
    
    // read a single byte from each ATTINY
    // If the ATTINY has booted correctly it should return 6
    bool booted = true;
    for (int i=1; i<=NUM_MOTORS; i++){
    	std::vector<char> buf = i2c.requestNow(i, 1);
    	if (buf.size() != 1) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
            booted = false;
        } else {
            if ((int)buf[0] == 6){
                rt_printf("Device %i is awake!\n", i);
            } else {
                rt_printf("Device %i wrong response: %d\n", i, buf[0]);
                booted = false;
            }
        }
    }
    
    usleep(100000);
    
    return booted;
}
void Axis::cleanup(){
	i2c.cleanup();
}

char Axis::highest_byte(int in){
	return (char)((in >> 24) & 0xFF);
}
char Axis::higher_byte(int in){
	return (char)((in >> 16) & 0xFF);
}
char Axis::high_byte(int in){
	return (char)((in >> 8) & 0xFF);
}
char Axis::low_byte(int in){
	return (char)(in & 0xFF);
}

void Axis::requestAll(){
	for (int i=1; i<=NUM_MOTORS; i++){
		i2c.request(i, 1);
	}
}

bool Axis::home(){
	if (!homing){
		sendHome();
		homingIterations = 0;
		for (int i=1; i<=NUM_MOTORS; i++){
			homeStatus[i] = 0;	// not homed
		}
	} else {
		if (homingIterations > 44100){
			if (!(homingIterations%44100)){
				rt_printf("HI %i, %i\n", homeStatus[0], Axis::status[0]);
			}
			for (int i=1; i<=NUM_MOTORS; i++){
				if (homingIterations < (44100+22050)){	// first quarter second of homing
					if (Axis::status[i] == 0 && homeStatus[i] == 0){	// rehoming needed
						rehome(i);
					}
				} else {	// normal homing period
					if (homeStatus[i] == 0 && Axis::status[i] == 0){	// motor is now homed
						homeStatus[i] = 1; // homed
						rt_printf("motor %i homed in %fs\n", i, (float)homingIterations/44100.0f);
					} else if (homeStatus[i] == 2 && Axis::status[i] == 0){		// motor is seeking
						rehome(i);
					}
				}
			}
		}
	}
	homingIterations += 1;

	bool homeFinished = true;
	for (int i=1; i<=NUM_MOTORS; i++){
		if (homeStatus[i] != 1){
			homeFinished = false;
		}
	}
	return homeFinished;
}
void Axis::rehome(int address){
	// printf("rehome %i\n", address);
	if (homeStatus[address] == 0){	// not yet rehomed, move it a bit
		rt_printf("%i starting rehome shift (%i, %i)\n", address, Axis::status[address], homeStatus[address]);
		int position = 200*32;
		if (!(address % 2)){
            position *= -1;
        }
		i2c.write(address, {
    		0x1, 					// instruction
    		highest_byte(position),
    		higher_byte(position),
    		high_byte(position),
    		low_byte(position)
    	});
    	i2c.write(address, {(char)(10)});
    	homeStatus[address] = 2;
    	Axis::status[address] = -1;
	} else if (homeStatus[address] == 2){	// finished moving, do rehome
		rt_printf("%i starting rehome\n", address);
		int velocity = 2000;
		if (!(address % 2)){
            velocity *= -1;
        }
		i2c.write(address, {
    		0x6, 					// instruction
    		high_byte(velocity), 	// velocity high byte
    		low_byte(velocity)		// velocity low byte
    	});
    	homeStatus[address] = 0;
    	Axis::status[address] = -1;
	}
}

void Axis::sendHome(){
	int sign = 1, velocity, vel = 2000;
	for (int i=1; i<=NUM_MOTORS; i++){
        sign *= (-1);
        velocity = vel*sign;
        //rt_printf("address: %i, bytes: %i, %i, %i\n", i, 0x6, (int)high_byte(velocity), (int)low_byte(velocity));
    	i2c.write(i, {
    		0x6, 					// instruction
    		high_byte(velocity), 	// velocity high byte
    		low_byte(velocity)		// velocity low byte
    	});
    	/*char message[3];
        message[0] = 6;                         // instruction byte
        message[1] = (velocity >> 8) & 0xFF;    // high byte
        message[2] = velocity & 0xFF;           // low byte
        i2c.write(i, message, 3);*/
        homing = true;
    }
}

/*void Axis::setPosition(int address, int position){
	i2c.write(address, {
		0x1, 					// instruction
		highest_byte(position),
		higher_byte(position),
		high_byte(position),
		low_byte(position)
	});
	// TODO: go
}*/