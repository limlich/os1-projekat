#ifndef _kernelev_h_
#define _kernelev_h_

#include "types.h"

class PCB;

class KernelEv
{
public:
	KernelEv(IVTNo ivtNo);
	~KernelEv();

	void wait();
	void signal();

	// unlist a terminated pcb
	static void onPCBTermination(PCB * pcb);

private:
	IVTNo ivtNo_;
	PCB * pcb_;
	Bool blocked_;
};

#endif
