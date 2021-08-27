#include "semaphor.h"
#include "kernsem.h"
#include "lock.h"

Semaphore::Semaphore(int init)
{
	SOFT_LOCKED(
		
		myImpl = new KernelSem(init);

	);
}

Semaphore::~Semaphore()
{
	SOFT_LOCKED(

		delete myImpl;

	);
}

int Semaphore::wait(Time maxTimeToWait)
{
	return myImpl->wait(maxTimeToWait);
}

int Semaphore::signal(int n)
{
	return myImpl->signal(n);
}

int Semaphore::val() const
{
	return myImpl->val();
}
