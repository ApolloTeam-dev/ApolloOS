/*
    Copyright (C) 2025-2026, The AROS Development Team. All rights reserved.
*/

#include <proto/exec.h>
#include <aros/symbolsets.h>

#include "__posixc_intbase.h"
#include "__optionallibs.h"

int __dos64_available(struct PosixCIntBase *PosixCBase)
{
    if (PosixCBase->PosixCDOS64Base == NULL)
        PosixCBase->PosixCDOS64Base = OpenLibrary("dos64.library", 50);

    return PosixCBase->PosixCDOS64Base != NULL;
}

/* Close the optional libraries opened by this base */
static void __optionallibs_close(struct PosixCIntBase *PosixCBase)
{
    if (PosixCBase->PosixCDOS64Base) {
        CloseLibrary(PosixCBase->PosixCDOS64Base);
        PosixCBase->PosixCDOS64Base = NULL;
    }
}

ADD2CLOSELIB(__optionallibs_close, 0)
