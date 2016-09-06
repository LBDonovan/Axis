#include <Axis.h>

// array to hold all I2C slave handles
int motors[NUM_MOTORS+1];

// microstepping level. must be power of two max 32
int microstepping[NUM_MOTORS+1] = {32};
int positions[NUM_MOTORS+1] = {0};
int velocities[NUM_MOTORS+1] = {0};
int accelerations[NUM_MOTORS+1] = {0};
int final_velocities[NUM_MOTORS+1] = {0};
int increments[NUM_MOTORS+1] = {0};

int offsets[20] = {-2750, 5030, -7100, -6710, 325, 5920, 1500, 9350, -4330, -3740, -3630, 9600, 8530, 8445, -3420, 6595, 710, -3600, 870, -7659};

// instruction byte codes:
// 1 means set position - go required
// 2 means set velocity - go required
// 3 means set max velocity - go required
// 4 means set acceleration - go required
// 5 means change microstepping - go NOT required
// 6 means home the motor, velocity is set - go NOT required
// 7 means set home offset - go required
// 10 means GO!

void staggeredPositionSync(int numRotations, int baseVelocity, bool alternating, bool clockwise, bool inverted, bool twoHalves){
    
    usleep(6000000);
    
void setMicrosteppingSpeedSync(baseVelocity, inverted);
    
    for (int i=1; i<=10; i++){
        
        if (inverted){
            
            velocities[11-i] = baseVelocity*i*microstepping[11-i];
            velocities[10+i] = baseVelocity*i*microstepping[10+i];

            accelerations[11-i] = velocities[11-i];
            accelerations[10+i] = velocities[10+i];
            
            if (clockwise){
                positions[11-i] = numRotations*200*36*i*microstepping[11-i]/14;
                positions[10+i] = numRotations*200*36*i*microstepping[10+i]/14;
            } else {
                positions[11-i] = (-1)*numRotations*200*36*i*microstepping[11-i]/14;
                positions[10+i] = (-1)*numRotations*200*36*i*microstepping[10+i]/14;
            }
            
            if (!alternating && !(i % 2)){
                positions[11-i] *= -1;
                positions[10+i] *= -1;
            }
            
        } else {
            
            velocities[i] = baseVelocity*i*microstepping[i];
            velocities[21-i] = baseVelocity*i*microstepping[21-i];
            
            accelerations[i] = velocities[i];
            accelerations[21-i] = velocities[21-i];
            
            if (clockwise){
                positions[i] = (-1)*numRotations*200*36*i*microstepping[i]/14;
                positions[21-i] = (-1)*numRotations*200*36*i*microstepping[21-i]/14;
            } else {
                positions[i] = numRotations*200*36*i*microstepping[i]/14;
                positions[21-i] = numRotations*200*36*i*microstepping[21-i]/14;
            }
            
            if (!alternating && !(i % 2)){
                positions[i] *= -1;
                positions[21-i] *= -1;
            }
            
        }
        
    }
    
    for (int i=1; i<=10; i++){
        
        char message[5];
        message[0] = 3;                         // instruction byte
        message[1] = (velocities[i] >> 24) & 0xFF;    // high byte
        message[2] = (velocities[i] >> 16) & 0xFF;
        message[3] = (velocities[i] >> 8) & 0xFF;
        message[4] = velocities[i] & 0xFF; 
        
        // first 10 motors
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", i, velocities[i]);
        }
        
        message[1] = (velocities[21-i] >> 24) & 0xFF;    // high byte
        message[2] = (velocities[21-i] >> 16) & 0xFF;
        message[3] = (velocities[21-i] >> 8) & 0xFF;
        message[4] = velocities[21-i] & 0xFF; 
        
        // second 10
        if (write(motors[21-i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", 21-i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", 21-i, velocities[i]);
        }
        
        message[0] = 4;                         // instruction byte
        message[1] = (accelerations[i] >> 24) & 0xFF;    // high byte
        message[2] = (accelerations[i] >> 16) & 0xFF;
        message[3] = (accelerations[i] >> 8) & 0xFF;
        message[4] = accelerations[i] & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i acceleration set to %i\n", i, accelerations[i]);
        }
        
        message[1] = (accelerations[21-i] >> 24) & 0xFF;    // high byte
        message[2] = (accelerations[21-i] >> 16) & 0xFF;
        message[3] = (accelerations[21-i] >> 8) & 0xFF;
        message[4] = accelerations[21-i] & 0xFF; 
        
        if (write(motors[21-i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", 21-i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", 21-i, velocities[i]);
        }
        
        rt_printf("%i: microstepping %i, acceleration %i, velocity %i, position %i\n", i, microstepping[i], accelerations[i], velocities[i], positions[i]);
        rt_printf("%i: microstepping %i, acceleration %i, velocity %i, position %i\n", 21-i, microstepping[21-i], accelerations[21-i], velocities[21-i], positions[21-i]);
    }
    goAllSync();
    usleep(10000);
    
    for (int i=1; i<=10; i++){
        
        char message[5];
        message[0] = 7;                         // instruction byte
        message[1] = (positions[i] >> 24) & 0xFF;    // high byte
        message[2] = (positions[i] >> 16) & 0xFF;
        message[3] = (positions[i] >> 8) & 0xFF;
        message[4] = positions[i] & 0xFF;
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i position set to %i\n", i, positions[i]);
        }
        
        if (!twoHalves){
            positions[21-i] *= (-1);
        }
        
        message[1] = (positions[21-i] >> 24) & 0xFF;    // high byte
        message[2] = (positions[21-i] >> 16) & 0xFF;
        message[3] = (positions[21-i] >> 8) & 0xFF;
        message[4] = positions[21-i] & 0xFF;
        
        if (write(motors[21-i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", 21-i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", 21-i, velocities[i]);
        }
        
    }
    
    goAllSync();
    
}

