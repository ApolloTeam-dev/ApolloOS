/*
    Copyright (C) 2026, The AROS Development Team. All rights reserved.

    Internal 64-bit file access helpers for posixc.library: use
    dos64.library when available, otherwise delegate to the 32-bit
    dos.library calls, widening/narrowing as needed.
*/

#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#define __NOBLIBBASE__

#include "__posixc_intbase.h"
#include "__optionallibs.h"
#include "__dos64.h"

#include <proto/dos64.h>

/* Returns the dos64.library base, or NULL when it is not available */
static struct Library *__dos64_base(void)
{
    struct PosixCIntBase *PosixCBase =
        (struct PosixCIntBase *)__aros_getbase_PosixCBase();

    if (__dos64_available(PosixCBase))
        return PosixCBase->PosixCDOS64Base;

    return NULL;
}

/* The dos64.library base when the fcb's filesystem handles 64-bit
   packets, NULL otherwise (32-bit path) */
static struct Library *__dos64_base_for(fcb *fcb)
{
    if (!(fcb->privflags & _FCB_FS64))
        return NULL;

    return __dos64_base();
}

void __dos64_probe(fcb *fcb)
{
    struct Library *DOS64Base = __dos64_base();

    if (DOS64Base != NULL && fcb->handle != BNULL
        && IsFileSystem64(fcb->handle))
        fcb->privflags |= _FCB_FS64;
}

QUAD __dos64_seek(fcb *fcb, QUAD position, LONG mode)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return Seek64(fh, mode, position);

    if (position != (QUAD)(LONG)position)
    {
        SetIoErr(ERROR_OBJECT_TOO_LARGE);
        return -1;
    }
    return Seek(fh, (LONG)position, mode);
}

QUAD __dos64_getpos(fcb *fcb)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return GetFilePosition64(fh);

    return Seek(fh, 0, OFFSET_CURRENT);
}

QUAD __dos64_getsize(fcb *fcb)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return GetFileSize64(fh);

    /*
     * Seek() returns the previous position: seek to the end to learn
     * nothing yet, then back to where we were - that second call
     * reports the end position, i.e. the size.
     */
    QUAD pos = Seek(fh, 0, OFFSET_END);
    if (pos == -1)
        return -1;
    return Seek(fh, (LONG)pos, OFFSET_BEGINNING);
}

QUAD __dos64_setfilesize(fcb *fcb, QUAD size, LONG mode)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return SetFileSize64(fh, mode, size);

    if (size != (QUAD)(LONG)size)
    {
        SetIoErr(ERROR_OBJECT_TOO_LARGE);
        return -1;
    }
    return SetFileSize(fh, (LONG)size, mode);
}

QUAD __dos64_read(fcb *fcb, APTR buf, QUAD len)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return Read64(fh, buf, len);

    /* Partial transfers are permitted, so simply clamp */
    if (len > 0x7FFFF000LL)
        len = 0x7FFFF000LL;
    return Read(fh, buf, (LONG)len);
}

QUAD __dos64_write(fcb *fcb, CONST_APTR buf, QUAD len)
{
    struct Library *DOS64Base = __dos64_base_for(fcb);
    BPTR fh = fcb->handle;

    if (DOS64Base != NULL)
        return Write64(fh, buf, len);

    if (len > 0x7FFFF000LL)
        len = 0x7FFFF000LL;
    return Write(fh, buf, (LONG)len);
}

/* Derive the 64-bit fields of a FileInfoBlock filled by a 32-bit examine */
static void __dos64_fib32to64(struct FileInfoBlock *fib)
{
    UQUAD size = (UQUAD)(ULONG)fib->fib_Size;

    fib->fib_Size64      = size;
    fib->fib_NumBlocks64 = (UQUAD)(ULONG)fib->fib_NumBlocks;
    fib->fib_ActExtFlags = 0;
    if (size > 0x7FFFFFFFULL)
        fib->fib_Size = 0;
}

LONG __dos64_examine(BPTR obj, struct FileInfoBlock *fib, int what)
{
    struct Library *DOS64Base = __dos64_base();
    LONG ret = DOSFALSE;

    if (DOS64Base != NULL)
    {
        switch (what)
        {
        case __DOS64_EXAMINE:
            return Examine64(obj, fib, NULL);
        case __DOS64_EXAMINE_FH:
            return ExamineFH64(obj, fib, NULL);
        case __DOS64_EXAMINE_NEXT:
            return ExNext64(obj, fib, NULL);
        }
        return DOSFALSE;
    }

    /* The 64-bit fields share the block, so examine into it directly */
    switch (what)
    {
    case __DOS64_EXAMINE:
        ret = Examine(obj, fib);
        break;
    case __DOS64_EXAMINE_FH:
        ret = ExamineFH(obj, fib);
        break;
    case __DOS64_EXAMINE_NEXT:
        ret = ExNext(obj, fib);
        break;
    }

    if (ret)
        __dos64_fib32to64(fib);

    return ret;
}

LONG __dos64_info(BPTR lock, struct InfoData64 *id)
{
    struct Library *DOS64Base = __dos64_base();
    LONG ret;

    if (DOS64Base != NULL)
        return Info64(lock, id);

    /* InfoData64 starts with a standard InfoData */
    ret = Info(lock, (struct InfoData *)id);
    if (ret)
    {
        id->id_NumBlocks64     = (UQUAD)(ULONG)id->id_NumBlocks;
        id->id_NumBlocksUsed64 = (UQUAD)(ULONG)id->id_NumBlocksUsed;
    }
    return ret;
}
