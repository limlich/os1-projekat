#ifndef _kernsem_h_
#define _kernsem_h_

#include "types.h"
#include "queue.h"
#include "sortlist.h"

class PCB;

class KernelSem
{
public:
	KernelSem(int init) : value_(init) {}
	~KernelSem();

	int wait(Time maxTimeToWait);
	int signal(int n);
	int val() const { return value_; }

	// elapsedTime - time since the last call to this function
	static void updateWaitingPCBs(Time elapsedTime);

	// unlist a terminated pcb
	static void onPCBTermination(PCB * pcb);

private:
	static Bool cmpFunc(const void*, const void*);

	int value_;
	Queue blocked_;
	static SortedList blockedWaiting_;
	static Time elapsedTime_;
};

#endif