void setMicrosteppingSpeedSync(int baseVelocity, bool inverted){
    if (baseVelocity == 16){
        if (inverted){
            setMicrosteppingSync(1, 2);
            setMicrosteppingSync(2, 2);
            setMicrosteppingSync(3, 4);
            setMicrosteppingSync(4, 4);
            setMicrosteppingSync(5, 4);
            setMicrosteppingSync(6, 4);
            setMicrosteppingSync(7, 8);
            setMicrosteppingSync(8, 8);
            setMicrosteppingSync(9, 16);
            setMicrosteppingSync(10, 16);
            
            setMicrosteppingSync(11, 16);
            setMicrosteppingSync(12, 16);
            setMicrosteppingSync(13, 8);
            setMicrosteppingSync(14, 8);
            setMicrosteppingSync(15, 4);
            setMicrosteppingSync(16, 4);
            setMicrosteppingSync(17, 4);
            setMicrosteppingSync(18, 4);
            setMicrosteppingSync(19, 2);
            setMicrosteppingSync(20, 2);
        } else {
            setMicrosteppingSync(1, 16);
            setMicrosteppingSync(2, 16);
            setMicrosteppingSync(3, 8);
            setMicrosteppingSync(4, 4);
            setMicrosteppingSync(5, 4);
            setMicrosteppingSync(6, 4);
            setMicrosteppingSync(7, 2);
            setMicrosteppingSync(8, 4);
            setMicrosteppingSync(9, 4);
            setMicrosteppingSync(10, 2);
            
            setMicrosteppingSync(11, 2);
            setMicrosteppingSync(12, 4);
            setMicrosteppingSync(13, 4);
            setMicrosteppingSync(14, 2);
            setMicrosteppingSync(15, 4);
            setMicrosteppingSync(16, 4);
            setMicrosteppingSync(17, 4);
            setMicrosteppingSync(18, 8);
            setMicrosteppingSync(19, 16);
            setMicrosteppingSync(20, 16);
        }
    }
}

void setupI2C(){
    
    // setup I2C bus
    // motors[0] is address 0, for broadcast to all devices
    if ((motors[0] = open("/dev/i2c-1", 02)) < 0){
		rt_printf("Can't open I2C file\n");
	}
	
	// Assign slave address
	if (ioctl(motors[0], I2C_SLAVE, 0) < 0){
		rt_printf("Can't find device %i\n", 0);
	} else {
		//rt_printf("Found device %i!\n", 0);
	}
    
    for (int i=1; i<=NUM_MOTORS; i++){
    
        // open i2c file
        if ((motors[i] = open("/dev/i2c-1", 02)) < 0){
    		rt_printf("Can't open I2C file\n");
    	}
    	
    	// Assign slave address
    	if (ioctl(motors[i], I2C_SLAVE, i) < 0){
    		rt_printf("Can't find device %i\n", i);
    	} else {
    		//rt_printf("Found device %i!\n", i);
    	}

    }
}

