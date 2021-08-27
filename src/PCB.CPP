#include "pcb.h"
#include "os.h"
#include "main.h"
#include "idle.h"
#include "lock.h"
#include <dos.h>
#include <schedule.h>

Bool PCB::globalSignalBlocked_[NUM_SIGNALS] = {0};

Key getPcbKey(const void * pcb)
{
	return (Key)((const PCB*)pcb)->getId();
}

ID PCB::nextID_ = 0;
Map PCB::pcbMap_(getPcbKey);

PCB::PCB(Time timeSlice, Thread * thread)
	: thread_(thread), parent_((PCB*)OS::runningPCB), stackSize_(0), timeSlice_(timeSlice),
	  stack_(NULL), state_(CREATED), timeLeft_(timeSlice), softLock_(0),
	  blockPCB_(NULL), blockSem_(NULL), blockEvent_(NULL),
	  id_(nextID_++)
{
	SOFT_LOCKED(

		/* copy signal settings from parent thread */
		if (parent_)
			for (int i = 0; i < NUM_SIGNALS; ++i) {
				SignalHandler * tmp;
				parent_->signals_[i].handlers.iterReset();
				while ((tmp = (SignalHandler*)parent_->signals_[i].handlers.iterGet()) != NULL) {
					signals_[i].handlers.pushBack(new SignalHandler(*tmp));
					parent_->signals_[i].handlers.iterNext();
				}
				signals_[i].blocked = parent_->signals_[i].blocked;
			}
		pcbMap_.put(this);

	);
}

PCB::PCB(StackSize stackSize, Time timeSlice, Thread * thread)
	: thread_(thread), parent_((PCB*)OS::runningPCB), stackSize_(stackSize), timeSlice_(timeSlice),
	  stack_(NULL), state_(CREATED), timeLeft_(timeSlice), softLock_(0),
	  blockPCB_(NULL), blockSem_(NULL), blockEvent_(NULL),
	  id_(nextID_++)
{
	SOFT_LOCKED(

		/* copy signal settings from parent thread */
		if (OS::runningPCB)
			for (int i = 0; i < NUM_SIGNALS; ++i) {
				SignalHandler * tmp;
				OS::runningPCB->signals_[i].handlers.iterReset();
				while ((tmp = (SignalHandler*)OS::runningPCB->signals_[i].handlers.iterGet()) != NULL) {
					signals_[i].handlers.pushBack(new SignalHandler(*tmp));
					OS::runningPCB->signals_[i].handlers.iterNext();
				}
				signals_[i].blocked = OS::runningPCB->signals_[i].blocked;
			}
		pcbMap_.put(this);
		initStack(defaultWrapperFunction);

	);
}

PCB::~PCB()
{
	SOFT_LOCKED(

		for (int i = 0; i < NUM_SIGNALS; ++i)
			unregisterAllHandlers(i);
		while (!receivedSignals_.empty())
			delete (SignalId*)receivedSignals_.popFront();

		pcbMap_.remove(id_);
		destroyStack();

	);
}

void PCB::initStack(WrapperFunction wrapperFunc)
{
	SOFT_LOCKED(

		size_t stackCount = (size_t)stackSize_ / sizeof(Word);

		if (!stack_)
			stack_ = new Word[stackCount];

		stack_[stackCount - 1u] = (Word)(1u << 9); /* PSWI = 1 */
		stack_[stackCount - 2u] = (Word)FP_SEG(wrapperFunc);
		stack_[stackCount - 3u] = (Word)FP_OFF(wrapperFunc);

		ss_ = (Word)FP_SEG(stack_ + stackCount - 12u);
		sp_ = (Word)FP_OFF(stack_ + stackCount - 12u);
		bp_ = sp_;

	);
}

void PCB::destroyStack()
{
	SOFT_LOCKED(

		if (stack_) {
			delete[] stack_;
			stack_ = NULL;
		}

	);
}

void PCB::waitToComplete()
{
	SOFT_LOCKED(

		if (OS::runningPCB != this && OS::idlePCB != this && OS::mainPCB != this &&
			getState() != CREATED && getState() != TERMINATED) {
			OS::runningPCB->setState(WAITING);
			waiting_.pushBack(OS::runningPCB);
			OS::runningPCB->blockPCB_ = this;
			dispatch();
		}

	);
}

