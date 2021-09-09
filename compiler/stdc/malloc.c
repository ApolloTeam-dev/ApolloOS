/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    C99 function malloc().
*/

#include "__stdc_intbase.h"

#include <errno.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <aros/symbolsets.h>
#include <aros/debug.h>

#include "debug.h"

/*****************************************************************************

    NAME */
#include <stdlib.h>

        void *malloc (

/*  SYNOPSIS */
        size_t size)

/*  FUNCTION
        Allocate size bytes of memory and return the address of the
        first byte.

    INPUTS
        size - How much memory to allocate.

    RESULT
        A pointer to the allocated memory or NULL. If you don't need the
        memory anymore, you can pass this pointer to free(). If you don't,
        the memory will be freed for you when the application exits.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        free()

    INTERNALS

******************************************************************************/
{
    struct StdCIntBase *StdCBase = (struct StdCIntBase *)__aros_getbase_StdCBase();
    UBYTE *mem = NULL;

    /* Allocate the memory */
    mem = AllocPooled (StdCBase->mempool, size + AROS_ALIGN(sizeof(size_t)));
    if (mem)
    {
        *((size_t *)mem) = size;
        mem += AROS_ALIGN(sizeof(size_t));
    }
    else
        errno = ENOMEM;

    return mem;

} /* malloc */


int __init_memstuff(struct StdCIntBase *StdCBase)
{
    D(bug("[%s] %s: task(0x%p), StdCBase(0x%p)\n", STDCNAME, __func__,
          FindTask(NULL), StdCBase
    ));

    StdCBase->mempool = CreatePool(MEMF_ANY | MEMF_SEM_PROTECTED, 65536L, 4096L);

    D(bug("[%s] %s: StdCBase->mempool(0x%p)\n", STDCNAME, __func__, StdCBase->mempool));

    if (!StdCBase->mempool)
    {
        return 0;
    }

    return 1;
}


void __exit_memstuff(struct StdCIntBase *StdCBase)
{
    D(bug("[%s] %s: task(0x%p), StdCBase(0x%p), acb_mempool(0x%p)\n", STDCNAME, __func__,
          FindTask(NULL), StdCBase, StdCBase->mempool
    ));

    if (StdCBase->mempool)
    {
        DeletePool(StdCBase->mempool);
    }
}

ADD2OPENLIB(__init_memstuff, 0);
ADD2CLOSELIB(__exit_memstuff, 0);