void setMicrosteppingSync(int motor, int newstep){
    
    if (motor == 0){
        for (int i=0; i<=NUM_MOTORS; i++){
            microstepping[i] = newstep;
        }
    } else {
        microstepping[motor] = newstep;
    }
    
    char message[2];
    message[0] = 5;                         // instruction byte
    message[1] = newstep;    // high byte
    
    if (write(motors[motor], message, 2) != 2) {
        // i2c read failed
        rt_printf("Device %i not responding\n", motor);
    } else {
        //rt_printf("Device %i microstepping set to %i\n", motor, newstep);
    }
    
}

void bootCheck(){
    
    // read a single byte from each ATTINY
    // If the ATTINY has booted correctly it should return 6
    for (int i=1; i<=NUM_MOTORS; i++){
        char buf[1] = {0};
    	if (read(motors[i], buf, 1) != 1) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            if ((int)buf[0] == 6){
                rt_printf("Device %i is awake!\n", i);
            } else {
                rt_printf("Device %i wrong response: %d\n", i, buf[0]);
            }
        }
    }
}

void reset(){
    for (int i=1; i<=NUM_MOTORS; i++){
        
        setMicrosteppingSync(0, 32);
        int velocity = 200*32;
        int acceleration = 200*32;
        
        char message[5];
        message[0] = 3;                         // instruction byte
        message[1] = (velocity >> 24) & 0xFF;    // high byte
        message[2] = (velocity >> 16) & 0xFF;
        message[3] = (velocity >> 8) & 0xFF;
        message[4] = velocity & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", i, velocity);
        }
        
        message[0] = 4;                         // instruction byte
        message[1] = (acceleration >> 24) & 0xFF;    // high byte
        message[2] = (acceleration >> 16) & 0xFF;
        message[3] = (acceleration >> 8) & 0xFF;
        message[4] = acceleration & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i acceleration set to %i\n", i, acceleration);
        }
        
    }
    goAllSync();
}

void homeSync(int vel){
    
    int sign = -1;
    int velocity = vel;
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        sign *= (-1);
        velocity = vel*sign;
        
        char message[3];
        message[0] = 6;                         // instruction byte
        message[1] = (velocity >> 8) & 0xFF;    // high byte
        message[2] = velocity & 0xFF;           // low byte
        
    	if (write(motors[i], message, 3) != 3) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            rt_printf("Device %i homing at velocity %i\n", i, velocity);
        }
        
    }
    
    bool homed = false;
    bool motorHomed[NUM_MOTORS+1] = {};
    int iterations = 0;
    bool rehomeNeeded = false;
    std::vector<int> rehomeIndices;
    while(!homed){
        bool homeFlag = true;
        for (int i=1; i<=NUM_MOTORS; i++){
            
            if (!motorHomed[i]){
                if (checkModeSync(i) == 0){
                    rt_printf("Motor %i home with %i iterations!\n", i, iterations);
                    motorHomed[i] = true;
                    if (iterations < 10){
                        rehomeNeeded = true;
                        rehomeIndices.push_back(i);
                    }
                } else {
                    homeFlag = false;
                }
            }
            
        }
        
        if (homeFlag){
            homed = true;
        }
        iterations += 1;
    }
    
    if (rehomeNeeded){

        int position = 200*32;
        
        for (unsigned int i=0; i<rehomeIndices.size(); i++){
            
            rt_printf("Rehoming device %i\n", rehomeIndices[i]);
            
            if (rehomeIndices[i] % 2){
                position = abs(position);
            } else {
                position = abs(position)*-1;
            }

            char message[5];
            message[0] = 1;                         // instruction byte
            message[1] = (position >> 24) & 0xFF;    // high byte
            message[2] = (position >> 16) & 0xFF;
            message[3] = (position >> 8) & 0xFF;
            message[4] = position & 0xFF; 
            
        	if (write(motors[rehomeIndices[i]], message, 5) != 5) {
                // i2c read failed
                rt_printf("Device %i not responding\n", rehomeIndices[i]);
            } else {
                rt_printf("Device %i re-homing to position %i\n", rehomeIndices[i], position);
            }
            
        }
        
        goAllSync();
        
        usleep(5000000);
        
        for (unsigned int i=0; i<rehomeIndices.size(); i++){
            
            motorHomed[rehomeIndices[i]] = false;
            
            if (rehomeIndices[i] % 2){
                velocity = abs(velocity);
            } else {
                velocity = abs(velocity)*-1;
            }

            char message[3];
            message[0] = 6;                         // instruction byte
            message[1] = (velocity >> 8) & 0xFF;
            message[2] = velocity & 0xFF; 
            
        	if (write(motors[rehomeIndices[i]], message, 3) != 3) {
                // i2c read failed
                rt_printf("Device %i not responding\n", rehomeIndices[i]);
            } else {
                rt_printf("Device %i homing at velocity %i\n", rehomeIndices[i], velocity);
            }
            
        }
        
        homed = false;
        while(!homed){
            bool homeFlag = true;
            for (unsigned int i=0; i<rehomeIndices.size(); i++){
                
                if (!motorHomed[rehomeIndices[i]]){
                    if (checkModeSync(rehomeIndices[i]) == 0){
                        rt_printf("Motor %i home!\n", rehomeIndices[i]);
                        motorHomed[rehomeIndices[i]] = true;
                    } else {
                        homeFlag = false;
                    }
                }
                
            }
            
            if (homeFlag){
                homed = true;
            }

        }
    }
    
    rt_printf("Homed!\n");
    
    usleep(1000000);

}

