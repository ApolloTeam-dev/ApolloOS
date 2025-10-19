/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/tools/SysExplorer/catalogs/sysexplorer.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef SYSEXPLORER_STRINGS_H
#define SYSEXPLORER_STRINGS_H


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
#define MSG_MENU_PROJECT 3
#define MSG_MENU_PROPERTIES 4
#define MSG_MENU_EXPANDALL 5
#define MSG_MENU_COLLAPSEALL 6
#define MSG_MENU_QUIT 7
#define MSG_HIDD_TREE 8
#define MSG_PROPERTIES 9
#define MSG_LABEL_DRIVER 10
#define MSG_LABEL_HW_DESCRIPTION 11
#define MSG_LABEL_VENDOR 12
#define MSG_SYSTEM_PROPERTIES 13
#define MSG_VERSION 14
#define MSG_PROCESSORS 15
#define MSG_HPET 16
#define MSG_RAM 17
#define MSG_BOOTLOADER 18
#define MSG_ARGUMENTS 19
#define MSG_AVAILABLE 20
#define MSG_GENERAL 21
#define MSG_NAME 22
#define MSG_HARDWARE_NAME 23
#define MSG_PRODUCT_ID 24
#define MSG_PRODUCER_NAME 25
#define MSG_PRODUCER_ID 26
#define MSG_DEVICE_PROPERTIES 27
#define MSG_ATA_PROPERTIES 28
#define MSG_ATA 29
#define MSG_USE_IOALT 30
#define MSG_USE_32BIT 31
#define MSG_USE_DMA 32
#define MSG_USE_80WIRE 33
#define MSG_ATA_UNIT_PROPERTIES 34
#define MSG_UNIT_NUMBER 35
#define MSG_MODEL 36
#define MSG_REVISION 37
#define MSG_SERIAL 38
#define MSG_TRANSFER_MODES 39
#define MSG_MULTISECTOR 40
#define MSG_REMOVABLE 41
#define MSG_CONFIG_MODES 42

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_TITLE_STR "System Explorer"
#define MSG_WINTITLE_STR "System Explorer"
#define MSG_DESCRIPTION_STR "System and hardware information tool"
#define MSG_MENU_PROJECT_STR "Project"
#define MSG_MENU_PROPERTIES_STR "Properties"
#define MSG_MENU_EXPANDALL_STR "Expand All"
#define MSG_MENU_COLLAPSEALL_STR "Collapse All"
#define MSG_MENU_QUIT_STR "Quit"
#define MSG_HIDD_TREE_STR "HIDD Tree"
#define MSG_PROPERTIES_STR "Properties"
#define MSG_LABEL_DRIVER_STR "Driver instance name"
#define MSG_LABEL_HW_DESCRIPTION_STR "Hardware description"
#define MSG_LABEL_VENDOR_STR "Vendor information"
#define MSG_SYSTEM_PROPERTIES_STR "System properties"
#define MSG_VERSION_STR "Version"
#define MSG_PROCESSORS_STR "Processors"
#define MSG_HPET_STR "HPET"
#define MSG_RAM_STR "Memory"
#define MSG_BOOTLOADER_STR "Boot loader"
#define MSG_ARGUMENTS_STR "Boot Arguments"
#define MSG_AVAILABLE_STR "Available for use"
#define MSG_GENERAL_STR "General"
#define MSG_NAME_STR "Name"
#define MSG_HARDWARE_NAME_STR "Hardware Name"
#define MSG_PRODUCT_ID_STR "Product ID"
#define MSG_PRODUCER_NAME_STR "Vendor Name"
#define MSG_PRODUCER_ID_STR "Vendor ID"
#define MSG_DEVICE_PROPERTIES_STR "Device properties"
#define MSG_ATA_PROPERTIES_STR "IDE bus properties"
#define MSG_ATA_STR "IDE"
#define MSG_USE_IOALT_STR "Supports alternate registers"
#define MSG_USE_32BIT_STR "Supports 32-bit PIO"
#define MSG_USE_DMA_STR "Supports DMA"
#define MSG_USE_80WIRE_STR "80-wire cable"
#define MSG_ATA_UNIT_PROPERTIES_STR "IDE device properties"
#define MSG_UNIT_NUMBER_STR "Unit number"
#define MSG_MODEL_STR "Model"
#define MSG_REVISION_STR "Revision"
#define MSG_SERIAL_STR "Serial"
#define MSG_TRANSFER_MODES_STR "Supported modes"
#define MSG_MULTISECTOR_STR "Multisector limit"
#define MSG_REMOVABLE_STR "Removable"
#define MSG_CONFIG_MODES_STR "Configured modes"

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
  {MSG_MENU_PROJECT,(STRPTR)MSG_MENU_PROJECT_STR},
  {MSG_MENU_PROPERTIES,(STRPTR)MSG_MENU_PROPERTIES_STR},
  {MSG_MENU_EXPANDALL,(STRPTR)MSG_MENU_EXPANDALL_STR},
  {MSG_MENU_COLLAPSEALL,(STRPTR)MSG_MENU_COLLAPSEALL_STR},
  {MSG_MENU_QUIT,(STRPTR)MSG_MENU_QUIT_STR},
  {MSG_HIDD_TREE,(STRPTR)MSG_HIDD_TREE_STR},
  {MSG_PROPERTIES,(STRPTR)MSG_PROPERTIES_STR},
  {MSG_LABEL_DRIVER,(STRPTR)MSG_LABEL_DRIVER_STR},
  {MSG_LABEL_HW_DESCRIPTION,(STRPTR)MSG_LABEL_HW_DESCRIPTION_STR},
  {MSG_LABEL_VENDOR,(STRPTR)MSG_LABEL_VENDOR_STR},
  {MSG_SYSTEM_PROPERTIES,(STRPTR)MSG_SYSTEM_PROPERTIES_STR},
  {MSG_VERSION,(STRPTR)MSG_VERSION_STR},
  {MSG_PROCESSORS,(STRPTR)MSG_PROCESSORS_STR},
  {MSG_HPET,(STRPTR)MSG_HPET_STR},
  {MSG_RAM,(STRPTR)MSG_RAM_STR},
  {MSG_BOOTLOADER,(STRPTR)MSG_BOOTLOADER_STR},
  {MSG_ARGUMENTS,(STRPTR)MSG_ARGUMENTS_STR},
  {MSG_AVAILABLE,(STRPTR)MSG_AVAILABLE_STR},
  {MSG_GENERAL,(STRPTR)MSG_GENERAL_STR},
  {MSG_NAME,(STRPTR)MSG_NAME_STR},
  {MSG_HARDWARE_NAME,(STRPTR)MSG_HARDWARE_NAME_STR},
  {MSG_PRODUCT_ID,(STRPTR)MSG_PRODUCT_ID_STR},
  {MSG_PRODUCER_NAME,(STRPTR)MSG_PRODUCER_NAME_STR},
  {MSG_PRODUCER_ID,(STRPTR)MSG_PRODUCER_ID_STR},
  {MSG_DEVICE_PROPERTIES,(STRPTR)MSG_DEVICE_PROPERTIES_STR},
  {MSG_ATA_PROPERTIES,(STRPTR)MSG_ATA_PROPERTIES_STR},
  {MSG_ATA,(STRPTR)MSG_ATA_STR},
  {MSG_USE_IOALT,(STRPTR)MSG_USE_IOALT_STR},
  {MSG_USE_32BIT,(STRPTR)MSG_USE_32BIT_STR},
  {MSG_USE_DMA,(STRPTR)MSG_USE_DMA_STR},
  {MSG_USE_80WIRE,(STRPTR)MSG_USE_80WIRE_STR},
  {MSG_ATA_UNIT_PROPERTIES,(STRPTR)MSG_ATA_UNIT_PROPERTIES_STR},
  {MSG_UNIT_NUMBER,(STRPTR)MSG_UNIT_NUMBER_STR},
  {MSG_MODEL,(STRPTR)MSG_MODEL_STR},
  {MSG_REVISION,(STRPTR)MSG_REVISION_STR},
  {MSG_SERIAL,(STRPTR)MSG_SERIAL_STR},
  {MSG_TRANSFER_MODES,(STRPTR)MSG_TRANSFER_MODES_STR},
  {MSG_MULTISECTOR,(STRPTR)MSG_MULTISECTOR_STR},
  {MSG_REMOVABLE,(STRPTR)MSG_REMOVABLE_STR},
  {MSG_CONFIG_MODES,(STRPTR)MSG_CONFIG_MODES_STR},
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


#endif /* SYSEXPLORER_STRINGS_H */
