/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    Change the position in a stream.
*/

#include "__stdio.h"

/*****************************************************************************

    NAME */
#include <stdio.h>

        int fseeko64 (

/*  SYNOPSIS */
        FILE * stream,
        off64_t  offset,
        int    whence)

/*  FUNCTION
        Change the current position in a stream.

    INPUTS
        stream - Modify this stream
        offset, whence - How to modify the current position. whence
                can be SEEK_SET, then offset is the absolute position
                in the file (0 is the first byte), SEEK_CUR then the
                position will change by offset (ie. -5 means to move
                5 bytes to the beginning of the file) or SEEK_END.
                SEEK_END means that the offset is relative to the
                end of the file (-1 is the last byte and 0 is
                the EOF).

    RESULT
        0 on success and -1 on error. If an error occurred, the global
        variable errno is set.

    NOTES
        Allows seeking on files that may be larger than 2GB, if the
        underlying filesystem supports it.

    EXAMPLE

    BUGS
        Not fully compatible with ISO fseeko, especially in 'ab' and 'a+b'
        modes

    SEE ALSO
        fopen64(), ftello64()

    INTERNALS
        just a hack, not 64-bit

******************************************************************************/
{

    return __fseeko64(stream, offset, whence);

} /* fseeko64 */
