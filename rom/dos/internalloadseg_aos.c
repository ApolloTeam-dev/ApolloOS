/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    Desc:
*/

#include <aros/debug.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/arossupport.h>

#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dosasl.h>
#include <dos/doshunks.h>
#include <dos/dosextens.h>
#include <aros/asmcall.h>
#include <aros/macros.h>

#include "dos_intern.h"
#include "internalloadseg.h"
#include "include/loadseg.h"

#define GETHUNKPTR(x) ((UBYTE*)(BADDR(hunktab[x]) + sizeof(BPTR)))

#define LOADSEG_HUNK_BUFFER 2048

struct SRBuffer
{
    APTR buffer;
    ULONG offset;
    ULONG avail;
};

static int read_block_buffered(BPTR file, APTR buffer, ULONG size, SIPTR * funcarray, struct SRBuffer * srb, struct DosLibrary * DOSBase)
{
    if (!srb) {
        return ilsRead(file, buffer, size) == size ? 0 : 1;
    }
    if (!srb->buffer) {
        srb->buffer = ilsAllocMem(LOADSEG_HUNK_BUFFER, 0);
        if (!srb->buffer)
            return 1;
        srb->avail = 0;
    }
    while (size) {
        ULONG tocopy;
        if (srb->avail == 0) {
            srb->offset = 0;
            srb->avail = ilsRead(file, srb->buffer, LOADSEG_HUNK_BUFFER);
        }
        if (srb->avail == 0)
            return 1;
        tocopy = srb->avail > size ? size : srb->avail;
        CopyMem(srb->buffer + srb->offset, buffer, tocopy);
        size -= tocopy;
        buffer += tocopy;
        srb->avail -= tocopy;
        srb->offset += tocopy;
    }
    return 0;
}

/* Seek forward by count ULONGs.
 * Returns 0 on success, 1 on failure.
 */
static int seek_forward(BPTR fd, ULONG count, SIPTR *funcarray, struct SRBuffer *srb, struct DosLibrary *DOSBase)
{
    int err = 0;
    ULONG tmp;

    /* For AOS compatibility, we can't use DOS/Seek() here,
     * as AOS callers to InternalLoadSeg will not pass
     * in a Seek element of the funcarray, and the read
     * callback of funcarray may be for reading in-memory
     * instead of pointing to DOS/Read.
     *
     * Luckily, reading HUNKs is linear, so we can just
     * read ahead.
     */
    while (count && !(err = read_block_buffered(fd, &tmp, sizeof(tmp), funcarray, srb, DOSBase)))
        count--;

    return err;
}

BPTR InternalLoadSeg_AOS(BPTR fh,
                         BPTR table,
                         SIPTR * funcarray,
                         LONG  * stacksize,
                         struct DosLibrary * DOSBase)
{
  #define ERROR(a)    { *error=a; goto end; }


  BPTR *hunktab = BADDR(table);
  BPTR firsthunk = BNULL, prevhunk = BNULL;
  ULONG lcount, hunktype, first, last, curhunk, lasthunk, numhunks;
  UWORD wcount;
  LONG t;
  UBYTE name_buf[255];
  register int i;
  BPTR last_p = 0;
  UBYTE *overlaytable = NULL;
  ULONG tmp, req;
  SIPTR dummy;
  struct SRBuffer srbbuf, *srb;
#if DEBUG
  static STRPTR segtypes[] = { "CODE", "DATA", "BSS", };
#endif

  SIPTR *error = &dummy;
  
  if (DOSBase) {
    struct Process *me = (struct Process *)FindTask(NULL);
    ASSERT_VALID_PROCESS(me);

    error =&me->pr_Result2;
  }
  srb = &srbbuf;
  srb->buffer = NULL;

  curhunk = lasthunk = 0; /* keep GCC quiet */
  /* start point is HUNK_HEADER + 4 */
  while (1)
  {
    if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
      goto end;
    if (lcount == 0L)
      break;
    lcount = AROS_BE2LONG(lcount);
    lcount *= 4;
    if (read_block_buffered(fh, name_buf, lcount, funcarray, srb, DOSBase))
      goto end;
    D(bug("\tlibname: \"%.*s\"\n", lcount, name_buf));
  }
  if (read_block_buffered(fh, &numhunks, sizeof(numhunks), funcarray, srb, DOSBase))
    goto end;

