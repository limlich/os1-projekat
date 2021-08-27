#include "kernelev.h"
#include "os.h"
#include "pcb.h"
#include "ivt.h"
#include "lock.h"

KernelEv::KernelEv(IVTNo ivtNo)
	: ivtNo_(ivtNo), pcb_(OS::runningPCB), blocked_(false)
{
	SOFT_LOCKED(

		IVTEntry::getEntry(ivtNo)->attachEvent(this);

	);
}

KernelEv::~KernelEv()
{
	SOFT_LOCKED(

		IVTEntry::getEntry(ivtNo_)->detachEvent();

		if (blocked_) {
			pcb_->reschedule();
			pcb_->blockEvent_ = NULL;
			/* 	- it would be undefined behaviour to continue
			running KernelEv::wait() from thread unblocked above
			after the destruction of event, so we wait for thread
			to finish executing to make sure it is out of wait()
			*/
			pcb_->waitToComplete();
		}

	);
}

void KernelEv::wait()
{
	SOFT_LOCKED(

		if (OS::runningPCB == pcb_) {
			pcb_->setState(BLOCKED);
			pcb_->blockEvent_ = this;
			blocked_ = true;
			dispatch();
		}

	);
}

void KernelEv::signal()
{
	SOFT_LOCKED(

		if (blocked_) {
			pcb_->reschedule();
			pcb_->blockEvent_ = NULL;
			blocked_ = false;
		}

	);
}

void KernelEv::onPCBTermination(PCB * pcb)
{
	SOFT_LOCKED(

		if (pcb->blockEvent_) {
			pcb->blockEvent_->blocked_ = false;
			pcb->blockEvent_->pcb_ = NULL;
		}

	);
}
