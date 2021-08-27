#ifndef _main_h_
#define _main_h_

#include "pcb.h"

class MainPCB : public PCB
{
public:
	MainPCB()
		: PCB(defaultTimeSlice, NULL)
	{
		setState(RUNNING);
	}
};

#endif
