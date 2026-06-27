/*
    Copyright � 1995-2010, The AROS Development Team. All rights reserved.
    $Id$

    Internal data structures for bootloader.resource
*/

#ifndef BOOTLOADER_INTERN_H
#define BOOTLOADER_INTERN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif
#ifndef UTILITY_UTILITY_H
#include <utility/utility.h>
#endif
#ifndef HIDD_HIDD_H
#include <hidd/hidd.h>
#endif
#include <aros/bootloader.h>

struct BootLoaderBase
{
    struct Node		 bl_Node;
    ULONG		 Flags;
    STRPTR		 LdrName;
    struct List		 Args;
    struct List		 DriveInfo;
    struct VesaInfo	*Vesa;
};

#define BL_FLAGS_CMDLINE 0x01
#define BL_FLAGS_DRIVES  0x02

#if APOLLO_DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#if DEBUG
#define D(x) x
#else
#define D(x)
#endif

#endif //BOOTLOADER_INTERN_H
