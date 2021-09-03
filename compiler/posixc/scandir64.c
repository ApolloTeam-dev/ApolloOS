/*
    Copyright (C) 2020, The AROS Development Team. All rights reserved.

    POSIX.1-2008 function scandir64().
*/

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************

    NAME */
#include <dirent.h>

        int scandir64 (

/*  SYNOPSIS */
        const char *dir,
        struct dirent64 ***namelist,
        int (*select)(const struct dirent64 *),
        int (*compar)(const struct dirent64 **, const struct dirent64 **)
        )

/*  FUNCTION
        Scan directory

    INPUTS
        dir      - Directory to be scanned
        namelist - Array with the found entries.
        select   - Filter function which must return non-zero if entry shall be
                   added. If NULL all entries will be added.
        compar   - Function which will be used by qsort() for sorting of the
                   entries. The function alphasort() can be used for sorting
                   in alphabetical order. If NULL sorting order isn't specified.

    RESULT
        Number of entries

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        __posixc_alphasort()

    INTERNALS

******************************************************************************/
{
    DIR *dirp = NULL;
    struct dirent64 *dp;
    struct dirent64 *newdp;
    int cnt = 0;
    struct dirent64 **darr = NULL;
    struct dirent64 **newdarr;
    int arrcnt = 0;
    int selected;
    int olderrno;

    errno = 0;

    if ((dirp = opendir(dir)) == NULL)
    {
        goto fail;
    }

    do
    {
        if ((dp = readdir64(dirp)) != NULL)
        {
            selected = 0;
            if (select != NULL)
            {
                olderrno = errno;
                selected = select(dp);
                errno = olderrno;
            }
            if ((select == NULL) || selected)
            {
                if ((darr == NULL) || cnt >= arrcnt)
                {
                    arrcnt += 50;
                    newdarr = realloc(darr, arrcnt * sizeof(*darr));
                    if (newdarr == NULL)
                    {
                        goto fail;
                    }
                    darr = newdarr;
                }
                newdp = malloc(sizeof(*newdp));
                if (newdp == NULL)
                {
                    goto fail;
                }
                memcpy(newdp, dp, sizeof(*newdp));
                darr[cnt] = newdp;
                cnt++;
            }
        }
    } while (dp != NULL);

    closedir(dirp);

    if (compar != NULL)
    {
        olderrno = errno;
        qsort (darr, cnt, sizeof (*darr), (int (*) (const void *, const void *)) compar);
        errno = olderrno;
    }
    *namelist = darr;
    return cnt;

fail:
    while (cnt > 0)
    {
        free(darr[--cnt]);
    }
    free(darr);
    closedir(dirp);
    return -1;
}
