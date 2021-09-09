/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    Change the position in a stream.
*/

#include <errno.h>

/*****************************************************************************

    NAME */
#include <stdio.h>

        int __posixc_fsetpos (

/*  SYNOPSIS */
        FILE            * stream,
        const fpos_t    * pos)

/*  FUNCTION
        Change the current position in a stream. This function is equivalent
        to fseek() with whence set to SEEK_SET. However, on some systems
        fpos_t may be a complex structure, so this routine may be the only
        way to portably reposition a stream.

    INPUTS
        stream - Modify this stream
        pos - The new position in the stream.

    RESULT
        0 on success and -1 on error. If an error occurred, the global
        variable errno is set.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_fgetpos()

    INTERNALS

******************************************************************************/
{
    int retval;

    if ( pos == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    retval = fseek (stream, *pos, SEEK_SET);

    return retval;
} /* __posixc_fsetpos */