int checkModeSync(int motor){
    
    int returnValue = -1;
    
    char buf[1] = {0};
	if (read(motors[motor], buf, 1) != 1) {
        // i2c read failed
        rt_printf("Device %i not responding\n", motor);
    } else {
        returnValue = (int)buf[0];
    }
    
    return returnValue;
    
}

void rampToConstantVelocitySync(int vel, bool alternating){
    
    rt_printf("Velocity ramping to %i\n", vel);
    
    int velocity = 0;
    
    while (velocity < vel){
        
        velocity += 1;
        
        setConstantVelocitySync(velocity, alternating);
        
        usleep(10000);
        
    }
    
    rt_printf("Velocity ramped to %i\n", velocity);
    
}

void setConstantVelocitySync(int vel, bool alternating){
    
    int velocity = vel;
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        if (i % 2){
            velocity = vel;
        } else {
            if (alternating){
                velocity *= -1;
            }
        }

        char message[5];
        message[0] = 2;                         // instruction byte
        message[1] = (velocity >> 24) & 0xFF;    // high byte
        message[2] = (velocity >> 16) & 0xFF;
        message[3] = (velocity >> 8) & 0xFF;
        message[4] = velocity & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i velocity set to %i\n", i, velocity);
        }
        
    }
    
    goAllSync();
    
}

void setConstantPositionSync(int numRotations, int microstepping, int velocity, bool alternating){
    
    usleep(6000000);
    
    setMicrosteppingSync(0, microstepping);
    int position;

    for (int i=1; i<=NUM_MOTORS; i++){

        char message[5];
        message[0] = 3;                         // instruction byte
        message[1] = (velocity >> 24) & 0xFF;    // high byte
        message[2] = (velocity >> 16) & 0xFF;
        message[3] = (velocity >> 8) & 0xFF;
        message[4] = velocity & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", i, velocity);
        }
        
        message[0] = 4;                         // instruction byte
        
        if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i acceleration set to %i\n", i, velocity);
        }
        
    }
        
    goAllSync();
    
    usleep(10000);
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        if (!alternating){
            if (i%2) {
                position = (-1)*numRotations*200*36*microstepping/14;
            } else {
                position = numRotations*200*36*microstepping/14;
            }
        } else {
            position = numRotations*200*36*microstepping/14;
        }

        char message[5];
        message[0] = 7;                         // instruction byte
        message[1] = (position >> 24) & 0xFF;    // high byte
        message[2] = (position >> 16) & 0xFF;
        message[3] = (position >> 8) & 0xFF;
        message[4] = position & 0xFF; 
        
        if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i position set to %i\n", i, position);
        }
        
    }
    
    goAllSync();
    
}