  numhunks = AROS_BE2LONG(numhunks);

  D(bug("\tHunk count: %ld\n", numhunks));

  if (!hunktab) {
    hunktab = ilsAllocVec(sizeof(BPTR) * (numhunks + 1 + 1), MEMF_CLEAR);
    if (hunktab == NULL)
      ERROR(ERROR_NO_FREE_STORE);
  }

  if (read_block_buffered(fh, &first, sizeof(first), funcarray, srb, DOSBase))
    goto end;

  first = AROS_BE2LONG(first);

  D(bug("\tFirst hunk: %ld\n", first));
  curhunk = first;
  if (read_block_buffered(fh, &last, sizeof(last), funcarray, srb, DOSBase))
    goto end;

  last = AROS_BE2LONG(last);

  D(bug("\tLast hunk: %ld\n", last));
        
  for (i = first; i <= numhunks; i++) {
    UBYTE *hunkptr;
    ULONG hunksize;

    if (i > last) {
      hunktab[i] = BNULL;
      continue;
    }

    if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
      goto end;

    lcount = AROS_BE2LONG(lcount);
    tmp = lcount & (HUNKF_FAST | HUNKF_CHIP);
    lcount &= 0x3FFFFFFF; // Was 0xFFFFFF -- removed limit of 64Mb (by HUNK-Size) in Amiga executable files -- commit fdd8e7a
    D(bug("\tHunk %d size: 0x%06lx bytes in ", i, lcount*4));
    req = MEMF_CLEAR | MEMF_PUBLIC;
    if (tmp == (HUNKF_FAST | HUNKF_CHIP)) {
      if (read_block_buffered(fh, &req, sizeof(req), funcarray, srb, DOSBase))
        goto end;
      req = AROS_BE2LONG(req);
      D(bug("FLAGS=%08x", req));
    } else if (tmp == HUNKF_FAST) {
      D(bug("FAST"));
      req |= MEMF_FAST;
    } else if (tmp == HUNKF_CHIP) {
      D(bug("CHIP"));
      req |= MEMF_CHIP;
    }
    D(bug(" memory"));

    /*
     * Allocate enough space to store the code, the hunk length, and the
     * next hunk pointer.
     * NB - On 64bit systems, Hunk segments are ALWAYS
     * allocated in the 32bit address space using MEMF_31BIT.
     * This is to make sure that relocatable symbols can correctly be
     * resolved, allowing hunk files to be LoadSeged on 64bit
     * architectures, and so that traditional AmigaOS Font, and
     * Keymap files can be correctly processed and used.
     */
    hunksize = lcount * 4 + sizeof(ULONG) + sizeof(BPTR);
    hunkptr = ilsAllocVec(hunksize, req | MEMF_31BIT);
    if (!hunkptr)
      ERROR(ERROR_NO_FREE_STORE);
    hunktab[i] = MKBADDR(hunkptr);
    D(bug(" @%p\n", hunkptr));
    if (!firsthunk)
      firsthunk = hunktab[i];
      /* Link hunks
         if this is not the first hunk that is loaded, then connect
         it to the previous one (pointer to the field where the
         pointer to the next hunk is located)
       */
    if (prevhunk)
      ((BPTR *)(BADDR(prevhunk)))[0] = hunktab[i];
    prevhunk = hunktab[i];
  }

