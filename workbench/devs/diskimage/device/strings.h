/****************************************************************
   This file was created automatically by `FlexCat 2.19'
   from "/home/willem/ApolloOS/workbench/devs/diskimage/device/catalogs/diskimagedevice.cd".

   Do NOT edit by hand!
****************************************************************/

#ifndef DISKIMAGEDEVICE_STRINGS_H
#define DISKIMAGEDEVICE_STRINGS_H


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

#define MSG_OK_GAD 0
#define MSG_CANCEL_GAD 1
#define MSG_PASSWORD_REQ 2
#define MSG_NOZLIB 3
#define MSG_NOCAPSDEV 4
#define MSG_CANCELED 5
#define MSG_NOPASSWD 6
#define MSG_WRONGPASSWD 7
#define MSG_ZLIBERR 8
#define MSG_CAPSERR 9
#define MSG_XPKERR 10
#define MSG_XADERR 11
#define MSG_UNKNDISKIMGTYPE 12
#define MSG_UNKNCOMPMETHOD 13
#define MSG_EOF 14
#define MSG_BADDATA 15
#define MSG_BADCRC 16
#define MSG_BADCHECKSUM 17
#define MSG_REQ 18
#define MSG_REQVER 19
#define MSG_WRONGDAA 20
#define MSG_EXPATERR 21
#define MSG_BZLIBERR 22
#define MSG_UNKNENCRMETHOD 23

#endif /* CATCOMP_NUMBERS */

/***************************************************************/

#ifdef CATCOMP_STRINGS

#define MSG_OK_GAD_STR "_Ok"
#define MSG_CANCEL_GAD_STR "_Cancel"
#define MSG_PASSWORD_REQ_STR "Enter password:"
#define MSG_NOZLIB_STR "z.library required"
#define MSG_NOCAPSDEV_STR "capsimage.device required"
#define MSG_CANCELED_STR "operation canceled"
#define MSG_NOPASSWD_STR "password required"
#define MSG_WRONGPASSWD_STR "wrong password"
#define MSG_ZLIBERR_STR "z.library error"
#define MSG_CAPSERR_STR "capsimage.device error"
#define MSG_XPKERR_STR "xpkmaster.library error"
#define MSG_XADERR_STR "xadmaster.library error"
#define MSG_UNKNDISKIMGTYPE_STR "unsupported disk image type/format"
#define MSG_UNKNCOMPMETHOD_STR "unsupported compression method"
#define MSG_EOF_STR "unexpected end of file"
#define MSG_BADDATA_STR "file contains bad data"
#define MSG_BADCRC_STR "CRC check on file data failed"
#define MSG_BADCHECKSUM_STR "checksum error"
#define MSG_REQ_STR "%s required"
#define MSG_REQVER_STR "%s v%ld or newer required"
#define MSG_WRONGDAA_STR "wrong DAA file (multi-file DAA)"
#define MSG_EXPATERR_STR "expat.library error"
#define MSG_BZLIBERR_STR "bz2.library error"
#define MSG_UNKNENCRMETHOD_STR "unsupported encryption method"

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
  {MSG_OK_GAD,(STRPTR)MSG_OK_GAD_STR},
  {MSG_CANCEL_GAD,(STRPTR)MSG_CANCEL_GAD_STR},
  {MSG_PASSWORD_REQ,(STRPTR)MSG_PASSWORD_REQ_STR},
  {MSG_NOZLIB,(STRPTR)MSG_NOZLIB_STR},
  {MSG_NOCAPSDEV,(STRPTR)MSG_NOCAPSDEV_STR},
  {MSG_CANCELED,(STRPTR)MSG_CANCELED_STR},
  {MSG_NOPASSWD,(STRPTR)MSG_NOPASSWD_STR},
  {MSG_WRONGPASSWD,(STRPTR)MSG_WRONGPASSWD_STR},
  {MSG_ZLIBERR,(STRPTR)MSG_ZLIBERR_STR},
  {MSG_CAPSERR,(STRPTR)MSG_CAPSERR_STR},
  {MSG_XPKERR,(STRPTR)MSG_XPKERR_STR},
  {MSG_XADERR,(STRPTR)MSG_XADERR_STR},
  {MSG_UNKNDISKIMGTYPE,(STRPTR)MSG_UNKNDISKIMGTYPE_STR},
  {MSG_UNKNCOMPMETHOD,(STRPTR)MSG_UNKNCOMPMETHOD_STR},
  {MSG_EOF,(STRPTR)MSG_EOF_STR},
  {MSG_BADDATA,(STRPTR)MSG_BADDATA_STR},
  {MSG_BADCRC,(STRPTR)MSG_BADCRC_STR},
  {MSG_BADCHECKSUM,(STRPTR)MSG_BADCHECKSUM_STR},
  {MSG_REQ,(STRPTR)MSG_REQ_STR},
  {MSG_REQVER,(STRPTR)MSG_REQVER_STR},
  {MSG_WRONGDAA,(STRPTR)MSG_WRONGDAA_STR},
  {MSG_EXPATERR,(STRPTR)MSG_EXPATERR_STR},
  {MSG_BZLIBERR,(STRPTR)MSG_BZLIBERR_STR},
  {MSG_UNKNENCRMETHOD,(STRPTR)MSG_UNKNENCRMETHOD_STR},
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


#endif /* DISKIMAGEDEVICE_STRINGS_H */
