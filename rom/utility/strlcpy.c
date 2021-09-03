/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.
*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <aros/crt_replacement.h>

#include "intern.h"

/*****************************************************************************

    NAME */
#include <proto/utility.h>

        AROS_LH3(LONG, Strlcpy,

/*  SYNOPSIS */
        AROS_LHA(STRPTR, destination, A0),
        AROS_LHA(CONST_STRPTR, source, A1),
        AROS_LHA(LONG, size, D0),

/*  LOCATION */
        struct UtilityBase *, UtilityBase, 50, Utility)

/*  FUNCTION
    Copies the string 'source' into 'destination'. String will be
    null-terminated. Not more than 'size' Bytes will be written.
    
    INPUTS
    destination - the target. Might be NULL.
    source      - the string which will be copied. Might be NULL.
    size        - the size of the 'destination'
    
    RESULT
    The string lenght of 'source'.
    
    NOTES

    EXAMPLE
    Strlcpy(buffer, "Hello", sizeof buffer);
    
    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    if (destination)
    {
        if (source)
        {
            STRPTR d = destination;
            CONST_STRPTR s = source;
            LONG n = size;

            if (n != 0 && --n != 0)
            {
                do
                {
                    if ((*d++ = *s++) == 0)
                        break;
                } while (--n != 0);
            }

            if (n == 0)
            {
                if (size != 0)
                    *d = '\0';
                while (*s++)
                    ;
            }

            return s - source - 1;
        }
        if (size > 0)
        {
            *destination = '\0';
        }
    }
    else
    {
        if (source)
        {
            return Strlen(source);
        }
    }
    return 0;

    AROS_LIBFUNC_EXIT
}
