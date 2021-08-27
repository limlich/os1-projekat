#include "context.h"
#include "os.h"
#include "pcb.h"
#include "idle.h"
#include "kernsem.h"
#include "ivt.h"
#include "lock.h"
#include <schedule.h>

#define IV_OLD_TIMER IV_SYS0

InterruptRoutine Context::oldTimerRoutine = NULL;
volatile Bool Context::requestedChange_ = false;

void Context::init()
{
	HARD_LOCKED(

		oldTimerRoutine = setInterrupt(IVTNO_TIMER, timerRoutine);

	);
}

void Context::restore()
{
	HARD_LOCKED(

		setInterrupt(IVTNO_TIMER, oldTimerRoutine);

	);
}

void Context::switchContextSig()
{
	HARD_LOCKED(

		OS::runningPCB->handleSignals(); /* Handle signals */
		requestedChange_ = true;
		timerRoutine();

	);
}

void Context::switchContextNoSig()
{
	HARD_LOCKED(

		requestedChange_ = true;
		timerRoutine();

	);
}

static volatile Word tss, tsp, tbp;
static volatile Time elapsedTime = 0;

void interrupt Context::timerRoutine(...)
{
	if (!requestedChange_) {
		oldTimerRoutine();
		tick();
		OS::runningPCB->timerTick();
		++elapsedTime;

		// Handle signals
		if (!softLock) { // Don't handle while executing system code
			OS::runningPCB->handleSignals();
			if (OS::runningPCB->getState() == TERMINATED)
				requestedChange_ = true;
		}
	}

	if (requestedChange_ || (!softLock && OS::runningPCB->timedOut())) {
		switch (OS::runningPCB->getState()) {
			case TERMINATED:
				break;

			case RUNNING:
				// Reschedule
				OS::runningPCB->setState(READY);

				if (OS::runningPCB != OS::idlePCB)
					Scheduler::put(OS::runningPCB);

			default:
				// Save context
				asm {
					mov tss, ss;
					mov tsp, sp;
					mov tbp, bp;
				}

				OS::runningPCB->ss_ = tss;
				OS::runningPCB->sp_ = tsp;
				OS::runningPCB->bp_ = tbp;
				OS::runningPCB->softLock_ = softLock;
		}

		// Unblock PCBs whose semaphore wait(int) timed out
		KernelSem::updateWaitingPCBs(elapsedTime);
		elapsedTime = 0;

		// Get next thread to execute
		OS::runningPCB = Scheduler::get();

		// Idle thread is always ready if necessary
		if (!OS::runningPCB)
			OS::runningPCB = OS::idlePCB;

		OS::runningPCB->setState(RUNNING);
		OS::runningPCB->timerStart();

		// Restore context
		tss = OS::runningPCB->ss_;
		tsp = OS::runningPCB->sp_;
		tbp = OS::runningPCB->bp_;
		asm {
			mov ss, tss;
			mov sp, tsp;
			mov bp, tbp;
		}
		softLock = OS::runningPCB->softLock_;

		requestedChange_ = false;
	}
}
