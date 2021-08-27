#include "ivt.h"
#include "lock.h"
#include <dos.h>

IVTEntry* IVTEntry::table_[IVT_SIZE] = {0};

InterruptRoutine setInterrupt(IVTNo ivtNo, InterruptRoutine newRoutine)
{
	InterruptRoutine oldRoutine;

	HARD_LOCKED(

		oldRoutine = getvect(ivtNo);
		setvect(ivtNo, newRoutine);

	);

	return oldRoutine;
}

IVTEntry::IVTEntry(IVTNo ivtNo, InterruptRoutine routine)
	: ivtNo_(ivtNo), newRoutine(routine), event_(NULL)
{
	HARD_LOCKED(

		table_[ivtNo] = this;
		oldRoutine = getvect(ivtNo_);

	);
}

IVTEntry::~IVTEntry()
{
	HARD_LOCKED(

		detachEvent();
		table_[ivtNo_] = NULL;

	);
}

void IVTEntry::attachEvent(KernelEv * event)
{
	HARD_LOCKED(

		event_ = event;
		oldRoutine = getvect(ivtNo_);
		setvect(ivtNo_, newRoutine);

	);
}

void IVTEntry::detachEvent()
{
	HARD_LOCKED(

		setvect(ivtNo_, oldRoutine);
		event_ = NULL;

	);
}
