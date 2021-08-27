#ifndef _context_h_
#define _context_h_

#include "types.h"

class Context
{
public:
	// Switch context with signal handling
	static void switchContextSig();
	// Switch context without signal handling
	static void switchContextNoSig();

private:
	friend class OS;

	static void init();
	static void restore();

	static void interrupt timerRoutine(...);
	static InterruptRoutine oldTimerRoutine;

	static volatile Bool requestedChange_;
};

extern void tick();

#endif
