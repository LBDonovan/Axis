#include <Bela.h>
#include <Motor.h>

Axis axis;

int gState = 0;
int requestCount = 0;

bool setup(BelaContext *context, void *userData)
{
	return axis.setup();
}

void render(BelaContext *context, void *userData)
{
	
	for (unsigned int n=0; n<context->audioFrames; n++){
		if (requestCount++ > 4410){
			requestCount = 0;
			axis.requestAll();
		}
		if (gState == 0){
			if(axis.home()){
				rt_printf("HOME!\n");
				gState += 1;
			}
		}
	}
} 

void cleanup(BelaContext *context, void *userData)
{
	axis.cleanup();
}