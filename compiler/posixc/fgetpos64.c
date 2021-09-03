/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    Get the position in a stream.
*/

#include <errno.h>

/*****************************************************************************

    NAME */
#include <stdio.h>

        int fgetpos64 (

/*  SYNOPSIS */
        FILE   * stream,
        __fpos64_t * pos)

/*  FUNCTION
        Get the current position in a stream. This function is equivalent
        to ftell(). However, on some systems fpos_t may be a complex
        structure, so this routine may be the only way to portably
        get the position of a stream.

    INPUTS
        stream - The stream to get the position from.
        pos - Pointer to the fpos_t position structure to fill.

    RESULT
        0 on success and -1 on error. If an error occurred, the global
        variable errno is set.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_fsetpos()

    INTERNALS

******************************************************************************/
{
    if ( pos == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    *pos = ftell (stream);

    if ( *pos < 0L )
    {
        return -1;
    }

    return 0;
} /* fgetpos64 */

