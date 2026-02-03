/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/tools/BoingIconBar/catalogs/boingiconbar.cd".

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

#define MSG_CX_NAME 0
#define MSG_CX_TITLE 1
#define MSG_CX_DESC 2
#define MSG_MENU_SETTINGS 3
#define MSG_MENU_QUIT 4

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_CX_NAME_STR "BoingIconBar"
#define MSG_CX_TITLE_STR "Icon Toolbar"
#define MSG_CX_DESC_STR "Start Programs from Toolbar"
#define MSG_MENU_SETTINGS_STR "Settings"
#define MSG_MENU_QUIT_STR "Quit"

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
  {MSG_CX_NAME,(STRPTR)MSG_CX_NAME_STR},
  {MSG_CX_TITLE,(STRPTR)MSG_CX_TITLE_STR},
  {MSG_CX_DESC,(STRPTR)MSG_CX_DESC_STR},
  {MSG_MENU_SETTINGS,(STRPTR)MSG_MENU_SETTINGS_STR},
  {MSG_MENU_QUIT,(STRPTR)MSG_MENU_QUIT_STR},
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
