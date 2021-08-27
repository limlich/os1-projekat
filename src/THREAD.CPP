#include "thread.h"
#include "pcb.h"
#include "context.h"
#include "os.h"
#include "lock.h"

Thread::Thread(StackSize stackSize, Time timeSlice)
{
	SOFT_LOCKED(

		myPCB = new PCB(stackSize, timeSlice, this);

	);
}

Thread::~Thread()
{
	waitToComplete();

	SOFT_LOCKED(

		delete myPCB;

	);
}

void Thread::start()
{
	myPCB->reschedule();
}

void Thread::waitToComplete()
{
	myPCB->waitToComplete();
}

ID Thread::getId()
{
	return myPCB->getId();
}

ID Thread::getRunningId()
{
	return OS::runningPCB->getId();
}

Thread * Thread::getThreadById(ID id)
{
	return PCB::getPCBById(id)->thread();
}

void Thread::signal(SignalId signal)
{
	myPCB->signal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler)
{
	myPCB->registerHandler(signal, handler);
}

void Thread::unregisterAllHandlers(SignalId id)
{
	myPCB->unregisterAllHandlers(id);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2)
{
	myPCB->swap(id, hand1, hand2);
}

void Thread::blockSignal(SignalId signal)
{
	myPCB->blockSignal(signal);
}

void Thread::blockSignalGlobally(SignalId signal)
{
	PCB::blockSignalGlobally(signal);
}

void Thread::unblockSignal(SignalId signal)
{
	myPCB->unblockSignal(signal);
}

void Thread::unblockSignalGlobally(SignalId signal)
{
	PCB::unblockSignalGlobally(signal);
}

void dispatch()
{
	Context::switchContextSig();
}
