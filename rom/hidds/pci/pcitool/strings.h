/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/rom/hidds/pci/pcitool/catalogs/pcitool.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef PCITOOL_STRINGS_H
#define PCITOOL_STRINGS_H


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif


#ifdef CATCOMP_ARRAY
#ifndef CATCOMPARRAY
#define CATCOMPARRAY CatCompArray
#endif
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif

/***************************************************************/

#ifdef CATCOMP_NUMBERS

#define MSG_TITLE 0
#define MSG_WINTITLE 1
#define MSG_DESCRIPTION 2
#define MSG_ERROR_LOCALE 3
#define MSG_ERROR_OK 4
#define MSG_ERROR_HEADER 5
#define MSG_DRIVER_INFO 6
#define MSG_DRIVER_NAME 7
#define MSG_DIRECT_BUS 8
#define MSG_HARDWARE_INFO 9
#define MSG_PCI_DEVICE_INFO 10
#define MSG_DEVICE_DESCRIPTION 11
#define MSG_VENDORNAME 12
#define MSG_PRODUCTNAME 13
#define MSG_SUBSYSTEM 14
#define MSG_VENDORID 15
#define MSG_PRODUCTID 16
#define MSG_REVISIONID 17
#define MSG_INTERFACE 18
#define MSG_CLASS 19
#define MSG_SUBCLASS 20
#define MSG_IRQ 21
#define MSG_ROM_BASE 22
#define MSG_ROM_SIZE 23
#define MSG_BRIDGE 24
#define MSG_MEMORY_RANGE 25
#define MSG_PREFETCHABLE_MEMORY 26
#define MSG_IO_RANGE 27
#define MSG_IO_MSG 28
#define MSG_IO 29
#define MSG_MEMORY 30
#define MSG_PREFETCHABLE 31
#define MSG_YES 32
#define MSG_NO 33
#define MSG_NA 34
#define MSG_NOT 35
#define MSG_SIZE4 36
#define MSG_SIZE8 37
#define MSG_SAVETORAMDISK 38
#define MSG_SAVEALLTORAMDISK 39
#define MSG_UNUSED 40
#define MSG_IO_BASE 41
#define MSG_OWNER 42
#define MSG_PCIE_DEVICE_INFO 43
#define MSG_PCIE_SERIAL_NUMBER 44

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_TITLE_STR "PCI Tool"
#define MSG_WINTITLE_STR "PCI Tool"
#define MSG_DESCRIPTION_STR "PCI querying and managment"
#define MSG_ERROR_LOCALE_STR "Can't open locale"
#define MSG_ERROR_OK_STR "Ok"
#define MSG_ERROR_HEADER_STR "ERROR"
#define MSG_DRIVER_INFO_STR "Driver info"
#define MSG_DRIVER_NAME_STR "Driver name:"
#define MSG_DIRECT_BUS_STR "Direct bus:"
#define MSG_HARDWARE_INFO_STR "Hardware info:"
#define MSG_PCI_DEVICE_INFO_STR "PCI device info"
#define MSG_DEVICE_DESCRIPTION_STR "Description:"
#define MSG_VENDORNAME_STR "Vendor Name:"
#define MSG_PRODUCTNAME_STR "Product Name:"
#define MSG_SUBSYSTEM_STR "Subsystem:"
#define MSG_VENDORID_STR "VendorID:"
#define MSG_PRODUCTID_STR "ProductID:"
#define MSG_REVISIONID_STR "RevisionID:"
#define MSG_INTERFACE_STR "Interface:"
#define MSG_CLASS_STR "Class:"
#define MSG_SUBCLASS_STR "Subclass:"
#define MSG_IRQ_STR "IRQ:"
#define MSG_ROM_BASE_STR "ROM Base:"
#define MSG_ROM_SIZE_STR "ROM Size:"
#define MSG_BRIDGE_STR "Bridge handles bus %d (ISA %senabled, VGA %senabled)"
#define MSG_MEMORY_RANGE_STR "Memory ranges from %08x to %08x"
#define MSG_PREFETCHABLE_MEMORY_STR "Prefetchable ranges memory from %08x to %08x"
#define MSG_IO_RANGE_STR "IO ranges from %04x to %04x"
#define MSG_IO_MSG_STR "IO: %s, MEM: %s, Master: %s, PaletteSnoop: %s, 66MHz capable: %s"
#define MSG_IO_STR "I/O"
#define MSG_MEMORY_STR "Memory"
#define MSG_PREFETCHABLE_STR "Prefetchable Memory"
#define MSG_YES_STR "yes"
#define MSG_NO_STR "no"
#define MSG_NA_STR "N/A"
#define MSG_NOT_STR "not"
#define MSG_SIZE4_STR "%s at 0x%04x (size 0x%04x)"
#define MSG_SIZE8_STR "%s at 0x%08x (size 0x%08x)"
#define MSG_SAVETORAMDISK_STR "Save Device Information to RAM Disk"
#define MSG_SAVEALLTORAMDISK_STR "Save complete PCI Information to RAM Disk"
#define MSG_UNUSED_STR "--unused--"
#define MSG_IO_BASE_STR "I/O base:"
#define MSG_OWNER_STR "Used by:"
#define MSG_PCIE_DEVICE_INFO_STR "PCIe device info"
#define MSG_PCIE_SERIAL_NUMBER_STR "PCIe Serial Number"

