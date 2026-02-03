/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/devs/diskimage/zune_gui/catalogs/diskimagegui.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef DISKIMAGEGUI_STRINGS_H
#define DISKIMAGEGUI_STRINGS_H


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

#define MSG_UNIT_LBL 0
#define MSG_DEVICE_LBL 1
#define MSG_FILENAME_LBL 2
#define MSG_WRITEPROTECT_LBL 3
#define MSG_NO_DISK 4
#define MSG_APPDESCRIPTION 5
#define MSG_PRIORITY_LBL 50
#define MSG_PLUGIN_LBL 51
#define MSG_PLUGIN_GAD 52
#define MSG_AUTO_LBL 53
#define MSG_WRITESUPPORT_LBL 54
#define MSG_PROJECT_ABOUT 100
#define MSG_PROJECT_HIDE 101
#define MSG_PROJECT_ICONIFY 102
#define MSG_PROJECT_SNAPSHOT 103
#define MSG_PROJECT_QUIT 104
#define MSG_SETTINGS_MENU 105
#define MSG_SETTINGS_CHANGETEMPDIR 106
#define MSG_SETTINGS_SAVE 107
#define MSG_SETTINGS_PLUGINS 108
#define MSG_PROJECT_CLOSE 109
#define MSG_OK_GAD 200
#define MSG_ABOUT_REQ 201
#define MSG_ERROR 202
#define MSG_NOAISS_REQ 203
#define MSG_OLDAISS_REQ 204
#define MSG_IMAGENOTFOUND_REQ 205
#define MSG_CANCEL_GAD 206
#define MSG_SAVE_GAD 207
#define MSG_USE_GAD 208
#define MSG_INSERT_GAD 300
#define MSG_EJECT_GAD 301
#define MSG_WRITEPROTECT_GAD 302
#define MSG_REFRESH_GAD 303
#define MSG_PLUGINS_WND 304
#define MSG_MAIN_WND 305
#define MSG_ABOUT_WND 306
#define MSG_ERROR_WND 307
#define MSG_SETDEVICETYPE_GAD 308
#define MSG_SETDEVICETYPE_WND 309
#define MSG_DEVICETYPE_GAD 310
#define MSG_DEVICETYPE_DIRECT_ACCESS 311
#define MSG_DEVICETYPE_CDROM 312
#define MSG_DEVICETYPE_OPTICAL_DISK 313

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_UNIT_LBL_STR "Unit"
#define MSG_DEVICE_LBL_STR "Device"
#define MSG_FILENAME_LBL_STR "Disk Image"
#define MSG_WRITEPROTECT_LBL_STR "WP"
#define MSG_NO_DISK_STR "No disk present"
#define MSG_APPDESCRIPTION_STR "Control GUI for diskimage.device"
#define MSG_PRIORITY_LBL_STR "Pri"
#define MSG_PLUGIN_LBL_STR "Plugin"
#define MSG_PLUGIN_GAD_STR "Use plugin"
#define MSG_AUTO_LBL_STR "(Auto)"
#define MSG_WRITESUPPORT_LBL_STR "W"
#define MSG_PROJECT_ABOUT_STR "About..."
#define MSG_PROJECT_HIDE_STR "Hide"
#define MSG_PROJECT_ICONIFY_STR "Iconify"
#define MSG_PROJECT_SNAPSHOT_STR "Snapshot"
#define MSG_PROJECT_QUIT_STR "Quit"
#define MSG_SETTINGS_MENU_STR "Settings"
#define MSG_SETTINGS_CHANGETEMPDIR_STR "Change temp directory..."
#define MSG_SETTINGS_SAVE_STR "Save settings"
#define MSG_SETTINGS_PLUGINS_STR "Plugins..."
#define MSG_PROJECT_CLOSE_STR "Close"
#define MSG_OK_GAD_STR "Ok"
#define MSG_ABOUT_REQ_STR "%s version %ld.%ld\n"\
	"%s version %ld.%ld\n\n"\
	"Copyright (C) 2007-2010 Fredrik Wikstrom\n"\
	"Website: http://a500.org\n"\
	"E-mail: fredrik@a500.org"
#define MSG_ERROR_STR "Error"
#define MSG_NOAISS_REQ_STR "AISS assign not found.\n\n"\
	"Download and install the latest version\n"\
	"from here: http://masonicons.de"
#define MSG_OLDAISS_REQ_STR "AISS v%ld.%ld or newer required.\n\n"\
	"Download and install the latest version\n"\
	"from here: http://masonicons.de"
