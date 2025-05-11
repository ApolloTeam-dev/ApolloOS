#ifndef PDEBUG_H
#define PDEBUG_H

#ifdef __AMIGAOS__

    #if APOLLO_DEBUG
    #define DEBUG 1
    #else
    #define DEBUG 0
    #endif

    #if DEBUG
    #define D(x) x
    #define bug kprintf
    #endif

    #define kprintf(x)
#else
    #include <aros/debug.h>
#endif

#endif