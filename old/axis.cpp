#include <Axis.h>

std::vector<I2CMessage*> I2CMessages;

extern int motors[NUM_MOTORS+1];
extern int status[NUM_MOTORS+1];
extern int microstepping;

AuxiliaryTask I2CSend;
AuxiliaryTask I2CRead;

bool sendingI2C = false;
bool readingI2C = false;

int getStatus(){
    int statusValue = 0;
    for (int i=1; i<=NUM_MOTORS; i++){
        if (status[i] != 0){
            statusValue = status[i];
        }
    }
    return statusValue;
}

void createAuxTasks(){
    I2CSend = Bela_createAuxiliaryTask(i2cSendTask, BELA_AUDIO_PRIORITY-1, "AxisI2CSendTask");
    I2CRead = Bela_createAuxiliaryTask(i2cReadTask, BELA_AUDIO_PRIORITY-2, "AxisI2CReadTask");
}

void scheduleTasks(){
    /*if (!sendingI2C){
        sendingI2C = true;
        Bela_scheduleAuxiliaryTask(I2CSend);
    } else {
        //rt_printf("Can't send I2C, aux task is busy\n");
    }*/
    if (!readingI2C){
        readingI2C = true;
        Bela_scheduleAuxiliaryTask(I2CRead);
    } else {
        //rt_printf("Can't read I2C, aux task is busy\n");
    }
}

void i2cSendTask(){
    for (unsigned int i=0; i<I2CMessages.size(); i++){
        if (write(motors[I2CMessages[i]->address], &(I2CMessages[i]->buffer[0]), I2CMessages[i]->buffer.size()) != (int)I2CMessages[i]->buffer.size()) {
            // i2c write failed
            rt_printf("Device %i not responding\n", i);
        }
        usleep(100);
    }
    I2CMessages.clear();
    sendingI2C = false;
}

void i2cReadTask(){
    char buf[1];
    for (int i=1; i<=NUM_MOTORS; i++){
        if (read(motors[i], buf, 1) != 1) {
            // i2c read failed
            rt_printf("Device %i not responding\n", i);
            status[i] = -1;
        } else {
            status[i] = (int)buf[0];
        }
    }
    readingI2C = false;
    //rt_printf("status read\n");
}



void setConstantVelocity(int vel, bool alternating){
    
    if (!sendingI2C){
        sendingI2C = true;
        Bela_scheduleAuxiliaryTask(I2CSend);
    } else {
        //rt_printf("busy\n");
        return;
    }
    
    int sign = -1;
    int velocity = vel;
    
    for (int i=1; i<=NUM_MOTORS; i++){
        
        if (alternating){
            sign *= (-1);
            velocity = vel*sign;
        }
        
        I2CMessage* msg = new I2CMessage(i);
        
        msg->buffer.push_back(2);                         // instruction byte
        msg->buffer.push_back((velocity >> 8) & 0xFF);    // high byte
        msg->buffer.push_back(velocity & 0xFF);           // low byte
        
        I2CMessages.push_back(msg);
        
        //rt_printf("motor %i velocity set to %i\n", i, velocity);
        
    }
    
    //go(0);

}

void setPosition(int motor, int position){
    
    I2CMessage* msg = new I2CMessage(motor);
        
    msg->buffer.push_back(1);                         // instruction byte
    msg->buffer.push_back((position >> 8) & 0xFF);    // high byte
    msg->buffer.push_back(position & 0xFF);           // low byte
    
    I2CMessages.push_back(msg);
    
    //rt_printf("motor %i position set to %i\n", motor, position);
    
}

void setVelocity(int motor, int velocity){
    
    I2CMessage* msg = new I2CMessage(motor);
        
    msg->buffer.push_back(2);                         // instruction byte
    msg->buffer.push_back((velocity >> 8) & 0xFF);    // high byte
    msg->buffer.push_back(velocity & 0xFF);           // low byte
    
    I2CMessages.push_back(msg);
    
    //rt_printf("motor %i velocity set to %i\n", motor, velocity);
    
}

void setMaxVelocity(int motor, int velocity){
    
    I2CMessage* msg = new I2CMessage(motor);
        
    msg->buffer.push_back(3);                         // instruction byte
    msg->buffer.push_back((velocity >> 8) & 0xFF);    // high byte
    msg->buffer.push_back(velocity & 0xFF);           // low byte
    
    I2CMessages.push_back(msg);
    
}

void setAcceleration(int motor, int acceleration){
    
    I2CMessage* msg = new I2CMessage(motor);
        
    msg->buffer.push_back(4);                         // instruction byte
    msg->buffer.push_back((acceleration >> 8) & 0xFF);    // high byte
    msg->buffer.push_back(acceleration & 0xFF);           // low byte
    
    I2CMessages.push_back(msg);
    
}

void setMicrostepping(int motor, int newMicroStepping){
    
    microstepping = newMicroStepping;
    
    I2CMessage* msg = new I2CMessage(motor);
        
    msg->buffer.push_back(5);                         // instruction byte
    msg->buffer.push_back(microstepping); 
    
    I2CMessages.push_back(msg);
    
}


void go(int motor){
    
    I2CMessage* msg = new I2CMessage(motor);

    msg->buffer.push_back(10);                         // instruction byte
    
    I2CMessages.push_back(msg);
    
    //rt_printf("go!\n");

}