#define MSG_IMAGENOTFOUND_REQ_STR "Image file '%s' not found."
#define MSG_CANCEL_GAD_STR "_Cancel"
#define MSG_SAVE_GAD_STR "_Save"
#define MSG_USE_GAD_STR "_Use"
#define MSG_INSERT_GAD_STR "Insert"
#define MSG_EJECT_GAD_STR "Eject"
#define MSG_WRITEPROTECT_GAD_STR "Write Protect"
#define MSG_REFRESH_GAD_STR "Refresh"
#define MSG_PLUGINS_WND_STR "Plugins"
#define MSG_MAIN_WND_STR "%s: Hot Key = <%s>"
#define MSG_ABOUT_WND_STR "About - %s"
#define MSG_ERROR_WND_STR "Error - %s"
#define MSG_SETDEVICETYPE_GAD_STR "Set Device Type"
#define MSG_SETDEVICETYPE_WND_STR "Unit: %ld - Device Type"
#define MSG_DEVICETYPE_GAD_STR "Device Type"
#define MSG_DEVICETYPE_DIRECT_ACCESS_STR "Direct Access (Default setting)"
#define MSG_DEVICETYPE_CDROM_STR "CD-ROM"
#define MSG_DEVICETYPE_OPTICAL_DISK_STR "Optical Disk"

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
  {MSG_UNIT_LBL,(STRPTR)MSG_UNIT_LBL_STR},
  {MSG_DEVICE_LBL,(STRPTR)MSG_DEVICE_LBL_STR},
  {MSG_FILENAME_LBL,(STRPTR)MSG_FILENAME_LBL_STR},
  {MSG_WRITEPROTECT_LBL,(STRPTR)MSG_WRITEPROTECT_LBL_STR},
  {MSG_NO_DISK,(STRPTR)MSG_NO_DISK_STR},
  {MSG_APPDESCRIPTION,(STRPTR)MSG_APPDESCRIPTION_STR},
  {MSG_PRIORITY_LBL,(STRPTR)MSG_PRIORITY_LBL_STR},
  {MSG_PLUGIN_LBL,(STRPTR)MSG_PLUGIN_LBL_STR},
  {MSG_PLUGIN_GAD,(STRPTR)MSG_PLUGIN_GAD_STR},
  {MSG_AUTO_LBL,(STRPTR)MSG_AUTO_LBL_STR},
  {MSG_WRITESUPPORT_LBL,(STRPTR)MSG_WRITESUPPORT_LBL_STR},
  {MSG_PROJECT_ABOUT,(STRPTR)MSG_PROJECT_ABOUT_STR},
  {MSG_PROJECT_HIDE,(STRPTR)MSG_PROJECT_HIDE_STR},
  {MSG_PROJECT_ICONIFY,(STRPTR)MSG_PROJECT_ICONIFY_STR},
  {MSG_PROJECT_SNAPSHOT,(STRPTR)MSG_PROJECT_SNAPSHOT_STR},
  {MSG_PROJECT_QUIT,(STRPTR)MSG_PROJECT_QUIT_STR},
  {MSG_SETTINGS_MENU,(STRPTR)MSG_SETTINGS_MENU_STR},
  {MSG_SETTINGS_CHANGETEMPDIR,(STRPTR)MSG_SETTINGS_CHANGETEMPDIR_STR},
  {MSG_SETTINGS_SAVE,(STRPTR)MSG_SETTINGS_SAVE_STR},
  {MSG_SETTINGS_PLUGINS,(STRPTR)MSG_SETTINGS_PLUGINS_STR},
  {MSG_PROJECT_CLOSE,(STRPTR)MSG_PROJECT_CLOSE_STR},
  {MSG_OK_GAD,(STRPTR)MSG_OK_GAD_STR},
  {MSG_ABOUT_REQ,(STRPTR)MSG_ABOUT_REQ_STR},
  {MSG_ERROR,(STRPTR)MSG_ERROR_STR},
  {MSG_NOAISS_REQ,(STRPTR)MSG_NOAISS_REQ_STR},
  {MSG_OLDAISS_REQ,(STRPTR)MSG_OLDAISS_REQ_STR},
  {MSG_IMAGENOTFOUND_REQ,(STRPTR)MSG_IMAGENOTFOUND_REQ_STR},
  {MSG_CANCEL_GAD,(STRPTR)MSG_CANCEL_GAD_STR},
  {MSG_SAVE_GAD,(STRPTR)MSG_SAVE_GAD_STR},
  {MSG_USE_GAD,(STRPTR)MSG_USE_GAD_STR},
  {MSG_INSERT_GAD,(STRPTR)MSG_INSERT_GAD_STR},
  {MSG_EJECT_GAD,(STRPTR)MSG_EJECT_GAD_STR},
  {MSG_WRITEPROTECT_GAD,(STRPTR)MSG_WRITEPROTECT_GAD_STR},
  {MSG_REFRESH_GAD,(STRPTR)MSG_REFRESH_GAD_STR},
  {MSG_PLUGINS_WND,(STRPTR)MSG_PLUGINS_WND_STR},
  {MSG_MAIN_WND,(STRPTR)MSG_MAIN_WND_STR},
  {MSG_ABOUT_WND,(STRPTR)MSG_ABOUT_WND_STR},
  {MSG_ERROR_WND,(STRPTR)MSG_ERROR_WND_STR},
  {MSG_SETDEVICETYPE_GAD,(STRPTR)MSG_SETDEVICETYPE_GAD_STR},
  {MSG_SETDEVICETYPE_WND,(STRPTR)MSG_SETDEVICETYPE_WND_STR},
  {MSG_DEVICETYPE_GAD,(STRPTR)MSG_DEVICETYPE_GAD_STR},
  {MSG_DEVICETYPE_DIRECT_ACCESS,(STRPTR)MSG_DEVICETYPE_DIRECT_ACCESS_STR},
  {MSG_DEVICETYPE_CDROM,(STRPTR)MSG_DEVICETYPE_CDROM_STR},
  {MSG_DEVICETYPE_OPTICAL_DISK,(STRPTR)MSG_DEVICETYPE_OPTICAL_DISK_STR},
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


#endif /* DISKIMAGEGUI_STRINGS_H */
