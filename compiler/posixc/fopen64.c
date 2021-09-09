/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    C99 function fopen().
*/

#include "__stdio.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

        FILE * fopen64 (

/*  SYNOPSIS */
        const char * pathname,
        const char * mode)

/*  FUNCTION
        Opens a file with the specified name in the specified mode.

    INPUTS
        pathname - Path and filename of the file you want to open.
        mode - How to open the file:

                r: Open for reading. The stream is positioned at the
                        beginning of the file.

                r+: Open for reading and writing. The stream is positioned
                        at the beginning of the file.

                w: Open for writing. If the file doesn't exist, then
                        it is created. If it does already exist, then
                        it is truncated. The stream is positioned at the
                        beginning of the file.

                w+: Open for reading and writing. If the file doesn't
                        exist, then it is created. If it does already
                        exist, then it is truncated. The stream is
                        positioned at the beginning of the file.

                a: Open for writing. If the file doesn't exist, then
                        it is created. The stream is positioned at the
                        end of the file.

                a+: Open for reading and writing. If the file doesn't
                        exist, then it is created. The stream is positioned
                        at the end of the file.

                b: Open in binary more. This has no effect and is ignored.

    RESULT
        A pointer to a FILE handle or NULL in case of an error. When NULL
        is returned, then errno is set to indicate the error.

    NOTES
        Provides access larger to files that may be larger than 2 GB, if the
        underlying filesystem supports it.
       
        This function must not be used in a shared library or
        in a threaded application.

    EXAMPLE

    BUGS
        Most modes are not supported right now.

    SEE ALSO
        fseeko64(), ftello64()

    INTERNALS

******************************************************************************/
{
    return __fopen(pathname, mode, 1);
} /* fopen64 */
