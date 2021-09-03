/*
    Copyright (C) 2013-2020, The AROS Development Team. All rights reserved

    Desc:
*/

#include <aros/debug.h>

#include <proto/exec.h>

/* We want all other bases obtained from our base */
#define __NOLIBBASE__

#include <proto/oop.h>

#include <aros/symbolsets.h>
#include <hidd/ata.h>
#include <hidd/storage.h>
#include <hidd/hidd.h>
#include <hidd/pci.h>

#include "ata_pci_intern.h"

static CONST_STRPTR attrBaseIDs[] =
{
    IID_Hidd_PCIDevice,
    IID_Hidd_PCIDriver,
    IID_Hidd,
    IID_Hidd_Bus,
    IID_Hidd_ATABus,
    IID_HW,
    NULL
};

#define AB_MANDATORY 2

#if defined(__OOP_NOMETHODBASES__)
static CONST_STRPTR const methBaseIDs[] =
{
    IID_HW,
    IID_Hidd_StorageController,
    NULL
};
#endif

static int pciata_init(struct atapciBase *base)
{
    D(bug("[ATA:PCI] %s()\n", __func__));

    base->psd.cs_UtilityBase = TaggedOpenLibrary(TAGGEDOPEN_UTILITY);
    if (!base->psd.cs_UtilityBase)
        return FALSE;

    base->psd.cs_KernelBase = OpenResource("kernel.resource");
    if (!base->psd.cs_KernelBase)
        return FALSE;

    /*
     * We handle also legacy ISA devices, so we can work without PCI subsystem.
     * Because of this, we do not obtain PCI bases here. We do it later, in device
     * discovery code.
     */
    if (OOP_ObtainAttrBasesArray(&base->psd.hiddAttrBase, &attrBaseIDs[AB_MANDATORY]))
        return FALSE;

    base->psd.storageRoot = OOP_NewObject(NULL, CLID_Hidd_Storage, NULL);
    if (!base->psd.storageRoot)
        base->psd.storageRoot = OOP_NewObject(NULL, CLID_HW_Root, NULL);
    if (!base->psd.storageRoot)
    {
        OOP_ReleaseAttrBasesArray(&base->psd.hiddAttrBase, &attrBaseIDs[AB_MANDATORY]);
        return FALSE;
    }
    D(bug("[ATA:PCI] %s: storage root @ 0x%p\n", __func__, base->psd.storageRoot);)

    if ((base->psd.ataClass = OOP_FindClass(CLID_Hidd_ATA)) == NULL)
    {
        OOP_ReleaseAttrBasesArray(&base->psd.hiddAttrBase, &attrBaseIDs[AB_MANDATORY]);
        return FALSE;
    }
    D(
      bug("[ATA:PCI] %s: Base %s Class @ 0x%p\n", __func__, CLID_Hidd_ATA, base->psd.ataClass);
      bug("[ATA:PCI] %s: PCI %s Class @ 0x%p\n", __func__, CLID_Hidd_ATABus, base->psd.ataPCIClass);
      bug("[ATA:PCI] %s: PCI %s Class @ 0x%p\n", __func__, CLID_Hidd_ATABus, base->psd.ataPCIBusClass);
    )

#if defined(__OOP_NOMETHODBASES__)
    if (OOP_ObtainMethodBasesArray(&base->psd.HWMethodBase, methBaseIDs))
    {
        bug("[ATA:PCI] %s: Failed to obtain MethodBases!\n", __func__);
        bug("[ATA:PCI] %s:     %s = %p\n", __func__, methBaseIDs[0], base->psd.HWMethodBase);
        bug("[ATA:PCI] %s:     %s = %p\n", __func__, methBaseIDs[1], base->psd.HiddSCMethodBase);
        OOP_ReleaseAttrBasesArray(&base->psd.hiddAttrBase, attrBaseIDs);
        return FALSE;
    }
#endif

    return TRUE;
}

static int pciata_expunge(struct atapciBase *base)
{
    /* Release all attribute bases */
    OOP_ReleaseAttrBasesArray(&base->psd.hiddAttrBase, &attrBaseIDs[AB_MANDATORY]);

    if (base->psd.cs_UtilityBase)
        CloseLibrary(base->psd.cs_UtilityBase);

    return TRUE;
}

ADD2INITLIB(pciata_init, 0)
ADD2EXPUNGELIB(pciata_expunge, 0)