  while(!read_block_buffered(fh, &hunktype, sizeof(hunktype), funcarray, srb, DOSBase))
  {
    hunktype = AROS_BE2LONG(hunktype);
    D(bug("Hunk Type: %d\n", hunktype & 0xFFFFFF));

    switch(hunktype & 0xFFFFFF)
    {
      case HUNK_SYMBOL:
        /* The SYMBOL_HUNK looks like this:
             ---------------------
             | n = size of       |  This
             |   symbol in longs |  may
             |-------------------|  be
             | n longwords = name|  repeated
             | of symbol         |  any
             |-------------------|  number
             | value (1 long)    |  of times
             --------------------|
             | 0 = end of HUNK_  |
             |       SYMBOL      |
             --------------------   */

        D(bug("HUNK_SYMBOL (skipping)\n"));
          while(!read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase) && lcount)
          {
            lcount = AROS_BE2LONG(lcount) ;

            if (seek_forward(fh, lcount+1, funcarray, srb, DOSBase))
              goto end;
          }
      break;

      case HUNK_UNIT:

        if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
          goto end;

        lcount = AROS_BE2LONG(lcount) ;

        lcount *= 4;
        if (read_block_buffered(fh, name_buf, lcount, funcarray, srb, DOSBase))
          goto end;
        D(bug("HUNK_UNIT: \"%.*s\"\n", lcount, name_buf));
        break;

      case HUNK_CODE:
      case HUNK_DATA:
      case HUNK_BSS:

        if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
          goto end;

        lcount = AROS_BE2LONG(lcount);

        D(bug("HUNK_%s(%d): Length: 0x%06lx bytes in ",
          segtypes[(hunktype & 0xFFFFFF)-HUNK_CODE], curhunk, lcount*4));

        if ((hunktype & 0xFFFFFF) != HUNK_BSS && lcount)
        {
          if (read_block_buffered(fh, GETHUNKPTR(curhunk), lcount*4, funcarray, srb, DOSBase))
            goto end;

        }

        lasthunk = curhunk;
        ++curhunk;
      break;

      case HUNK_RELOC32:
        D(bug("HUNK_RELOC32:\n"));
        while (1)
        {
          ULONG *addr, val;
          ULONG offset;

          if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
            goto end;
          if (lcount == 0L)
            break;

          lcount = AROS_BE2LONG(lcount);

          i = lcount;
          if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
            goto end;

          lcount = AROS_BE2LONG(lcount);

          D(bug("\tHunk #%ld:\n", lcount));
          while (i > 0)
          {
            if (read_block_buffered(fh, &offset, sizeof(offset), funcarray, srb, DOSBase))
              goto end;

            offset = AROS_BE2LONG(offset);

            //D(bug("\t\t0x%06lx\n", offset));
            addr = (ULONG *)(GETHUNKPTR(lasthunk) + offset);

            val = AROS_BE2LONG(*addr) + (IPTR)GETHUNKPTR(lcount);

            D(bug("\t %08x:    %08x ->", addr, *addr));
            *addr = (ULONG)AROS_LONG2BE(val);
            D(bug(" %08x (%08x)\n", *addr, val));

            --i;
          }
        }
      break;

      case HUNK_RELRELOC32:
        D(bug("HUNK_RELRELOC32:\n"));
        while (1)
        {
          ULONG *addr, val;
          UWORD offset;

          if (read_block_buffered(fh, &wcount, sizeof(wcount), funcarray, srb, DOSBase))
            goto end;
          if (wcount == 0L)
            break;

          wcount = AROS_BE2WORD(wcount);

          i = wcount;
          if (read_block_buffered(fh, &wcount, sizeof(wcount), funcarray, srb, DOSBase))
            goto end;

          wcount = AROS_BE2WORD(wcount);

          D(bug("\tHunk #%ld:\n", wcount));
          while (i > 0)
          {
            if (read_block_buffered(fh, &offset, sizeof(offset), funcarray, srb, DOSBase))
              goto end;

            offset = AROS_BE2WORD(offset);

            //D(bug("\t\t0x%06lx\n", offset));
            addr = (ULONG *)(GETHUNKPTR(lasthunk) + offset);

            val = AROS_BE2LONG(*addr) + (IPTR)(GETHUNKPTR(wcount) - GETHUNKPTR(lasthunk));

            *addr = (ULONG)AROS_LONG2BE(val);

            --i;
          }
        }
      break;
        
      case HUNK_ABSRELOC16:
      case HUNK_DREL32: /* For compatibility with V37 */
      case HUNK_RELOC32SHORT:
        {
          ULONG Wordcount = 0;
          ULONG offset;

          while (1)
          {
            ULONG *addr, val;
            UWORD word;
            
            Wordcount++;
            
            if (read_block_buffered(fh, &word, sizeof(word), funcarray, srb, DOSBase))
              goto end;
            if (word == 0L)
              break;

            word = AROS_BE2LONG(word);

            i = word;
            Wordcount++;
            if (read_block_buffered(fh, &word, sizeof(word), funcarray, srb, DOSBase))
              goto end;

            word = AROS_BE2WORD(word);

            lcount = word;
            D(bug("\tHunk #%ld @%p: %ld relocations\n", lcount, GETHUNKPTR(lasthunk), i));
            while (i > 0)
            {
              Wordcount++;
              /* read a 16bit number (2 bytes) */
              if (read_block_buffered(fh, &word, sizeof(word), funcarray, srb, DOSBase))
                goto end;

              /* offset now contains the byte offset in it`s 16 highest bits.
                 These 16 highest bits have to become the 16 lowest bits so
                 we get the word we need.  */
              offset = AROS_BE2WORD(word);

              D(bug("\t\t0x%06lx += 0x%lx\n", offset, GETHUNKPTR(lcount)));
              addr = (ULONG *)(GETHUNKPTR(lasthunk) + offset);

              if ((hunktype & 0xFFFFFF) == HUNK_ABSRELOC16)
              {
                val = (AROS_BE2LONG(*addr) - ((IPTR)GETHUNKPTR(lasthunk) + offset)) + (IPTR)GETHUNKPTR(lcount);
              }
              else
              {
                val = AROS_BE2LONG(*addr) + (IPTR)GETHUNKPTR(lcount);
              }
              *addr = (ULONG)AROS_LONG2BE(val);

              --i;
            } /* while (i > 0)*/
          } /* while (1) */

          /* if the amount of words read was odd, then skip the following
           16-bit word   */
          if (0x1 == (Wordcount & 0x1)) {
            UWORD word;
            read_block_buffered(fh, &word, sizeof(word), funcarray, srb, DOSBase);
          }
        }
      break;

      case HUNK_END:
      {
        D(bug("HUNK_END\n"));
        /* DOSBase == NULL: Called from RDB filesystem loader which does not
         * know filesystem's original size. Exit if last HUNK_END. This can't
         * be done normally because it would break overlayed executables.
         */
        if (!DOSBase && curhunk > last)
            goto done;
      }
      break;

      case HUNK_RELOC16:
        bug("HUNK_RELOC16 not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_RELOC8:
        bug("HUNK_RELOC8 not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_NAME:
        bug("HUNK_NAME not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_EXT:
        bug("HUNK_EXT not implemented\n");
        ERROR(ERROR_BAD_HUNK);

      case HUNK_DEBUG:
        if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
          goto end;

        lcount = AROS_BE2LONG(lcount);

        D(bug("HUNK_DEBUG (%x Bytes)\n",lcount));
        if (seek_forward(fh, lcount, funcarray, srb, DOSBase))
          goto end;
        break;

      case HUNK_OVERLAY:
      {
        D(bug("HUNK_OVERLAY:\n"));
        if (table) /* overlay inside overlay? */
          ERROR(ERROR_BAD_HUNK);
        if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
          goto end;
        lcount = AROS_BE2LONG(lcount);
        D(bug("Overlay table size: %d\n", lcount));

        /* See above for MEMF_31BIT explanation */
        lcount = lcount * 4 + sizeof(ULONG) + sizeof(ULONG);
        overlaytable = ilsAllocVec(lcount, MEMF_CLEAR | MEMF_31BIT);
        if (overlaytable == NULL)
          ERROR(ERROR_NO_FREE_STORE);
        if (read_block_buffered(fh, overlaytable, lcount - sizeof(ULONG), funcarray, srb, DOSBase))
            goto end;
        goto done;
      }

      case HUNK_BREAK:
        D(bug("HUNK_BREAK\n"));
        if (!table)
          ERROR(ERROR_BAD_HUNK);
        goto done;

      default:
        if (hunktype & HUNKF_ADVISORY) {
          D(bug("Unknown hunk 0x%06lx with advisory flag skipped\n", hunktype & 0xFFFFFF));
          if (read_block_buffered(fh, &lcount, sizeof(lcount), funcarray, srb, DOSBase))
            goto end;
          lcount = AROS_BE2LONG(lcount);
          if (seek_forward(fh, lcount * 4, funcarray, srb, DOSBase))
            goto end;
        } else {
          bug("Hunk type 0x%06lx not implemented\n", hunktype & 0xFFFFFF);
          ERROR(ERROR_BAD_HUNK);
        }
    } /* switch */
  } /* while */

done:
  if (hunktab)
  {
    ULONG hunksize;

#if defined(DOCACHECLEAR)
    /* Clear caches */
    for (t = first; t < numhunks && t <= last; t++)
    {
        hunksize = *((ULONG*)BADDR(hunktab[t]) - 1);
        if (hunksize)
          ils_ClearCache(BADDR(hunktab[t]), hunksize, CACRF_ClearI | CACRF_ClearD);
    }
#endif

    if (table)
      return firsthunk;

    hunksize = *((ULONG*)BADDR(hunktab[0]) - 1);
    if (last > first && hunksize >= 32 / 4)
    {
      /* NOTE: HUNK_OVERLAY is not required for overlay mode. */
      ULONG *h = (ULONG*)(BADDR(hunktab[first]));

      if (h[2] == 0x0000abcd)
      {
#if __WORDSIZE != 32
        D(bug("overlay not supported!\n"));
        ERROR(ERROR_BAD_HUNK);
#else
        /* overlay executable */
        h[3] = (ULONG)fh;
        h[4] = (ULONG)overlaytable;
        h[5] = (ULONG)MKBADDR(hunktab);
        D(bug("overlay loaded!\n"));
        return (BPTR)(-(LONG)MKBADDR(h));
#endif
      }
    }

    if (overlaytable) {
      ilsFreeVec(overlaytable);
      ERROR(ERROR_BAD_HUNK);
    }

    last_p = firsthunk;

    register_hunk(fh, firsthunk, NULL, DOSBase);

    ilsFreeVec(hunktab);
    hunktab = NULL;
  }

end:
  if (hunktab != NULL)
  {
    for (t = 0 /* first */; t < numhunks /* last */; t++)
      ilsFreeVec(BADDR(hunktab[t]));
    ilsFreeVec(hunktab);
  }
  ilsFreeMem(srb->buffer, LOADSEG_HUNK_BUFFER);
  return last_p;
} /* InternalLoadSeg */

#ifdef __mc68000
static AROS_UFH4(LONG, ReadFunc,
        AROS_UFHA(BPTR, file,   D1),
        AROS_UFHA(APTR, buffer, D2),
        AROS_UFHA(LONG, length, D3),
        AROS_UFHA(struct DosLibrary *, DOSBase, A6)
)
{
    AROS_USERFUNC_INIT

    return FRead(file, buffer, 1, length);

    AROS_USERFUNC_EXIT
}

static AROS_UFH3(APTR, AllocFunc,
        AROS_UFHA(ULONG, length, D0),
        AROS_UFHA(ULONG, flags,  D1),
        AROS_UFHA(struct ExecBase *, SysBase, A6)
)
{
    AROS_USERFUNC_INIT

    return AllocMem(length, flags);

    AROS_USERFUNC_EXIT
}

static AROS_UFH3(void, FreeFunc,
        AROS_UFHA(APTR, buffer, A1),
        AROS_UFHA(ULONG, length, D0),
        AROS_UFHA(struct ExecBase *, SysBase, A6)
)
{
    AROS_USERFUNC_INIT

    FreeMem(buffer, length);

    AROS_USERFUNC_EXIT
}

AROS_UFH4(BPTR, LoadSeg_Overlay,
    AROS_UFHA(UBYTE*, name, D1),
    AROS_UFHA(BPTR, hunktable, D2),
    AROS_UFHA(BPTR, fh, D3),
    AROS_UFHA(struct DosLibrary *, DosBase, A6))
{
    AROS_USERFUNC_INIT

    void (*FunctionArray[3])();
    ULONG hunktype;

    FunctionArray[0] = (APTR)ReadFunc;
    FunctionArray[1] = (APTR)AllocFunc;
    FunctionArray[2] = (APTR)FreeFunc;

    D(bug("LoadSeg_Overlay. table=%x fh=%x\n", hunktable, fh));
    if (read_block_buffered(fh, &hunktype, sizeof(hunktype), (SIPTR*)FunctionArray, NULL, DosBase))
        return BNULL;
    hunktype = AROS_BE2LONG(hunktype);
    if (hunktype != HUNK_HEADER)
        return BNULL;
    return InternalLoadSeg_AOS(fh, hunktable, (SIPTR*)FunctionArray, NULL, DosBase);

    AROS_USERFUNC_EXIT
}

#endif
