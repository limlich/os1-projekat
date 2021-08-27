#ifndef _types_h_
#define _types_h_

#include <stddef.h>

typedef int Bool;
#define true 1
#define false 0

typedef unsigned int Word;

typedef unsigned int Time; // time, x 55ms

typedef unsigned char IVTNo;
typedef void interrupt (*InterruptRoutine)(...);

#endif
