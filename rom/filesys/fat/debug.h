/*
    Copyright (C) 1995-2015, The AROS Development Team. All rights reserved.
*/

//#ifdef __AROS__
//#include <aros/debug.h>
//#else
//#if DEBUG > 0
//#include <clib/debug_protos.h>

#if DEBUG
#define D(x) x
#define bug kprintf
#else
#define D(x) 
#endif

/*/static inline void RawPutChars(const UBYTE *string, int len)
{
    while (len--)
        kputc(*string++);
}

#else
#define D(x)
#endif
#if DEBUG > 1
#define DB2(x) x
#else
#define DB2(x)
#endif
#endif*/
