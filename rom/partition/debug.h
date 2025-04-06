#ifndef PDEBUG_H
#define PDEBUG_H

#define DEBUG 1

#if DEBUG
#include <aros/debug.h>
#define D(x) x
#define bug kprintf
#endif
#endif