void setConstantPositionDelaySync(int numRotations, int microstepping, int velocity, bool alternating, int delay, bool direction){
    
    setMicrosteppingSync(0, microstepping);
    int position;

    for (int i=1; i<=NUM_MOTORS; i++){

        char message[5];
        message[0] = 3;                         // instruction byte
        message[1] = (velocity >> 24) & 0xFF;    // high byte
        message[2] = (velocity >> 16) & 0xFF;
        message[3] = (velocity >> 8) & 0xFF;
        message[4] = velocity & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i max velocity set to %i\n", i, velocity);
        }
        
        message[0] = 4;                         // instruction byte
        
        if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i acceleration set to %i\n", i, velocity);
        }
        
    }
        
    goAllSync();
    
    usleep(10000);
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        if (!alternating){
            if (i%2) {
                position = (-1)*numRotations*200*36*microstepping/14;
            } else {
                position = numRotations*200*36*microstepping/14;
            }
        } else {
            position = numRotations*200*36*microstepping/14;
        }

        char message[5];
        message[0] = 7;                         // instruction byte
        message[1] = (position >> 24) & 0xFF;    // high byte
        message[2] = (position >> 16) & 0xFF;
        message[3] = (position >> 8) & 0xFF;
        message[4] = position & 0xFF; 
        
        int motor = 21-i;
        
        if (direction){
            motor = i;
        }
        
        if (write(motors[motor], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i position set to %i\n", i, position);
        }
        
        goSync(motor);
        
        usleep(delay);
        
    }
    
    goAllSync();
    
}

void rampToStaggeredVelocitySync(int vel){
    
    rt_printf("Velocity ramping to %i\n", vel);
    
    for (int i=1; i<=10; i++){
        if (i >= 3 && i <= 5){
            setMicrosteppingSync(i, 8);
        } else if (i == 6){
            setMicrosteppingSync(i, 8);
        } else if (i == 7){
            setMicrosteppingSync(i, 4);
        } else if (i == 8){
            setMicrosteppingSync(i, 4);
        } else if (i == 9){
            setMicrosteppingSync(i, 4);
        } else if (i == 10){
            setMicrosteppingSync(i, 4);
        }
        
        velocities[i] = 0;
        if (i % 2){
            final_velocities[i] = vel*i*microstepping[i];
        } else {
            final_velocities[i] = vel*i*microstepping[i]*-1;
        }
        increments[i] = final_velocities[i]/vel;
        
        rt_printf("%i: microstepping %i, final velocity %i, increment %i\n", i, microstepping[i], final_velocities[i], increments[i]);
        
    }
    
    for (int j=1; j<=vel; j++){
        for (int i=1; i<=10; i++){
            
            velocities[i] += increments[i];
            
            char message[5];
            message[0] = 2;                         // instruction byte
            message[1] = (velocities[i] >> 24) & 0xFF;    // high byte
            message[2] = (velocities[i] >> 16) & 0xFF;
            message[3] = (velocities[i] >> 8) & 0xFF;
            message[4] = velocities[i] & 0xFF;
            
        	if (write(motors[i], message, 5) != 5) {
                // i2c read failed
                rt_printf("Device %i not responding\n", i);
            } else {
                rt_printf("Device %i velocity set to %i\n", i, velocities[i]);
            }
        }
        goAllSync();
        usleep(1600000);
    }
    
    /*int velocity = 0;
    
    while (velocity < vel){
        
        velocity += 1;
        
        setStaggeredVelocitySync(velocity);
        
        usleep(1600000);
        
    }*/
    
    rt_printf("Velocity ramped to %i\n", vel);
    
}

