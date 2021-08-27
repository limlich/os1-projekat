#include "kernsem.h"
#include "pcb.h"
#include "os.h"
#include "lock.h"

Bool KernelSem::cmpFunc(const void * l, const void * r)
{
	return ((const PCB*)l)->unblockTime_ <= ((const PCB*)r)->unblockTime_;
}

SortedList KernelSem::blockedWaiting_(KernelSem::cmpFunc);
Time KernelSem::elapsedTime_ = 0;

KernelSem::~KernelSem()
{
	SOFT_LOCKED(

		PCB * bPcb = NULL;

		blocked_.iterReset();

		while ((bPcb = (PCB*)blocked_.iterGet()) != NULL) {
			if (bPcb->unblockTime_)
				blockedWaiting_.remove(bPcb);

			bPcb->reschedule();
			blocked_.iterNext();
		}

		/* 	- it would be undefined behaviour to continue
			running KernelSem::wait() from threads unblocked above
			after the destruction of the semaphore, so we wait for threads
			to finish executing to make sure they are out of wait()
		*/
		while ((bPcb = (PCB*)blocked_.popFront()) != NULL) {
			bPcb->blockSem_ = NULL; /* meanwhile, other threads may leave the list if destroyed */
			bPcb->waitToComplete();
		}

	);
}

int KernelSem::wait(Time maxTimeToWait)
{
	SOFT_LOCKED(

		int retVal = 1;

		if (--value_ < 0) {
			OS::runningPCB->setState(BLOCKED);
			OS::runningPCB->blockSem_ = this;
			OS::runningPCB->blockTimedOut_ = false;
			OS::runningPCB->unblockTime_ = maxTimeToWait;

			blocked_.pushBack(OS::runningPCB);

			if (maxTimeToWait) {
				OS::runningPCB->unblockTime_ += elapsedTime_;
				blockedWaiting_.insert(OS::runningPCB);
			}

			dispatch();

			retVal = !OS::runningPCB->blockTimedOut_;
		}

	);

	return retVal;
}

int KernelSem::signal(int n)
{
	SOFT_LOCKED(

		int retVal = blocked_.size() < n ? blocked_.size() : n;

		if (n >= 0) {
			value_ += (n += !n);

			PCB * bPcb = NULL;

			while (n-- && (bPcb = (PCB*)blocked_.popFront()) != NULL) {
				if (bPcb->unblockTime_)
					blockedWaiting_.remove(bPcb);

				bPcb->reschedule();
				bPcb->blockSem_ = NULL;
			}
		}

	);

	return retVal;
}

void KernelSem::updateWaitingPCBs(Time elapsedTime)
{
	SOFT_LOCKED(

		if (blockedWaiting_.empty())
			elapsedTime_ = 0;
		else if (elapsedTime) {
			elapsedTime_ += elapsedTime;

			PCB * bPcb;

			while (!blockedWaiting_.empty() && ((PCB*)blockedWaiting_.front())->unblockTime_ <= elapsedTime_) {
				bPcb = (PCB*)blockedWaiting_.popFront();
				bPcb->blockSem_->blocked_.remove(bPcb);
				bPcb->blockSem_->value_++;
				bPcb->blockTimedOut_ = true;
				bPcb->reschedule();
				bPcb->blockSem_ = NULL;
			}
		}

	);
}

void KernelSem::onPCBTermination(PCB * pcb)
{
	SOFT_LOCKED(

		if (pcb->blockSem_) {
			pcb->blockSem_->blocked_.remove(pcb);

			if (pcb->unblockTime_)
				blockedWaiting_.remove(pcb);

			pcb->blockSem_->value_++;
		}

	);
}
