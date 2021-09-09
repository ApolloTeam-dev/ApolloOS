/*
    Copyright (C) 1995-2021, The AROS Development Team. All rights reserved.

    Tell the position in a stream.
*/

#include <errno.h>
#include <dos/dos.h>
#include <proto/dos.h>
#include "__stdio.h"
#include "__fdesc.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

        long __posixc_ftell (

/*  SYNOPSIS */
        FILE * stream)

/*  FUNCTION
        Tell the current position in a stream.

    INPUTS
        stream - Obtain position of this stream

    RESULT
        The position on success and -1 on error.
        If an error occurred, the global variable errno is set.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_fopen(), __posixc_fseek(), __posixc_fwrite()

    INTERNALS

******************************************************************************/
{
    long cnt;
    BPTR fh;
    fdesc *fdesc = __getfdesc(stream->fd);

    if (!fdesc)
    {
        errno = EBADF;
        return 0;
    }

    fh = fdesc->fcb->handle;

    Flush (fh);
    cnt = Seek (fh, 0, OFFSET_CURRENT);

    if (cnt == -1)
        errno = __stdc_ioerr2errno (IoErr ());

    return cnt;
} /* ftell */