void setStaggeredVelocitySync(int vel){
    
   // int sign = -1;
    int velocity;
    
    for (int i=1; i<=10; i++){
        
        if (i % 2){
            velocity = vel*i;
        } else {
            velocity = -1*vel*i;
        }
        
        if (i >= 3 && i <= 5){
            velocity = velocity/2;
        } else if (i == 6){
            velocity = velocity/4;
        } else if (i == 7){
            velocity = velocity/4;
        } else if (i == 8){
            velocity = velocity/4;
        } else if (i == 9){
            velocity = velocity/8;
        } else if (i == 10){
            velocity = velocity/8;
        }
        
        rt_printf("%i velocty: %i\n", i, velocity);

        /*if (abs(velocity*i) > 2000){
            setMicrosteppingSync(i, 16);
            //setMicrosteppingSync(21-i, 16);
            velocity = (vel*sign*i)/2;
        } else {
            velocity = vel*sign*i;
        }*/
        
        //rt_printf("%i velocty: %i\n", i, velocity);

        char message[5];
        message[0] = 2;                         // instruction byte
        message[1] = (velocity >> 24) & 0xFF;    // high byte
        message[2] = (velocity >> 16) & 0xFF;
        message[3] = (velocity >> 8) & 0xFF;
        message[4] = velocity & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            //rt_printf("Device %i velocity set to %i\n", i, velocity);
        }
        
        /*if (write(motors[21-i], message, 3) != 3) {
            // i2c read failed
            rt_printf("Device %i not responding\n", 21-i);
        } else {
            rt_printf("Device %i velocity set to %i\n", 21-i, velocity);
        }*/
        
    }
    
    goAllSync();
    
}

void seekSync(int pos, bool alternating){
    
    int sign = -1;
    int position = pos;
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        if (alternating){
            sign *= (-1);
            position = pos*sign;
        }

        char message[5];
        message[0] = 1;                         // instruction byte
        message[1] = (position >> 24) & 0xFF;    // high byte
        message[2] = (position >> 16) & 0xFF;
        message[3] = (position >> 8) & 0xFF;
        message[4] = position & 0xFF; 
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            rt_printf("Device %i seeking position %i\n", i, position);
        }
        
    }
    
    goAllSync();
    
}

void seekRandSync(int pos, bool alternating){
    
    int sign = -1;
    
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        int position = rand() % pos - pos/2;
        
        if (alternating){
            sign *= (-1);
            position = position*sign;
        }

        char message[5];
        message[0] = 1;                         // instruction byte
        message[1] = (position >> 24) & 0xFF;    // high byte
        message[2] = (position >> 16) & 0xFF;
        message[3] = (position >> 8) & 0xFF;
        message[4] = position & 0xFF;
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            rt_printf("Device %i seeking position %i\n", i, position);
        }
        
    }
    
    goAllSync();
}

void setOffsetSync(){

    for (int i=1; i<=NUM_MOTORS; i++){
        
        char message[5];
        message[0] = 7;                         // instruction byte
        message[1] = (offsets[i-1] >> 24) & 0xFF;    // high byte
        message[2] = (offsets[i-1] >> 16) & 0xFF;
        message[3] = (offsets[i-1] >> 8) & 0xFF;
        message[4] = offsets[i-1] & 0xFF;
        
    	if (write(motors[i], message, 5) != 5) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
        } else {
            rt_printf("Device %i offset position %i\n", i, offsets[i-1]);
        }
        
    }
    
    goAllSync();
}

void goAllSync(){
    char message[1];
    message[0] = 10;                         // GO! instruction byte
    
	if (write(motors[0], message, 1) != 1) {
        // i2c read failed
        rt_printf("Device %i not responding\n", 0);
    } else {
        rt_printf("Go!\n");
    }
}

void goSync(int motor){
    char message[1];
    message[0] = 10;                         // GO! instruction byte
    
	if (write(motors[motor], message, 1) != 1) {
        // i2c read failed
        rt_printf("Device %i not responding\n", 0);
    } else {
        rt_printf("Go %i!\n", motor);
    }
}

void closeI2C(){
    //close I2C
	char message[5];	//Stahp!
	message[0] = 2;     // velocity instruction
	message[1] = 0;
	message[2] = 0;
	message[3] = 0;
	message[4] = 0;
	
	for (int i = 1; i<=NUM_MOTORS; i++){
		write(motors[i], message, 5);
	}
	
	goAllSync();
	
	for (int i = 0; i<=NUM_MOTORS; i++){
		if(close(motors[i])>0){
			rt_printf("can't close device %i", i);
		}
	}
}