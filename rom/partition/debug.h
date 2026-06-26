#ifndef PDEBUG_H
#define PDEBUG_H

#if APOLLO_DEBUG
#define DEBUG 0
#else
#define DEBUG 0
#endif

#if DEBUG
#define DD(x) x
#define D(x)
#define bug kprintf
#else
#define D(x)
#define DD(x)
#endif

#endif