/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    Desc:
*/

/*********************************************************************************************/

#include "global.h"

#include <aros/debug.h>

/*********************************************************************************************/

void LocalePrefs_Handler(STRPTR filename)
{
    struct Locale  *newloc /*, *oldloc */;
    
    D(bug("In IPrefs:LocalePrefs_Handler\n"));
    
    if ((newloc = OpenLocale(filename)))
    {
        D(bug("In IPrefs:LocalePrefs_Handler. OpenLocale(\"%s\") okay\n", filename));
        /* oldloc = */ LocalePrefsUpdate(newloc);
        D(bug("In IPrefs:LocalePrefs_Handler. New Locale installed\n", filename));
        
        /* Never close old locale */
        
        /* if (oldloc) CloseLocale(oldloc); */
    }
    
    D(bug("In IPrefs:LocalePrefs_Handler. Done.\n", filename));
}

/*********************************************************************************************/
