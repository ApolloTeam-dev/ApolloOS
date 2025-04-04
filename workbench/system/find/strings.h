/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/system/find/catalogs/find.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef FIND_STRINGS_H
#define FIND_STRINGS_H


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

#define MSG_APP_TITLE 0
#define MSG_APP_DESCRIPTION 1
#define MSG_APP_COPYRIGHT 2
#define MSG_WI_TITLE 3
#define MSG_OK 4
#define MSG_LST_FULLPATH 5
#define MSG_LST_SIZE 6
#define MSG_LST_ATTRIBUTES 7
#define MSG_LST_DATE 8
#define MSG_LST_TIME 9
#define MSG_LST_COMMENT 10
#define MSG_LBL_PATH 11
#define MSG_LBL_PATTERN 12
#define MSG_LBL_CONTENTS 13
#define MSG_BTN_START 14
#define MSG_BTN_STOP 15
#define MSG_BTN_OPEN 16
#define MSG_BTN_VIEW 17
#define MSG_BTN_DRAWER 18
#define MSG_ERR 19
#define MSG_ERR_NO_APPLICATION 20
#define MSG_ERR_READARGS 21
#define MSG_ERR_NO_MEM 22
#define MSG_ERR_NO_WINDOW 23
#define MSG_ERR_DOS 24
#define MSG_ERR_NO_FILE 25
#define MSG_ERR_NO_DIR 26
#define MSG_ERR_PATTERN 27

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_APP_TITLE_STR "Find"
#define MSG_APP_DESCRIPTION_STR "File search tool"
#define MSG_APP_COPYRIGHT_STR "Copyright © 2016 The AROS Development Team"
#define MSG_WI_TITLE_STR "Find"
#define MSG_OK_STR "OK"
#define MSG_LST_FULLPATH_STR "Full Path"
#define MSG_LST_SIZE_STR "Size"
#define MSG_LST_ATTRIBUTES_STR "Attributes"
#define MSG_LST_DATE_STR "Date"
#define MSG_LST_TIME_STR "Time"
#define MSG_LST_COMMENT_STR "Comment"
#define MSG_LBL_PATH_STR "Path"
#define MSG_LBL_PATTERN_STR "Pattern"
#define MSG_LBL_CONTENTS_STR "Contents"
#define MSG_BTN_START_STR "Start"
#define MSG_BTN_STOP_STR "Stop"
#define MSG_BTN_OPEN_STR "Open"
#define MSG_BTN_VIEW_STR "View"
#define MSG_BTN_DRAWER_STR "Drawer"
#define MSG_ERR_STR "Error"
#define MSG_ERR_NO_APPLICATION_STR "Can't create application."
#define MSG_ERR_READARGS_STR "Can't read commandline arguments."
#define MSG_ERR_NO_MEM_STR "Error:\nCan't allocate memory."
#define MSG_ERR_NO_WINDOW_STR "Error:\nCan't open window."
#define MSG_ERR_DOS_STR "DOS Error:\n%s"
#define MSG_ERR_NO_FILE_STR "Error:\nCan't open file\n\"%s\"."
#define MSG_ERR_NO_DIR_STR "Error:\nCan't open directory\n\"%s\"."
#define MSG_ERR_PATTERN_STR "Error:\nCan't parse pattern."

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
  {MSG_APP_TITLE,(STRPTR)MSG_APP_TITLE_STR},
  {MSG_APP_DESCRIPTION,(STRPTR)MSG_APP_DESCRIPTION_STR},
  {MSG_APP_COPYRIGHT,(STRPTR)MSG_APP_COPYRIGHT_STR},
  {MSG_WI_TITLE,(STRPTR)MSG_WI_TITLE_STR},
  {MSG_OK,(STRPTR)MSG_OK_STR},
  {MSG_LST_FULLPATH,(STRPTR)MSG_LST_FULLPATH_STR},
  {MSG_LST_SIZE,(STRPTR)MSG_LST_SIZE_STR},
  {MSG_LST_ATTRIBUTES,(STRPTR)MSG_LST_ATTRIBUTES_STR},
  {MSG_LST_DATE,(STRPTR)MSG_LST_DATE_STR},
  {MSG_LST_TIME,(STRPTR)MSG_LST_TIME_STR},
  {MSG_LST_COMMENT,(STRPTR)MSG_LST_COMMENT_STR},
  {MSG_LBL_PATH,(STRPTR)MSG_LBL_PATH_STR},
  {MSG_LBL_PATTERN,(STRPTR)MSG_LBL_PATTERN_STR},
  {MSG_LBL_CONTENTS,(STRPTR)MSG_LBL_CONTENTS_STR},
  {MSG_BTN_START,(STRPTR)MSG_BTN_START_STR},
  {MSG_BTN_STOP,(STRPTR)MSG_BTN_STOP_STR},
  {MSG_BTN_OPEN,(STRPTR)MSG_BTN_OPEN_STR},
  {MSG_BTN_VIEW,(STRPTR)MSG_BTN_VIEW_STR},
  {MSG_BTN_DRAWER,(STRPTR)MSG_BTN_DRAWER_STR},
  {MSG_ERR,(STRPTR)MSG_ERR_STR},
  {MSG_ERR_NO_APPLICATION,(STRPTR)MSG_ERR_NO_APPLICATION_STR},
  {MSG_ERR_READARGS,(STRPTR)MSG_ERR_READARGS_STR},
  {MSG_ERR_NO_MEM,(STRPTR)MSG_ERR_NO_MEM_STR},
  {MSG_ERR_NO_WINDOW,(STRPTR)MSG_ERR_NO_WINDOW_STR},
  {MSG_ERR_DOS,(STRPTR)MSG_ERR_DOS_STR},
  {MSG_ERR_NO_FILE,(STRPTR)MSG_ERR_NO_FILE_STR},
  {MSG_ERR_NO_DIR,(STRPTR)MSG_ERR_NO_DIR_STR},
  {MSG_ERR_PATTERN,(STRPTR)MSG_ERR_PATTERN_STR},
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


#endif /* FIND_STRINGS_H */
