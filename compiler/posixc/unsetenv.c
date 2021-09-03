/*
    Copyright (C) 1995-2021, The AROS Development Team. All rights reserved.

    POSIX.1-2008 function unsetenv().
*/

#include <proto/dos.h>

/*****************************************************************************

    NAME */
#include <stdlib.h>

        int unsetenv (

/*  SYNOPSIS */
        const char *name)

/*  FUNCTION
         deletes a variable from the environment.

    INPUTS
        name  --  Name of the environment variable to delete.

    RESULT
       Returns zero on success, or -1 if the variable was not found.

    NOTES
    
    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

******************************************************************************/
{
    DeleteVar(name, GVF_LOCAL_ONLY);
} /* unsetenv */

