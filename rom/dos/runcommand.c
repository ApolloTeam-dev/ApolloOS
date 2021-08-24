/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    Desc: Execute a loaded command synchronously
*/

#include <aros/debug.h>

#include <exec/memory.h>
#include <proto/exec.h>
#include <utility/tagitem.h>
#include <proto/dos.h>
#include <dos/stdio.h>

#include "dos_intern.h"

/*****************************************************************************

    NAME */
#include <proto/dos.h>

        AROS_LH4(LONG, RunCommand,

/*  SYNOPSIS */
        AROS_LHA(BPTR,   segList,   D1),
        AROS_LHA(ULONG,  stacksize, D2),
        AROS_LHA(CONST_STRPTR, argptr,    D3),
        AROS_LHA(ULONG,  argsize,   D4),

/*  LOCATION */
        struct DosLibrary *, DOSBase, 84, Dos)

/*  FUNCTION
        RunCommand() will run the command loaded in the |segList| with the
        arguments specified with a new stack of |stacksize| bytes. Note
        that the stacksize may be extended if this is required.

        The return code of the command run will be returned.

        This call will not return until the command has completed.

    INPUTS
        segList         - segment of program to run.
        stacksize       - size of the stack to use.
        argptr          - pointer to NULL-terminated arguments.
        argsize         - size of the arguments string.

    RESULT
        The return code from the program, or -1 if the command could not be
        started (e.g. no memory for the stack). See also IoErr().

    NOTES
        Programs expect the argument string to end with a newline ('\n')
        character (ReadArgs() requires it to work properly).

    EXAMPLE

    BUGS

    SEE ALSO
        SystemTagList()

    INTERNALS

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    /* Get pointer to process structure */
    struct Process *me=(struct Process *)FindTask(NULL);
    volatile APTR oldReturnAddr;
    STRPTR oldargs;
    struct StackSwapStruct sss;
    struct StackSwapArgs args;
    UBYTE *stack;
    LONG ret;
#if !(AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
    IPTR elfinfo = 0;
    struct TagItem segtags[2] =
    {
        { GSLI_ElfHandle,       (IPTR)&elfinfo  },
        { TAG_DONE,             0               }
    };
    BOOL archsuitable = FALSE;
#endif
    D(BOOL injected;)

    ASSERT_VALID_PROCESS(me);

#if !(AROS_FLAVOUR & AROS_FLAVOUR_BINCOMPAT)
    D(bug("[DOS] %s: seglist @ 0x%p\n", __func__, segList);)
    if (GetSegListInfo(segList, segtags))
    {
        D(bug("[DOS] %s: elfinfo == 0x%p\n", __func__, elfinfo);)
        if (elfinfo)
            archsuitable = TRUE;
    }

    if (!archsuitable)
    {
#if (0)
        /* TODO: report failure reason? */
        me->pr_Result2 = ;
#endif
        return -1;
    }
#endif

    if(stacksize < AROS_STACKSIZE)
        stacksize = AROS_STACKSIZE;

    stack=(UBYTE *)AllocMem(stacksize,MEMF_ANY);
    if(stack==NULL)
        return -1;

    sss.stk_Lower=stack;
    sss.stk_Upper=stack+stacksize;
    sss.stk_Pointer = sss.stk_Upper;

    oldargs=me->pr_Arguments;
    me->pr_Arguments=(STRPTR)argptr;

    /*
     * Inject command arguments to the beginning of input handle. Guru Book mentions this.
     * This fixes for example AmigaOS' C:Execute
     */
    D(bug("RunCommand: segList @%p I=0x%p O=%p Args='%*s' Argsize=%u\n", BADDR(segList), Input(), Output(), argsize, argptr, argsize));
    D(injected = ) vbuf_inject(Input(), argptr, argsize, DOSBase);
    D(bug("RunCommand: Arguments %sinjected into FileHandle %p\n", injected ? "" : "not ", Input()));

    /* pr_ReturnAddr is set by CallEntry routine */
    oldReturnAddr = me->pr_ReturnAddr;

    args.Args[0] = (IPTR)argptr;
    args.Args[1] = argsize;
    args.Args[2] = (IPTR)BADDR(segList) + sizeof(BPTR);
    args.Args[3] = (IPTR)me;

    APTR tsid = SaveTaskStorage();
    D(bug("RunCommand: b4 StackSwap() tsid=%x, thistask->TaskStorage=%x\n",
          tsid, FindTask(NULL)->tc_UnionETask.tc_ETask->et_TaskStorage
    ));

    ret = NewStackSwap(&sss, CallEntry, &args);

    D(bug("RunCommand: after StackSwap() tsid=%x, thistask->TaskStorage=%x\n",
          tsid, FindTask(NULL)->tc_UnionETask.tc_ETask->et_TaskStorage
    ));

    RestoreTaskStorage(tsid);

    D(bug("RunCommand: after RestoreTaskStorage() tsid=%x, thistask->TaskStorage=%x\n",
          tsid, FindTask(NULL)->tc_UnionETask.tc_ETask->et_TaskStorage
    ));
    
    me->pr_ReturnAddr = oldReturnAddr;
    me->pr_Arguments  = oldargs;

    /* Flush the current CLI input stream
     * NOTE: AmigaOS 3.1's C:Execute closes Input(),
     *       so we need to catch that here.
     */
    if (Input())
    {
        struct FileHandle *in = (struct FileHandle *)BADDR(Input());
        if (in->fh_Func3 != -1)
            Flush(Input());
    }

    FreeMem(stack,stacksize);
    
    return ret;
    
    AROS_LIBFUNC_EXIT
    
} /* RunCommand */
