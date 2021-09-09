/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    POSIX.1-2008 function alphasort64().
*/

#include <string.h>

/*****************************************************************************

    NAME */
#include <dirent.h>

        int alphasort64 (

/*  SYNOPSIS */
        const struct dirent64 **a,
        const struct dirent64 **b
        )

/*  FUNCTION
        Support function for scandir64().

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_scandir()

    INTERNALS

******************************************************************************/
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

