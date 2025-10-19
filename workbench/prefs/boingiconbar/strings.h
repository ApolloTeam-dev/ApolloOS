/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/prefs/boingiconbar/catalogs/boingiconbar.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef BOINGICONBAR_STRINGS_H
#define BOINGICONBAR_STRINGS_H


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

#define MSG_APP 0
#define MSG_DESCRIPTION 1
#define MSG_TITLE 2
#define MSG_DOCK 3
#define MSG_ADD_DOCK 4
#define MSG_DEL_DOCK 5
#define MSG_SAVE 6
#define MSG_PROGRAMS 7
#define MSG_ADD_P 8
#define MSG_DEL_P 9
#define MSG_OK 10

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_APP_STR "BIBPrefs"
#define MSG_DESCRIPTION_STR "BoingIconBar Preferences"
#define MSG_TITLE_STR "BoingIconBar Preferences"
#define MSG_DOCK_STR "\033cDock"
#define MSG_ADD_DOCK_STR "Add"
#define MSG_DEL_DOCK_STR "Delete"
#define MSG_SAVE_STR "Save"
#define MSG_PROGRAMS_STR "\033cPrograms"
#define MSG_ADD_P_STR "Add program"
#define MSG_DEL_P_STR "Del program"
#define MSG_OK_STR "OK"

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
  {MSG_APP,(STRPTR)MSG_APP_STR},
  {MSG_DESCRIPTION,(STRPTR)MSG_DESCRIPTION_STR},
  {MSG_TITLE,(STRPTR)MSG_TITLE_STR},
  {MSG_DOCK,(STRPTR)MSG_DOCK_STR},
  {MSG_ADD_DOCK,(STRPTR)MSG_ADD_DOCK_STR},
  {MSG_DEL_DOCK,(STRPTR)MSG_DEL_DOCK_STR},
  {MSG_SAVE,(STRPTR)MSG_SAVE_STR},
  {MSG_PROGRAMS,(STRPTR)MSG_PROGRAMS_STR},
  {MSG_ADD_P,(STRPTR)MSG_ADD_P_STR},
  {MSG_DEL_P,(STRPTR)MSG_DEL_P_STR},
  {MSG_OK,(STRPTR)MSG_OK_STR},
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


#endif /* BOINGICONBAR_STRINGS_H */
