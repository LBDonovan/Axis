/***** Motor.cpp *****/
#include <Motor.h>

bool Motor::setup(int adr){
	
	address = adr;
	
	if (!(address%2)){
		direction = -1;
	} else {
		direction = 1;
	}
	
	i2c.addSlave(address);
	
	std::vector<char> buf = i2c.requestNow(address, 1);
	
	bool booted = true;
	if (buf.size() != 1) {
        // i2c read failed
        rt_printf("Device %i not responding\n", address);
        booted = false;
    } else {
        if ((int)buf[0] == 6){
            rt_printf("Device %i is awake!\n", address);
        } else {
            rt_printf("Device %i wrong response: %d\n", address, buf[0]);
            booted = false;
        }
    }
    
    return booted;
}

void Motor::setStatus(int newStatus){
	status = newStatus;
}
int Motor::getStatus(){
	return status;
}
void Motor::requestStatus(){
	i2c.request(address, 1);
}
char Motor::highest_byte(int in){
	return (char)((in >> 24) & 0xFF);
}
char Motor::higher_byte(int in){
	return (char)((in >> 16) & 0xFF);
}
char Motor::high_byte(int in){
	return (char)((in >> 8) & 0xFF);
}
char Motor::low_byte(int in){
	return (char)(in & 0xFF);
}

bool Motor::home(){
	if (!homing){
		i2c.write(address, {
			HOMING_INSTRUCTION,
			high_byte(HOMING_VELOCITY*direction),
			low_byte(HOMING_VELOCITY*direction)
		});
		status = -1;
		homing = true;
		homed = false;
		reHomeSeeking = false;
		homingIterations = 0;
	} else {
		
		if (!homed && !reHomeSeeking && homingIterations < 22050 && status == 0){
			// if we're homed too soon, move a bit then re-home
			setPosition(200*32);
			go();
			reHomeSeeking = true;
		} else if (!homed && !reHomeSeeking && homingIterations >= 22050 && status == 0){
			homed = true;
			rt_printf("%i homed in %fs\n", address, (float)homingIterations/44100.0f);
		} else if (reHomeSeeking && homingIterations >= 22050 && status == 0){
			i2c.write(address, {
				HOMING_INSTRUCTION,
				high_byte(HOMING_VELOCITY*direction),
				low_byte(HOMING_VELOCITY*direction)
			});
			status = -1;
			reHomeSeeking = false;
		}
		
		homingIterations += 1;
		
	}
	
	return homed;
}

void Motor::setPosition(int position){
	i2c.write(address, {
		POSITION_INSTRUCTION,
		highest_byte(position*direction),
		higher_byte(position*direction),
		high_byte(position*direction),
		low_byte(position*direction)
	});
}

void Motor::go(){
	i2c.write(address, {GO_INSTRUCTION});
	status = -1;
}

