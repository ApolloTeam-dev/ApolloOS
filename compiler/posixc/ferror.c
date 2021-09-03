/*
    Copyright (C) 1995-2021, The AROS Development Team. All rights reserved.

    C99 function ferror().
*/

#include "__stdio.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

        int __posixc_ferror (

/*  SYNOPSIS */
        FILE * stream)

/*  FUNCTION
        Test the error flag of a stream. This flag is set automatically by
        any function that detects an error. To clear it, call clearerr().

    INPUTS
        stream - The stream to be tested.

    RESULT
        != 0, if the stream had an error, 0 otherwise.

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        clearerr()

    INTERNALS

******************************************************************************/
{
    return (stream->flags & __POSIXC_STDIO_ERROR) != 0;
} /* ferror */
