/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/c/R/catalogs/R.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef R_STRINGS_H
#define R_STRINGS_H


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
#define MSG_WIN_TITLE 2
#define MSG_OK 3
#define MSG_CANCEL 4
#define MSG_ERROR_ALLOCATE 5
#define MSG_ERROR_RETURN 6
#define MSG_ERROR_POOL 7
#define MSG_ERROR_STRUCT 8
#define MSG_ERROR_ARGS 9
#define MSG_ERROR_NOTFOUND 10
#define MSG_ERROR_TMPLT_GET 11
#define MSG_ERROR_TMPLT_PARSE 12
#define MSG_ERROR_GUI 13
#define MSG_ERROR_DEFAULTS 14

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_APP_TITLE_STR "R"
#define MSG_APP_DESCRIPTION_STR "GUI Generator"
#define MSG_WIN_TITLE_STR "R : %s"
#define MSG_OK_STR "OK"
#define MSG_CANCEL_STR "Cancel"
#define MSG_ERROR_ALLOCATE_STR "Can't allocate struct Req\n"
#define MSG_ERROR_RETURN_STR "\"%s\" failed, return code %ld\n"
#define MSG_ERROR_POOL_STR "Can't create poolmem\n"
#define MSG_ERROR_STRUCT_STR "Can't allocate struct Req\n"
#define MSG_ERROR_ARGS_STR "Failed to parse arguments\n"
#define MSG_ERROR_NOTFOUND_STR "Command not found\n"
#define MSG_ERROR_TMPLT_GET_STR "Failed to get template\n"
#define MSG_ERROR_TMPLT_PARSE_STR "Failed to parse the template\n"
#define MSG_ERROR_GUI_STR "Failed to create application object\n"
#define MSG_ERROR_DEFAULTS_STR "Failed to set the default values\n"

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
  {MSG_WIN_TITLE,(STRPTR)MSG_WIN_TITLE_STR},
  {MSG_OK,(STRPTR)MSG_OK_STR},
  {MSG_CANCEL,(STRPTR)MSG_CANCEL_STR},
  {MSG_ERROR_ALLOCATE,(STRPTR)MSG_ERROR_ALLOCATE_STR},
  {MSG_ERROR_RETURN,(STRPTR)MSG_ERROR_RETURN_STR},
  {MSG_ERROR_POOL,(STRPTR)MSG_ERROR_POOL_STR},
  {MSG_ERROR_STRUCT,(STRPTR)MSG_ERROR_STRUCT_STR},
  {MSG_ERROR_ARGS,(STRPTR)MSG_ERROR_ARGS_STR},
  {MSG_ERROR_NOTFOUND,(STRPTR)MSG_ERROR_NOTFOUND_STR},
  {MSG_ERROR_TMPLT_GET,(STRPTR)MSG_ERROR_TMPLT_GET_STR},
  {MSG_ERROR_TMPLT_PARSE,(STRPTR)MSG_ERROR_TMPLT_PARSE_STR},
  {MSG_ERROR_GUI,(STRPTR)MSG_ERROR_GUI_STR},
  {MSG_ERROR_DEFAULTS,(STRPTR)MSG_ERROR_DEFAULTS_STR},
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


#endif /* R_STRINGS_H */