#endif /* CATCOMP_STRINGS */

/***************************************************************/

#ifdef CATCOMP_ARRAY

#ifndef HAVE_CATCOMP_ARRAYTYPE
#define HAVE_CATCOMP_ARRAYTYPE
struct CatCompArrayType
{
  IPTR   cca_ID;
  STRPTR cca_Str;
};
#endif

static const struct CatCompArrayType CATCOMPARRAY[] =
{
  {MSG_TITLE,(STRPTR)MSG_TITLE_STR},
  {MSG_WINTITLE,(STRPTR)MSG_WINTITLE_STR},
  {MSG_DESCRIPTION,(STRPTR)MSG_DESCRIPTION_STR},
  {MSG_ERROR_LOCALE,(STRPTR)MSG_ERROR_LOCALE_STR},
  {MSG_ERROR_OK,(STRPTR)MSG_ERROR_OK_STR},
  {MSG_ERROR_HEADER,(STRPTR)MSG_ERROR_HEADER_STR},
  {MSG_DRIVER_INFO,(STRPTR)MSG_DRIVER_INFO_STR},
  {MSG_DRIVER_NAME,(STRPTR)MSG_DRIVER_NAME_STR},
  {MSG_DIRECT_BUS,(STRPTR)MSG_DIRECT_BUS_STR},
  {MSG_HARDWARE_INFO,(STRPTR)MSG_HARDWARE_INFO_STR},
  {MSG_PCI_DEVICE_INFO,(STRPTR)MSG_PCI_DEVICE_INFO_STR},
  {MSG_DEVICE_DESCRIPTION,(STRPTR)MSG_DEVICE_DESCRIPTION_STR},
  {MSG_VENDORNAME,(STRPTR)MSG_VENDORNAME_STR},
  {MSG_PRODUCTNAME,(STRPTR)MSG_PRODUCTNAME_STR},
  {MSG_SUBSYSTEM,(STRPTR)MSG_SUBSYSTEM_STR},
  {MSG_VENDORID,(STRPTR)MSG_VENDORID_STR},
  {MSG_PRODUCTID,(STRPTR)MSG_PRODUCTID_STR},
  {MSG_REVISIONID,(STRPTR)MSG_REVISIONID_STR},
  {MSG_INTERFACE,(STRPTR)MSG_INTERFACE_STR},
  {MSG_CLASS,(STRPTR)MSG_CLASS_STR},
  {MSG_SUBCLASS,(STRPTR)MSG_SUBCLASS_STR},
  {MSG_IRQ,(STRPTR)MSG_IRQ_STR},
  {MSG_ROM_BASE,(STRPTR)MSG_ROM_BASE_STR},
  {MSG_ROM_SIZE,(STRPTR)MSG_ROM_SIZE_STR},
  {MSG_BRIDGE,(STRPTR)MSG_BRIDGE_STR},
  {MSG_MEMORY_RANGE,(STRPTR)MSG_MEMORY_RANGE_STR},
  {MSG_PREFETCHABLE_MEMORY,(STRPTR)MSG_PREFETCHABLE_MEMORY_STR},
  {MSG_IO_RANGE,(STRPTR)MSG_IO_RANGE_STR},
  {MSG_IO_MSG,(STRPTR)MSG_IO_MSG_STR},
  {MSG_IO,(STRPTR)MSG_IO_STR},
  {MSG_MEMORY,(STRPTR)MSG_MEMORY_STR},
  {MSG_PREFETCHABLE,(STRPTR)MSG_PREFETCHABLE_STR},
  {MSG_YES,(STRPTR)MSG_YES_STR},
  {MSG_NO,(STRPTR)MSG_NO_STR},
  {MSG_NA,(STRPTR)MSG_NA_STR},
  {MSG_NOT,(STRPTR)MSG_NOT_STR},
  {MSG_SIZE4,(STRPTR)MSG_SIZE4_STR},
  {MSG_SIZE8,(STRPTR)MSG_SIZE8_STR},
  {MSG_SAVETORAMDISK,(STRPTR)MSG_SAVETORAMDISK_STR},
  {MSG_SAVEALLTORAMDISK,(STRPTR)MSG_SAVEALLTORAMDISK_STR},
  {MSG_UNUSED,(STRPTR)MSG_UNUSED_STR},
  {MSG_IO_BASE,(STRPTR)MSG_IO_BASE_STR},
  {MSG_OWNER,(STRPTR)MSG_OWNER_STR},
  {MSG_PCIE_DEVICE_INFO,(STRPTR)MSG_PCIE_DEVICE_INFO_STR},
  {MSG_PCIE_SERIAL_NUMBER,(STRPTR)MSG_PCIE_SERIAL_NUMBER_STR},
  {0,NULL}
};

#endif /* CATCOMP_ARRAY */

/***************************************************************/

#ifdef CATCOMP_BLOCK

#error CATCOMP_BLOCK not supported by this SD

#endif /* CATCOMP_BLOCK */

/***************************************************************/

#ifdef CATCOMP_CODE

#error CATCOMP_CODE not supported by this SD

#endif /* CATCOMP_CODE */

/***************************************************************/


#endif /* PCITOOL_STRINGS_H */
