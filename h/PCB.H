#ifndef _pcb_h_
#define _pcb_h_

#include "thread.h"
#include "kernsem.h"
#include "kernelev.h"
#include "queue.h"
#include "map.h"
#include "types.h"

enum PCBState
{
	CREATED,   /* new pcb */
	READY,     /* ready to run */
	RUNNING,   /* OS::runningPCB */
	WAITING,   /* waiting for another pcb to finish */
	BLOCKED,   /* waiting on a semaphore */
	TERMINATED /* finished */
};

class KernelSem;

typedef void (*WrapperFunction)();

#define NUM_SIGNALS 16

class PCB
{
public:
	PCB(Time timeSlice, Thread * thread);
	PCB(StackSize stackSize, Time timeSlice, Thread * thread);
	virtual ~PCB();

	Thread * thread() const { return thread_; }
	ID getId() const { return id_; }
	static PCB * getPCBById(ID id) { return (PCB*)pcbMap_.get(id); }

	PCBState getState() const { return state_; }
	void setState(PCBState newState) { state_ = newState; }

	void waitToComplete();
	void reschedule();

	// unlist a terminated pcb
	static void onPCBTermination(PCB * pcb);

	void signal(SignalId signal);
	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId id);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);
	void blockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	void unblockSignal(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);

	void handleSignals();
	static void signalHandler0();

protected:
	void initStack(WrapperFunction wrapperFunc);
	void destroyStack();

private:
	friend class Context;
	friend class KernelSem;
	friend class KernelEv;

	void timerStart() { timeLeft_ = timeSlice_; }
	void timerTick() { if (timeLeft_ > 0) --timeLeft_; }
	Bool timedOut() const { return timeSlice_ && !timeLeft_; }

	void rescheduleWaiting();

	static void defaultWrapperFunction();

	Thread * thread_;
	PCB * parent_;
	StackSize stackSize_;
	Time timeSlice_;
	Word * stack_;
	volatile Word ss_, sp_, bp_, softLock_;
	PCBState state_;
	Time timeLeft_;

	// Wait/Block data - for managing blocked PCBs and those terminated with signal(0)
	PCB * blockPCB_;
	KernelEv * blockEvent_;
	KernelSem * blockSem_;
	Time unblockTime_;
	Bool blockTimedOut_;

	// Waiting PCBs
	Queue waiting_;

	// Signal data
	struct Signal
	{
		Signal() : blocked(false) {}

		Queue handlers;
		Bool blocked;
	};
	Signal signals_[NUM_SIGNALS];
	static Bool globalSignalBlocked_[NUM_SIGNALS];
	Queue receivedSignals_;

	ID id_;
	static ID nextID_;
	static Map pcbMap_;
};

void signalHandler0();

#endif
