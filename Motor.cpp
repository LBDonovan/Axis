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
	if (newStatus == -1 || statusHolds == 0){
		status = newStatus;
	} else if (statusHolds > 0){
		statusHolds -= 1;
	}
}
int Motor::getStatus(){
	return status;
}
void Motor::holdStatus(int numTimes){
	statusHolds = numTimes;
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

void Motor::reset(){
	setMicrostepping(32);
	setMaxVelocity(200*32);
	setAcceleration(200*32);
	go();
}
void Motor::zero(){
	setOffset(lastPosition+1);
}

bool Motor::home(){
	if (!homing){
		rt_printf("%i setting homing %i\n", HOMING_VELOCITY);
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
			rt_printf("rehome seeking: %i\n", address);
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
			rt_printf("rehoming: %i\n", address);
		} else if (reHomeSeeking && homingIterations > 44100*10 && status == 1){
			i2c.write(address, {
				HOMING_INSTRUCTION,
				high_byte(HOMING_VELOCITY*direction),
				low_byte(HOMING_VELOCITY*direction)
			});
			status = -1;
			reHomeSeeking = false;
			rt_printf("force rehoming: %i\n", address);
		}
		
		homingIterations += 1;
		
	}
	
	return homed;
}
void Motor::doneHoming(){
	homing = false;
}

void Motor::setPosition(int position){
	i2c.write(address, {
		POSITION_INSTRUCTION,
		highest_byte(position*direction),
		higher_byte(position*direction),
		high_byte(position*direction),
		low_byte(position*direction)
	});
	lastPosition = position;
}
void Motor::setVelocity(int velocity){
	i2c.write(address, {
		VELOCITY_INSTRUCTION,
		highest_byte(velocity*direction),
		higher_byte(velocity*direction),
		high_byte(velocity*direction),
		low_byte(velocity*direction)
	});
}
void Motor::setMaxVelocity(int velocity){
	i2c.write(address, {
		MAX_VELOCITY_INSTRUCTION,
		highest_byte(velocity),
		higher_byte(velocity),
		high_byte(velocity),
		low_byte(velocity)
	});
}
void Motor::setAcceleration(int acceleration){
	i2c.write(address, {
		ACCELERATION_INSTRUCTION,
		highest_byte(acceleration),
		higher_byte(acceleration),
		high_byte(acceleration),
		low_byte(acceleration)
	});
}
void Motor::setMicrostepping(int microstepping){
	i2c.write(address, {
		MICROSTEPPING_INSTRUCTION,
		low_byte(microstepping)
	});
}
void Motor::setOffset(int offset){
	i2c.write(address, {
		HOMING_OFFSET_INSTRUCTION,
		highest_byte(offset*direction),
		higher_byte(offset*direction),
		high_byte(offset*direction),
		low_byte(offset*direction)
	});
	go();
}

void Motor::go(){
	i2c.write(address, {GO_INSTRUCTION});
	status = -1;
}
void Motor::stopNow(){
	char buffer[5] = {
		VELOCITY_INSTRUCTION,
		0,
		0,
		0,
		0
	};
	i2c.writeNow(address, buffer, 5);
	usleep(1000);
	char buf[1] = {GO_INSTRUCTION};
	i2c.writeNow(address, buf, 1);
}
