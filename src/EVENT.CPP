#include "event.h"
#include "kernelev.h"
#include "lock.h"

Event::Event(IVTNo ivtNo)
{
	SOFT_LOCKED(

		myImpl = new KernelEv(ivtNo);

	);
}

Event::~Event()
{
	SOFT_LOCKED(

		delete myImpl;

	);
}

void Event::wait()
{
	myImpl->wait();
}

void Event::signal()
{
	myImpl->signal();
}
