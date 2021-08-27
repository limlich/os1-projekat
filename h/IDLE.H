#ifndef _idle_h_
#define _idle_h_

#include "pcb.h"
#include "types.h"

class IdlePCB : public PCB
{
public:
	IdlePCB()
		: PCB(defaultStackSize, 1u, NULL)
	{
		initStack(idleWrapperFunction);
		setState(READY);
	}

private:
	static void idleWrapperFunction();
};

#endif
