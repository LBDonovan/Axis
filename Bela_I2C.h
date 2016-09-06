#ifndef BELA_I2C_H_INC
#define BELA_I2C_H_INC

#include <Bela.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <vector>
#include <queue>
#include <initializer_list>

#define MAX_I2C_ADDRESS 127

struct Bela_I2C_Message{
    Bela_I2C_Message():
        address(0),
        buffer(NULL),
        bufferLength(0){};
    Bela_I2C_Message(int address, int bufferLength):
    	address(address),
    	bufferLength(bufferLength){
    		buffer.resize(bufferLength);
    	}
    Bela_I2C_Message(int address, char* buffer, int bufferLength):
        address(address), 
        buffer(buffer, buffer + sizeof buffer / sizeof buffer[0]), 
        bufferLength(bufferLength){};
    int address;
    std::vector<char> buffer;
    int bufferLength;
};

class Bela_I2C{
    public:
        Bela_I2C(){}
        void setup(int bus=1);
        void addSlave(int address);
        void write(int address, char* buffer, int bufferSize);
        void write(int address, std::initializer_list<char> buffer);
        void writeNow(int address, char* buffer, int bufferSize);
        void request(int address, int bufferSize);
        std::vector<char> requestNow(int address, int bufferSize);
        bool messageWaiting();
        Bela_I2C_Message popMessage();
        void cleanup();
        void onReceive(void (*receiveCallback)(int address, std::vector<char> buffer));
        
    private:
        char bus[20];
    	int devices[MAX_I2C_ADDRESS];
        std::queue<Bela_I2C_Message> outQueue;
        std::queue<Bela_I2C_Message> readQueue;
        std::queue<Bela_I2C_Message> inQueue;
        
        void (*receiveCallback)(int address, std::vector<char> buffer);
        
        AuxiliaryTask Bela_I2C_Write_Task;
        static void sendQueue(void*);
        AuxiliaryTask Bela_I2C_Read_Task;
        static void receiveQueue(void*);
        
        
};

#endif