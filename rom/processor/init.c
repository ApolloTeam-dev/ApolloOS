/*
    Copyright � 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
*/

#include <aros/debug.h>
#include <aros/symbolsets.h>

#include <proto/exec.h>
#include <proto/kernel.h>

#include "processor_intern.h"

static LONG common_Init(struct ProcessorBase *ProcessorBase)
{
    KernelBase = OpenResource("kernel.resource");
    if (!KernelBase)
    	return FALSE;

    ProcessorBase->cpucount = KrnGetCPUCount();
   return TRUE;
}

ADD2INITLIB(common_Init, 0)
