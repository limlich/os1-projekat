#ifndef _lock_h_
#define _lock_h_

#include "types.h"

extern volatile Word softLock;

#define HARD_LOCKED(x) asm { pushf; cli; } x asm popf
#define HARD_UNLOCKED(x) asm { pushf; sti; } x asm popf
#define SOFT_LOCKED(x) ++softLock; x if (softLock) --softLock

#endif
