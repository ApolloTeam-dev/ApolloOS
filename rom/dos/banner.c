/*
    Copyright © 2020, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <proto/exec.h>
#include <exec/rawfmt.h>

#include <string.h>

char *generate_banner()
{
#if defined(__DISTRONAME__)
	BYTE const CDistro[] = "" __DISTRONAME__ " " __DISTROVERSION__ " (" __DISTRODATE__ ")";
#endif

    char *banner;

#if defined(__DISTRONAME__)
	IPTR CParams[3];
	CParams[0] = (IPTR)CDistro;
	CParams[1] = (IPTR)TaggedOpenLibrary(-2);
    CParams[2] = (IPTR)TaggedOpenLibrary(-3);
	banner = AllocVec(strlen((char *)CParams[0]) + strlen((char *)CParams[1]) + strlen((char *)CParams[2]) + 4, MEMF_CLEAR);
    RawDoFmt("%s\n\n", (RAWARG)CParams, RAWFMTFUNC_STRING, banner);
#else
	IPTR CParams[2];
	CParams[0] = (IPTR)TaggedOpenLibrary(-2);
	CParams[1] = (IPTR)TaggedOpenLibrary(-3);
    banner = AllocVec(strlen((char *)CParams[0]) + strlen((char *)CParams[1]) + 4, MEMF_CLEAR);
    RawDoFmt("%s%s\n%s", (RAWARG)CParams, RAWFMTFUNC_STRING, banner);
#endif
    return banner;
}
