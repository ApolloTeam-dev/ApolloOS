/*
    Copyright (C) 1995-2021, The AROS Development Team. All rights reserved.

    Change the position in a stream.
*/

/*****************************************************************************

    NAME */
#include <stdio.h>

        void __posixc_rewind (

/*  SYNOPSIS */
        FILE * stream)

/*  FUNCTION
        Change the current position in a stream to the beginning.

    INPUTS
        stream - Modify this stream

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_fopen(), __posixc_fwrite(), __posixc_fseek()

    INTERNALS

******************************************************************************/
{
    fseek (stream, 0L, SEEK_SET);
    clearerr (stream);
} /* rewind */

