#ifndef _ivt_h_
#define _ivt_h_

#include "kernelev.h"
#include "lock.h"
#include "types.h"

#define IVT_SIZE 256

#define IVTNO_TIMER 0x08

// returns old interrupt routine
InterruptRoutine setInterrupt(IVTNo ivtNo, InterruptRoutine newRoutine);

class IVTEntry
{
public:
	IVTEntry(IVTNo ivtNo, InterruptRoutine routine);
	~IVTEntry();

	void attachEvent(KernelEv * event);
	void detachEvent();

	void signal() { event_->signal(); }

	static IVTEntry * getEntry(IVTNo ivtNo) { return table_[ivtNo]; }

	InterruptRoutine oldRoutine;

private:
	IVTNo ivtNo_;
	KernelEv * event_;

	InterruptRoutine newRoutine;

	static IVTEntry* table_[IVT_SIZE];
};

#include "context.h"

#define PREPAREENTRY(ivtNo, callOldInt) 	\
extern IVTEntry ivtEntry##ivtNo;			\
											\
void interrupt int##ivtNo(...)          	\
{                                       	\
	ivtEntry##ivtNo.signal();           	\
											\
	if (callOldInt)                         \
		ivtEntry##ivtNo.oldRoutine();       \
											\
	if (!softLock)                          \
		Context::switchContextNoSig();		\
}                                           \
											\
IVTEntry ivtEntry##ivtNo(ivtNo, int##ivtNo);

#endif
