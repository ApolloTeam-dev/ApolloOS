/*
    Copyright (C) 1995-2021, The AROS Development Team. All rights reserved.

    C99 function fputs().
*/

#include <proto/dos.h>
#include <errno.h>
#include "__fdesc.h"
#include "__stdio.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

        int __posixc_fputs (

/*  SYNOPSIS */
        const char * str,
        FILE       * stream)

/*  FUNCTION
        Write a string to the specified stream.

    INPUTS
        str - Output this string...
        fh - ...to this stream

    RESULT
        > 0 on success and EOF on error.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_puts(), __posixc_fputc(), putc()

    INTERNALS

******************************************************************************/
{
    fdesc *fdesc = __getfdesc(stream->fd);

    if (!fdesc)
    {
        errno = EBADF;
        return EOF;
    }

    if (!str) str = "(null)";

    if (FPuts(fdesc->fcb->handle, str) == -1)
    {
        errno = __stdc_ioerr2errno(IoErr());
        return EOF;
    }

    return 0;
} /* fputs */

