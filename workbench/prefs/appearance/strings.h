/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/prefs/appearance/catalogs/appearance.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef APPEARANCE_STRINGS_H
#define APPEARANCE_STRINGS_H


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

#define MSG_WINTITLE 0
#define MSG_OK 1
#define MSG_ENABLETHEMES 2
#define MSG_PREVIEW 3
#define MSG_NOPREVIEW 4
#define MSG_SELECTEDTHEME 5
#define MSG_ENABLEZUNEPREFS 6
#define MSG_ENABLEWANDPREFS 7
#define MSG_SEC_THEMING 8
#define MSG_SEC_COMPOSITING 9
#define MSG_ENABLE_SCREEN_COMPOSITION 10
#define MSG_COMPOSITE_BELOW 11
#define MSG_COMPOSITE_LEFT 12
#define MSG_COMPOSITE_RIGHT 13
#define MSG_ENABLE_COMPOSITE_WITH_ALPHA 14

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_WINTITLE_STR "Appearance Preferences"
#define MSG_OK_STR "Ok"
#define MSG_ENABLETHEMES_STR "Enable theming"
#define MSG_PREVIEW_STR "Preview"
#define MSG_NOPREVIEW_STR "-Preview Unavailable-"
#define MSG_SELECTEDTHEME_STR "Theme:"
#define MSG_ENABLEZUNEPREFS_STR "Override Zune configuration"
#define MSG_ENABLEWANDPREFS_STR "Inherit Wanderer Configuration?"
#define MSG_SEC_THEMING_STR "Theming"
#define MSG_SEC_COMPOSITING_STR "Compositing"
#define MSG_ENABLE_SCREEN_COMPOSITION_STR "Enable screen composition"
#define MSG_COMPOSITE_BELOW_STR "Composite below"
#define MSG_COMPOSITE_LEFT_STR "Composite left"
#define MSG_COMPOSITE_RIGHT_STR "Composite right"
#define MSG_ENABLE_COMPOSITE_WITH_ALPHA_STR "Enable compositing with alpha"

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
  {MSG_WINTITLE,(STRPTR)MSG_WINTITLE_STR},
  {MSG_OK,(STRPTR)MSG_OK_STR},
  {MSG_ENABLETHEMES,(STRPTR)MSG_ENABLETHEMES_STR},
  {MSG_PREVIEW,(STRPTR)MSG_PREVIEW_STR},
  {MSG_NOPREVIEW,(STRPTR)MSG_NOPREVIEW_STR},
  {MSG_SELECTEDTHEME,(STRPTR)MSG_SELECTEDTHEME_STR},
  {MSG_ENABLEZUNEPREFS,(STRPTR)MSG_ENABLEZUNEPREFS_STR},
  {MSG_ENABLEWANDPREFS,(STRPTR)MSG_ENABLEWANDPREFS_STR},
  {MSG_SEC_THEMING,(STRPTR)MSG_SEC_THEMING_STR},
  {MSG_SEC_COMPOSITING,(STRPTR)MSG_SEC_COMPOSITING_STR},
  {MSG_ENABLE_SCREEN_COMPOSITION,(STRPTR)MSG_ENABLE_SCREEN_COMPOSITION_STR},
  {MSG_COMPOSITE_BELOW,(STRPTR)MSG_COMPOSITE_BELOW_STR},
  {MSG_COMPOSITE_LEFT,(STRPTR)MSG_COMPOSITE_LEFT_STR},
  {MSG_COMPOSITE_RIGHT,(STRPTR)MSG_COMPOSITE_RIGHT_STR},
  {MSG_ENABLE_COMPOSITE_WITH_ALPHA,(STRPTR)MSG_ENABLE_COMPOSITE_WITH_ALPHA_STR},
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


#endif /* APPEARANCE_STRINGS_H */
