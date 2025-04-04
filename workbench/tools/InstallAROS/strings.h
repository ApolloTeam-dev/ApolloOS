/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/tools/InstallAROS/catalogs/installaros.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef INSTALLAROS_STRINGS_H
#define INSTALLAROS_STRINGS_H


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
#define MSG_DESCRIPTION 1
#define MSG_WELCOME 2
#define MSG_INSTALLOPTIONS 3
#define MSG_DESTOPTIONS 4
#define MSG_ACCEPT 5
#define MSG_DRIVE 6
#define MSG_TYPE 7
#define MSG_DEVICE 8
#define MSG_UNIT 9
#define MSG_NAME 10
#define MSG_FILESYSTEM 11
#define MSG_SIZE 12
#define MSG_MENUMODE 13
#define MSG_CREATE 14
#define MSG_SPECIFYSIZE 15
#define MSG_DESTVOLUME 16
#define MSG_WORKVOLUME 17
#define MSG_CHOOSELANG 18
#define MSG_INSTALLCORE 19
#define MSG_INSTALLEXTRA 20
#define MSG_INSTALLDEVEL 21
#define MSG_INSTALLBOOT 22
#define MSG_FORMATPART 23
#define MSG_USEWORK 24
#define MSG_USEWORKFOR 25
#define MSG_BEGINWITHPARTITION 26
#define MSG_BEGINWITHOUTPARTITION 27
#define MSG_PARTITIONOPTIONS 28
#define MSG_DESTPARTITION 29
#define MSG_WORKPARTITION 30
#define MSG_USEFREE 31
#define MSG_WIPEDISK 32
#define MSG_USEEXISTING 33
#define MSG_PARTITIONTOOBIG 34
#define MSG_GRUBOPTIONS 35
#define MSG_GRUBDRIVE 36
#define MSG_GRUBGOPTIONS 37
#define MSG_GRUBGRUB 38
#define MSG_NOGRUBDEVICE 39
#define MSG_PARTITIONING 40
#define MSG_FORMATTING 41
#define MSG_INSTALL 42
#define MSG_COPYCORE 43
#define MSG_COPYEXTRA 44
#define MSG_COPYDEVEL 45
#define MSG_COPYBOOT 46
#define MSG_BOOTLOADER 47
#define MSG_POSTINSTALL 48
#define MSG_POSTINSTALL2 49
#define MSG_DONEREBOOT 50
#define MSG_DONEUSB 51
#define MSG_INSTALLING 52
#define MSG_INSTALLINGBOOT 53
#define MSG_DONE 54
#define MSG_CANCELOK 55
#define MSG_CANCELDANGER 56
#define MSG_BACK 57
#define MSG_PROCEED 58
#define MSG_NOTALLFILESCOPIED 59
#define MSG_PARTITIONINGFAILED 60
#define MSG_GRUBNONFFSWARNING 61
#define MSG_CANCELINSTALL 62
#define MSG_CONTINUECANCELINST 63
#define MSG_CONTINUECANCELPART 64
#define MSG_CONTINUEQUIT 65
#define MSG_EXISTSRETRY 66
#define MSG_RETRY 67
#define MSG_SCANRETRY 68
#define MSG_FINDRETRY 69
#define MSG_COULDNTOPEN 70
#define MSG_COULDNTWRITE 71
#define MSG_DISKFULL 72
#define MSG_TEXT 73
#define MSG_GFX 74
#define MSG_YES 75
#define MSG_YESALWAYS 76
#define MSG_NO 77
#define MSG_SKIP 78
#define MSG_OK 79
#define MSG_CANCEL 80
#define MSG_CANCEL2 81
#define MSG_QUIT 82
#define MSG_QUIT2 83
#define MSG_ERROR 84
#define MSG_WARNING 85
#define MSG_REBOOT 86
#define MSG_IOERROR 87
#define MSG_NOTFILESYSTEM 88
#define MSG_NOTDEVICE 89

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_TITLE_STR "AROS Installer"
#define MSG_DESCRIPTION_STR "Installs AROS on to a Drive."
#define MSG_WELCOME_STR "\033b\nWelcome to the AROS Research OS installer.\n\n\033nThis program allows you to install AROS\non your computer's hard disk or an attached USB drive.\n\nPlease be aware that the stability of this software\ncannot be guaranteed.\nIt is possible that \033bloss of data\033n may occur\non any drive in your system, including those\nused by other operating systems.\n"
#define MSG_INSTALLOPTIONS_STR "\nPlease choose your installation options.\n\n"
#define MSG_DESTOPTIONS_STR "\nPlease choose where to install AROS.\n\nIf you are unsure, then use the defaults"
#define MSG_ACCEPT_STR "Accept License Agreement?"
#define MSG_DRIVE_STR "\033bDrive:\033n"
#define MSG_TYPE_STR "Type:"
#define MSG_DEVICE_STR "Device:"
#define MSG_UNIT_STR "Unit:"
#define MSG_NAME_STR "Name:"
#define MSG_FILESYSTEM_STR "Filesystem:"
#define MSG_SIZE_STR "Size:"
#define MSG_MENUMODE_STR "Menu Mode:"
#define MSG_CREATE_STR "Create"
#define MSG_SPECIFYSIZE_STR "Specify Size"
#define MSG_DESTVOLUME_STR "Destination Partition"
#define MSG_WORKVOLUME_STR "Work Partition"
#define MSG_CHOOSELANG_STR "Choose Language Options"
#define MSG_INSTALLCORE_STR "Install AROS Core System"
#define MSG_INSTALLEXTRA_STR "Install Extra Software"
#define MSG_INSTALLDEVEL_STR "Install Debugging tools and Developer Software"
#define MSG_INSTALLBOOT_STR "Install Bootloader"
#define MSG_FORMATPART_STR "Format Partition"
#define MSG_USEWORK_STR "Use 'Work' Partition"
#define MSG_USEWORKFOR_STR "Use 'Work' Partition for Extras and Developer Files"
#define MSG_BEGINWITHPARTITION_STR "OK, we are ready to begin.\n\nSince you've chosen to format at least one partition,\nyou will no longer be able to undo changes\nafter this point.\n\nSelect Proceed to begin installation."
#define MSG_BEGINWITHOUTPARTITION_STR "OK, we are ready to begin...\n\nWe have collected enough information\nto begin installation on this computer.\n\n\nSelect Proceed to begin installation."
#define MSG_PARTITIONOPTIONS_STR "We will now create AROS partitions on the destination drive.\n\nPlease select how you would like to proceed.\n"
#define MSG_DESTPARTITION_STR "\033bSystem Partition:\033n"
#define MSG_WORKPARTITION_STR "\033bWork Partition:\033n"
#define MSG_USEFREE_STR "Only use free space"
#define MSG_WIPEDISK_STR "Wipe disk"
#define MSG_USEEXISTING_STR "Use existing AROS partitions (on any drive)"
#define MSG_PARTITIONTOOBIG_STR "The partition sizes you have requested are too large.\nThe maximum size of an SFS partition is %ld GB (%ld MB),\nwhile the maximum size of an FFS partition is %ld GB (%ld MB).\nPlease reduce the size of the affected partitions.\n"
#define MSG_GRUBOPTIONS_STR "AROS uses the GRUB bootloader.\n\nIn most cases, GRUB's bootblock should\nbe installed on the first drive in your system.\nIf the Installer detects a Microsoft Windows installation,\nit will be included as a choice in GRUB's boot menu.\n"
#define MSG_GRUBDRIVE_STR "Drive to install Bootloader's bootblock on:"
#define MSG_GRUBGOPTIONS_STR "GRUB Settings"
#define MSG_GRUBGRUB_STR "Path to GRUB files:"
#define MSG_NOGRUBDEVICE_STR "Please enter the device name and unit number for\nthe drive GRUB's bootblock should be installed on.\n"
#define MSG_PARTITIONING_STR "Partitioning your drives...\n\n"
#define MSG_FORMATTING_STR "Formatting '%s'..."
#define MSG_INSTALL_STR "Copying files to the destination drive.\n\nThis may take some time...\n"
#define MSG_COPYCORE_STR "Copying Core System files..."
#define MSG_COPYEXTRA_STR "Copying Extra Software..."
#define MSG_COPYDEVEL_STR "Copying Developer Files..."
#define MSG_COPYBOOT_STR "Copying BOOT files..."
#define MSG_BOOTLOADER_STR "Copying the bootloader to your\ndestination drive, and installing...\n"
#define MSG_POSTINSTALL_STR "Running post-install script..."
#define MSG_POSTINSTALL2_STR "Running the external post-install\nscript. Please wait.\n"
#define MSG_DONEREBOOT_STR "AROS partitions have now been created!\nTo continue installation, you must\nreboot AROS, and re-run\nthis installer application\n\nSelect Proceed to finish\n"
#define MSG_DONEUSB_STR "AROS partitions have now been created!\nTo continue installation, you must remove\nand re-attach your USB drive, and re-run\nthis installer application\n\nSelect Proceed to finish\n"
#define MSG_INSTALLING_STR "Installing AROS..."
#define MSG_INSTALLINGBOOT_STR "Installing Bootloader..."
#define MSG_DONE_STR "Congratulations, you now have AROS installed!\n\nTo boot AROS from the destination drive,\nremove the installation media and\nrestart your computer using the\npower switch or reset button.\n\nSelect Proceed to finish\n"
#define MSG_CANCELOK_STR "Are you sure you wish to cancel\nthis installation?\n"
#define MSG_CANCELDANGER_STR "Irreversible changes have been made\nto your system.\n\nCancelling now may leave your PC in an\nunbootable state\n\nAre you sure you wish to cancel\nthis installation?\n"
#define MSG_BACK_STR "<< _Back..."
#define MSG_PROCEED_STR "_Proceed"
#define MSG_NOTALLFILESCOPIED_STR "Not all files in '%s'\nwere copied.\nShould the installation continue?\n"
#define MSG_PARTITIONINGFAILED_STR "Partitioning of disk failed. InstallAROS\nwill now quit."
#define MSG_GRUBNONFFSWARNING_STR "You have selected a filesystem different\nthan FFS-Intl for the system partition. Since\nthis distribution of AROS is compiled with the\noriginal GRUB bootloader, you will not be able to\nboot the system after installation.\n\nShould the partitioning continue?\n"
#define MSG_CANCELINSTALL_STR "Cancel Installation..."
#define MSG_CONTINUECANCELINST_STR "*Continue Install|Cancel Install"
#define MSG_CONTINUECANCELPART_STR "Continue Partitioning|*Cancel Partitionin"
#define MSG_CONTINUEQUIT_STR "Continue|*Quit"
#define MSG_EXISTSRETRY_STR "File Already Exists\nReplace %s?"
#define MSG_RETRY_STR "Retry"
#define MSG_SCANRETRY_STR "Error scanning %s\nRetry?"
#define MSG_FINDRETRY_STR "Could not find %s\nRetry?"
#define MSG_COULDNTOPEN_STR "Couldn't Open %s"
#define MSG_COULDNTWRITE_STR "Couldn't Write to %s"
#define MSG_DISKFULL_STR "Couldn't Write to %s\nDisk Full!"
#define MSG_TEXT_STR "Text"
#define MSG_GFX_STR "Graphics"
#define MSG_YES_STR "Yes"
#define MSG_YESALWAYS_STR "Yes [Always]"
#define MSG_NO_STR "No"
#define MSG_SKIP_STR "Skip"
#define MSG_OK_STR "OK"
#define MSG_CANCEL_STR "Cancel"
#define MSG_CANCEL2_STR "_Cancel"
#define MSG_QUIT_STR "Quit"
#define MSG_QUIT2_STR "QUIT"
#define MSG_ERROR_STR "Error"
#define MSG_WARNING_STR "Warning"
#define MSG_REBOOT_STR "Reboot AROS now"
#define MSG_IOERROR_STR "IO Error has occured"
#define MSG_NOTFILESYSTEM_STR "device '%s' doesn't contain a file system\n"
#define MSG_NOTDEVICE_STR "'%s' doesn't contain a device name\n"

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
  {MSG_DESCRIPTION,(STRPTR)MSG_DESCRIPTION_STR},
  {MSG_WELCOME,(STRPTR)MSG_WELCOME_STR},
  {MSG_INSTALLOPTIONS,(STRPTR)MSG_INSTALLOPTIONS_STR},
  {MSG_DESTOPTIONS,(STRPTR)MSG_DESTOPTIONS_STR},
  {MSG_ACCEPT,(STRPTR)MSG_ACCEPT_STR},
  {MSG_DRIVE,(STRPTR)MSG_DRIVE_STR},
  {MSG_TYPE,(STRPTR)MSG_TYPE_STR},
  {MSG_DEVICE,(STRPTR)MSG_DEVICE_STR},
  {MSG_UNIT,(STRPTR)MSG_UNIT_STR},
  {MSG_NAME,(STRPTR)MSG_NAME_STR},
  {MSG_FILESYSTEM,(STRPTR)MSG_FILESYSTEM_STR},
  {MSG_SIZE,(STRPTR)MSG_SIZE_STR},
  {MSG_MENUMODE,(STRPTR)MSG_MENUMODE_STR},
  {MSG_CREATE,(STRPTR)MSG_CREATE_STR},
  {MSG_SPECIFYSIZE,(STRPTR)MSG_SPECIFYSIZE_STR},
  {MSG_DESTVOLUME,(STRPTR)MSG_DESTVOLUME_STR},
  {MSG_WORKVOLUME,(STRPTR)MSG_WORKVOLUME_STR},
  {MSG_CHOOSELANG,(STRPTR)MSG_CHOOSELANG_STR},
  {MSG_INSTALLCORE,(STRPTR)MSG_INSTALLCORE_STR},
  {MSG_INSTALLEXTRA,(STRPTR)MSG_INSTALLEXTRA_STR},
  {MSG_INSTALLDEVEL,(STRPTR)MSG_INSTALLDEVEL_STR},
  {MSG_INSTALLBOOT,(STRPTR)MSG_INSTALLBOOT_STR},
  {MSG_FORMATPART,(STRPTR)MSG_FORMATPART_STR},
  {MSG_USEWORK,(STRPTR)MSG_USEWORK_STR},
  {MSG_USEWORKFOR,(STRPTR)MSG_USEWORKFOR_STR},
  {MSG_BEGINWITHPARTITION,(STRPTR)MSG_BEGINWITHPARTITION_STR},
  {MSG_BEGINWITHOUTPARTITION,(STRPTR)MSG_BEGINWITHOUTPARTITION_STR},
  {MSG_PARTITIONOPTIONS,(STRPTR)MSG_PARTITIONOPTIONS_STR},
  {MSG_DESTPARTITION,(STRPTR)MSG_DESTPARTITION_STR},
  {MSG_WORKPARTITION,(STRPTR)MSG_WORKPARTITION_STR},
  {MSG_USEFREE,(STRPTR)MSG_USEFREE_STR},
  {MSG_WIPEDISK,(STRPTR)MSG_WIPEDISK_STR},
  {MSG_USEEXISTING,(STRPTR)MSG_USEEXISTING_STR},
  {MSG_PARTITIONTOOBIG,(STRPTR)MSG_PARTITIONTOOBIG_STR},
  {MSG_GRUBOPTIONS,(STRPTR)MSG_GRUBOPTIONS_STR},
  {MSG_GRUBDRIVE,(STRPTR)MSG_GRUBDRIVE_STR},
  {MSG_GRUBGOPTIONS,(STRPTR)MSG_GRUBGOPTIONS_STR},
  {MSG_GRUBGRUB,(STRPTR)MSG_GRUBGRUB_STR},
  {MSG_NOGRUBDEVICE,(STRPTR)MSG_NOGRUBDEVICE_STR},
  {MSG_PARTITIONING,(STRPTR)MSG_PARTITIONING_STR},
  {MSG_FORMATTING,(STRPTR)MSG_FORMATTING_STR},
  {MSG_INSTALL,(STRPTR)MSG_INSTALL_STR},
  {MSG_COPYCORE,(STRPTR)MSG_COPYCORE_STR},
  {MSG_COPYEXTRA,(STRPTR)MSG_COPYEXTRA_STR},
  {MSG_COPYDEVEL,(STRPTR)MSG_COPYDEVEL_STR},
  {MSG_COPYBOOT,(STRPTR)MSG_COPYBOOT_STR},
  {MSG_BOOTLOADER,(STRPTR)MSG_BOOTLOADER_STR},
  {MSG_POSTINSTALL,(STRPTR)MSG_POSTINSTALL_STR},
  {MSG_POSTINSTALL2,(STRPTR)MSG_POSTINSTALL2_STR},
  {MSG_DONEREBOOT,(STRPTR)MSG_DONEREBOOT_STR},
  {MSG_DONEUSB,(STRPTR)MSG_DONEUSB_STR},
  {MSG_INSTALLING,(STRPTR)MSG_INSTALLING_STR},
  {MSG_INSTALLINGBOOT,(STRPTR)MSG_INSTALLINGBOOT_STR},
  {MSG_DONE,(STRPTR)MSG_DONE_STR},
  {MSG_CANCELOK,(STRPTR)MSG_CANCELOK_STR},
  {MSG_CANCELDANGER,(STRPTR)MSG_CANCELDANGER_STR},
  {MSG_BACK,(STRPTR)MSG_BACK_STR},
  {MSG_PROCEED,(STRPTR)MSG_PROCEED_STR},
  {MSG_NOTALLFILESCOPIED,(STRPTR)MSG_NOTALLFILESCOPIED_STR},
  {MSG_PARTITIONINGFAILED,(STRPTR)MSG_PARTITIONINGFAILED_STR},
  {MSG_GRUBNONFFSWARNING,(STRPTR)MSG_GRUBNONFFSWARNING_STR},
  {MSG_CANCELINSTALL,(STRPTR)MSG_CANCELINSTALL_STR},
  {MSG_CONTINUECANCELINST,(STRPTR)MSG_CONTINUECANCELINST_STR},
  {MSG_CONTINUECANCELPART,(STRPTR)MSG_CONTINUECANCELPART_STR},
  {MSG_CONTINUEQUIT,(STRPTR)MSG_CONTINUEQUIT_STR},
  {MSG_EXISTSRETRY,(STRPTR)MSG_EXISTSRETRY_STR},
  {MSG_RETRY,(STRPTR)MSG_RETRY_STR},
  {MSG_SCANRETRY,(STRPTR)MSG_SCANRETRY_STR},
  {MSG_FINDRETRY,(STRPTR)MSG_FINDRETRY_STR},
  {MSG_COULDNTOPEN,(STRPTR)MSG_COULDNTOPEN_STR},
  {MSG_COULDNTWRITE,(STRPTR)MSG_COULDNTWRITE_STR},
  {MSG_DISKFULL,(STRPTR)MSG_DISKFULL_STR},
  {MSG_TEXT,(STRPTR)MSG_TEXT_STR},
  {MSG_GFX,(STRPTR)MSG_GFX_STR},
  {MSG_YES,(STRPTR)MSG_YES_STR},
  {MSG_YESALWAYS,(STRPTR)MSG_YESALWAYS_STR},
  {MSG_NO,(STRPTR)MSG_NO_STR},
  {MSG_SKIP,(STRPTR)MSG_SKIP_STR},
  {MSG_OK,(STRPTR)MSG_OK_STR},
  {MSG_CANCEL,(STRPTR)MSG_CANCEL_STR},
  {MSG_CANCEL2,(STRPTR)MSG_CANCEL2_STR},
  {MSG_QUIT,(STRPTR)MSG_QUIT_STR},
  {MSG_QUIT2,(STRPTR)MSG_QUIT2_STR},
  {MSG_ERROR,(STRPTR)MSG_ERROR_STR},
  {MSG_WARNING,(STRPTR)MSG_WARNING_STR},
  {MSG_REBOOT,(STRPTR)MSG_REBOOT_STR},
  {MSG_IOERROR,(STRPTR)MSG_IOERROR_STR},
  {MSG_NOTFILESYSTEM,(STRPTR)MSG_NOTFILESYSTEM_STR},
  {MSG_NOTDEVICE,(STRPTR)MSG_NOTDEVICE_STR},
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


#endif /* INSTALLAROS_STRINGS_H */
