/***** Bela_I2C.cpp *****/

#include<Bela_I2C.h>

void Bela_I2C::sendQueue(void* ptr){
    Bela_I2C *instance = (Bela_I2C*)ptr;
    while(!instance->outQueue.empty()){
        Bela_I2C_Message mes = instance->outQueue.front();
        // printf("writetask: sending instruction %i to address %i\n", mes.buffer[0], mes.address);
        int ret = ::write(instance->devices[mes.address], &mes.buffer[0], mes.bufferLength);
        if (ret != mes.bufferLength) {
            printf("Unable to send message to device %i with error %i\n", mes.address, ret);
        } else {
            //printf("sent: %i\n", ret);
        }
        instance->outQueue.pop();
    }
}
void Bela_I2C::receiveQueue(void* ptr){
    Bela_I2C *instance = (Bela_I2C*)ptr;
    while(!instance->readQueue.empty()){
    	//rt_printf("readQueue start, %i\n", instance->readQueue.size());
        Bela_I2C_Message mes = instance->readQueue.front();
        //rt_printf("requesting: %i bytes from %i\n", mes.bufferLength, mes.address);
        int ret = read(instance->devices[mes.address], &mes.buffer[0], mes.bufferLength);
    	if (ret != mes.bufferLength) {
            // i2c read failed
            rt_printf("Request failed with code: %i\n", ret);
        } else {
            //rt_printf("received: %i bytes from %i, byte 0: %i\n", mes.bufferLength, mes.address, (int)mes.buffer[0]);
            instance->receiveCallback(mes.address, mes.buffer);
        }
        instance->readQueue.pop();
        //rt_printf("readQueue end, %i\n", instance->readQueue.size());
    }
    //rt_printf("finished task\n");
}

void Bela_I2C::setup(int busNum){
    
    if (busNum <= 0 || busNum >= 3){
        fprintf(stderr, "I2C bus must be 0-2. Defaulting to /dev/i2c-1\n");
        busNum = 1;
    }
    
    sprintf (bus, "/dev/i2c-%i", busNum);
    
    // setup broadcast at address 0
    if ((devices[0] = open(bus, 02)) < 0){
		fprintf(stderr, "Unable to open I2C bus %s\n", bus);
		return;
	}
	
	if (ioctl(devices[0], I2C_SLAVE, 0) < 0){
		fprintf(stderr, "Unable to create broadcast device\n");
	}

    Bela_I2C_Write_Task = Bela_createAuxiliaryTask(sendQueue, BELA_AUDIO_PRIORITY-4, "Bela_I2C_Write_Task", this);
    Bela_I2C_Read_Task = Bela_createAuxiliaryTask(receiveQueue, BELA_AUDIO_PRIORITY-5, "Bela_I2C_Read_Task", this);

}

void Bela_I2C::addSlave(int address){
    
    if (address <= 0 || address >= MAX_I2C_ADDRESS){
        fprintf(stderr, "Incorrect I2C slave address, must be in range 1-127\n");
        return;
    }
    
    if ((devices[address] = open(bus, 02)) < 0){
		fprintf(stderr, "Unable to open I2C bus %s\n", bus);
		return;
	}
	
	if (ioctl(devices[address], I2C_SLAVE, address) < 0){
		fprintf(stderr, "Unable to find device %i\n", address);
	}
    
}

void Bela_I2C::write(int address, char* buffer, int bufferLength){
    Bela_I2C_Message *mes = new Bela_I2C_Message(address, buffer, bufferLength);
    outQueue.push(*mes);
    Bela_scheduleAuxiliaryTask(Bela_I2C_Write_Task);
}
void Bela_I2C::write(int address, std::initializer_list<char> buffer){
	Bela_I2C_Message *mes = new Bela_I2C_Message(address, buffer.size());
	mes->buffer = buffer;
    outQueue.push(*mes);
    Bela_scheduleAuxiliaryTask(Bela_I2C_Write_Task);
}
void Bela_I2C::writeNow(int address, char* buffer, int bufferLength){
    if (::write(devices[address], buffer, bufferLength) != bufferLength) {
        fprintf(stderr, "Unable to send message to device %i\n", address);
    }
}

void Bela_I2C::request(int address, int numBytes){
    Bela_I2C_Message *mes = new Bela_I2C_Message(address, numBytes);
    readQueue.push(*mes);
    Bela_scheduleAuxiliaryTask(Bela_I2C_Read_Task);
}
std::vector<char> Bela_I2C::requestNow(int address, int numBytes){
    Bela_I2C_Message *mes = new Bela_I2C_Message(address, numBytes);
    int ret = read(devices[mes->address], &mes->buffer[0], mes->bufferLength);
	if (ret != mes->bufferLength) {
        // i2c read failed
        rt_printf("Request failed with code: %i\n", ret);
    } else {
        //rt_printf("received: %i bytes from %i\n", mes->bufferLength, mes->address);
    }
    return mes->buffer;
}

void Bela_I2C::onReceive(void (*_receiveCallback)(int address, std::vector<char> buffer)){
	receiveCallback = _receiveCallback;
}

bool Bela_I2C::messageWaiting(){
    return !inQueue.empty();
}

Bela_I2C_Message Bela_I2C::popMessage(){
    
    Bela_I2C_Message poppedMessage;
    
    if (!inQueue.empty()){
        poppedMessage = inQueue.front();
        inQueue.pop();
    }
    
    return poppedMessage;
}

void Bela_I2C::cleanup(){
    for (int i = 0; i<=MAX_I2C_ADDRESS; i++){
		if(close(devices[i])>0){
			rt_printf("can't close device %i", i);
		}
	}
}