void PCB::reschedule()
{
	SOFT_LOCKED(

		setState(READY);
		Scheduler::put(this);

	);
}

void PCB::onPCBTermination(PCB * pcb)
{
	SOFT_LOCKED(

		if (pcb->blockPCB_)
			pcb->blockPCB_->waiting_.remove(pcb);

	);
}

void PCB::rescheduleWaiting()
{
	SOFT_LOCKED(

		PCB * pcb;

		while ((pcb = (PCB*)waiting_.popFront()) != NULL) {
			pcb->reschedule();
			pcb->blockPCB_ = NULL;
		}

	);
}

void PCB::defaultWrapperFunction()
{
	OS::runningPCB->thread_->run();

	SOFT_LOCKED(
		OS::runningPCB->signal(0);
		OS::runningPCB->unblockSignal(0);
		dispatch();
	);
}

void PCB::signal(SignalId signal)
{
	SOFT_LOCKED(

		receivedSignals_.pushBack(new SignalId(signal));

	);
}

void PCB::registerHandler(SignalId signal, SignalHandler handler)
{
	SOFT_LOCKED(

		signals_[signal].handlers.pushBack(new SignalHandler(handler));

	);
}

void PCB::unregisterAllHandlers(SignalId id)
{
	SOFT_LOCKED(

		SignalHandler * handler;

		while ((handler = (SignalHandler*)signals_[id].handlers.popFront()) != NULL)
			delete handler;

	);
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2)
{
	SOFT_LOCKED(

		SignalHandler * h1 = NULL;
		SignalHandler * h2 = NULL;

		Signal * signal = &signals_[id];
		SignalHandler * tmp;

		signal->handlers.iterReset();
		while ((tmp = (SignalHandler*)signal->handlers.iterGet()) != NULL) {
			if (*tmp == hand1)
				h1 = tmp;
			else if (*tmp == hand2)
				h2 = tmp;
			if (h1 && h2)
				break;

			signal->handlers.iterNext();
		}

		if (h1 && h2) {
			*h1 = hand2;
			*h2 = hand1;
		}

	);
}

void PCB::blockSignal(SignalId signal)
{
	SOFT_LOCKED(

		signals_[signal].blocked = true;

	);
}

void PCB::blockSignalGlobally(SignalId signal)
{
	SOFT_LOCKED(

		globalSignalBlocked_[signal] = true;

	);
}

void PCB::unblockSignal(SignalId signal)
{
	SOFT_LOCKED(

		signals_[signal].blocked = false;

	);
}

void PCB::unblockSignalGlobally(SignalId signal)
{
	SOFT_LOCKED(

		globalSignalBlocked_[signal] = false;

	);
}

void PCB::handleSignals()
{
	if (receivedSignals_.empty())
		return;

	SignalId * sigId;

	receivedSignals_.iterReset();
	while ((sigId = (SignalId*)receivedSignals_.iterGet()) != NULL) {
		Signal * signal = &signals_[*sigId];

		if (!signal->blocked && !globalSignalBlocked_[*sigId]) {
			SignalHandler * handler;

			SOFT_LOCKED(
			HARD_UNLOCKED(

				signal->handlers.iterReset();
				while ((handler = (SignalHandler*)signal->handlers.iterGet()) != NULL) {
					(*handler)();
					signal->handlers.iterNext();
				}

			);
			);

			receivedSignals_.remove(sigId);
			delete sigId;
		} else
			receivedSignals_.iterNext();
	}
}

void PCB::signalHandler0()
{
	if (OS::runningPCB->getState() != TERMINATED) {
		OS::runningPCB->setState(TERMINATED);
		OS::runningPCB->rescheduleWaiting();
		OS::runningPCB->parent_->signal(1);
	   	OS::runningPCB->signal(2);
		PCB::onPCBTermination(OS::runningPCB);
		KernelSem::onPCBTermination(OS::runningPCB);
		KernelEv::onPCBTermination(OS::runningPCB);
	}
}
