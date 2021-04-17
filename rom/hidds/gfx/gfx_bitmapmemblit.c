/*
    Copyright © 1995-2017, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <string.h>

#include <hidd/gfx.h>

#include "gfx_intern.h"
#include "gfx_debug.h"

/****************************************************************************************/

VOID BM__Hidd_BitMap__FillMemRect8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect8 *msg)
{
    UBYTE *start;
    LONG phase, width, height;
    LONG p,count,rest;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width;
        
    if ((phase = (IPTR)start & 7L)) {
    	phase = 8 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    fill32 = msg->fill;
    fill32 |= (fill32 << 8);
    fill32 |= (fill32 << 16);
    

    count=width>>3;
       asm volatile(
       "       bra 7f                        \n"
       "0:                                  \n"
       "       move.l %[phase],%[count]     \n"
       "       bra 2f                       \n"
       "1:     move.b %[color],(%[address])+ \n"
       "2:     dbra   %[count],1b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       lsr.l  #3,%[count]           \n" 
       "       bra 4f                        \n"
       "3:     move.l %[color],(%[address])+ \n"
       "       move.l %[color],(%[address])+ \n"
       "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #7,%[count]           \n" 
       "       bra 6f                       \n"
       "5:     move.b %[color],(%[address])+ \n"
       "6:     dbra   %[count],5b          \n"
       "                                    \n"
       "       adda.l %[modulo],%[address]  \n"
       "7:     dbra   %[yloop],0b           \n"
       :[count]"+d"(count), [yloop]"+d"(height)
       :[address]"a"(start), [color]"d"(fill32), [phase]"r"(phase), [width]"r"(width), [modulo]"r"(start_add) 
       :"cc");

};

/****************************************************************************************/

VOID BM__Hidd_BitMap__FillMemRect16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect16 *msg)
{
    UBYTE *start;
    LONG phase, width, height;
    LONG p, count, rest;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX * 2;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width * 2;
        
    if ((phase = ((IPTR)start & 7L)>>1)) {
    	phase = 4 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    fill32 = msg->fill;
    fill32 |= (fill32 << 16);
    

    count=width>>2;
       asm volatile(
       "       bra 7f                        \n"
       "0:                                  \n"
       "       move.l %[phase],%[count]     \n"
       "       bra 2f                       \n"
       "1:     move.w %[color],(%[address])+ \n"
       "2:     dbra   %[count],1b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       lsr.l  #2,%[count]           \n" 
       "       bra 4f                        \n"
       "3:     move.l %[color],(%[address])+ \n"
       "       move.l %[color],(%[address])+ \n"
       "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #3,%[count]           \n" 
       "       bra 6f                       \n"
       "5:     move.w %[color],(%[address])+ \n"
       "6:     dbra   %[count],5b          \n"
       "                                    \n"
       "       adda.l %[modulo],%[address]  \n"
       "7:     dbra   %[yloop],0b           \n"
       :[count]"+d"(count), [yloop]"+d"(height)
       :[address]"a"(start), [color]"d"(fill32), [phase]"r"(phase), [width]"r"(width), [modulo]"r"(start_add) 
       :"cc");
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__FillMemRect24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect24 *msg)
{
    UBYTE *start;
    LONG width, height, w;
    UBYTE fill1, fill2, fill3;
    ULONG start_add;
    ULONG fillL, fillW;
 
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX * 3;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width * 3;

    fill1 = (msg->fill >> 16) & 0xFF;
    fill2 = (msg->fill >> 8) & 0xFF;
    fill3 =  msg->fill & 0xFF;

    fillL = (msg->fill <<8) | fill1;
    fillW = msg->fill;

     w=width>>1; 
       asm volatile(
       "       bra 7f                        \n"
       "0:                                  \n"
       "       move.l %[width],%[count]     \n"
       "       lsr.l  #1,%[count]           \n" 
       "       bra 4f                        \n"
       "3:     move.l %[colorL],(%[address])+ \n"
       "       move.w %[colorW],(%[address])+ \n"
       "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #1,%[count]           \n" 
       "       bra 6f                       \n"
       "5:     move.b %[colorB],(%[address])+ \n"
       "       move.w %[colorW],(%[address])+ \n"
       "6:     dbra   %[count],5b          \n"
       "                                    \n"
       "       adda.l %[modulo],%[address]  \n"
       "7:     dbra   %[yloop],0b           \n"
       :[count]"+d"(w), [yloop]"+d"(height)
       :[address]"a"(start), [colorL]"r"(fillL), [colorW]"r"(fillW), [colorB]"r"(fill1), [width]"r"(width), [modulo]"r"(start_add) 
       :"cc");


}

/****************************************************************************************/

VOID BM__Hidd_BitMap__FillMemRect32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_FillMemRect32 *msg)
{
    UBYTE *start;
    LONG phase, width, height;
    LONG p, count, rest;
    ULONG fill32;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX * 4;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width * 4;
        
    phase = 0;
    if ((phase = (IPTR)start & 7L)) {
    	phase = 1;
	if (phase > width) phase = width;
	width -= phase;
    }

    fill32 = msg->fill;
        
    count=width>>1;
       asm volatile(
       "       bra 7f                        \n"
       "0:                                  \n"
       "       move.l %[phase],%[count]     \n"
       "       bra 2f                       \n"
       "1:     move.l %[color],(%[address])+ \n"
       "2:     dbra   %[count],1b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       lsr.l  #1,%[count]           \n" 
       "       bra 4f                        \n"
       "3:     move.l %[color],(%[address])+ \n"
       "       move.l %[color],(%[address])+ \n"
       "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #1,%[count]           \n" 
       "       bra 6f                       \n"
       "5:     move.l %[color],(%[address])+ \n"
       "6:     dbra   %[count],5b          \n"
       "                                    \n"
       "       adda.l %[modulo],%[address]  \n"
       "7:     dbra   %[yloop],0b           \n"
       :[count]"+d"(count), [yloop]"+d"(height)
       :[address]"a"(start), [color]"d"(fill32), [phase]"r"(phase), [width]"r"(width), [modulo]"r"(start_add) 
       :"cc");
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__InvertMemRect(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_InvertMemRect *msg)
{
    UBYTE *start;
    LONG phase, width, height, w, p;
    ULONG start_add;
    
    start = msg->dstBuf + msg->minY * msg->dstMod + msg->minX;
    width = msg->maxX - msg->minX + 1;
    height = msg->maxY - msg->minY + 1;
    start_add = msg->dstMod - width;
        
    if ((phase = (IPTR)start & 3L))
    {
    	phase = 4 - phase;
	if (phase > width) phase = width;
	width -= phase;
    }
    
    while(height--)
    {
    	UBYTE bg;
    	ULONG bg32;
	
    	w = width;
	p = phase;
	
	while(p--)
	{
	    bg = *start;
	    *start++ = ~bg;
	}
	while(w >= 4)
	{
	    bg32 = *(ULONG *)start;
	    *((ULONG *)start) = ~bg32;
	    w -= 4; start += 4;
	}
	while(w--)
	{
	    bg = *start;
	    *start++ = ~bg;
	}
	start += start_add;
    }
        
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyMemBox8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox8 *msg)
{
    UBYTE *src_start, *src_end, *dst_start;
    LONG phase, width, height, w;
    LONG p, count, rest;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_end   = msg->src + (msg->srcY+height) * msg->srcMod + (msg->srcX + msg->width);
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX;
    dst_start_add = msg->dstMod - width;
        
    if( ((IPTR)src_start > (IPTR)dst_start) || ((IPTR)src_end < (IPTR)dst_start) ){
	if ((phase = (IPTR)dst_start & 7L)) {
    	    phase = 8 - phase;
	    if (phase > width) phase = width;
	    width -= phase;
	}
    	descending = FALSE;
    }else{
      	src_start += (height - 1) * msg->srcMod + width;
   	dst_start += (height - 1) * msg->dstMod + width;
   	
   	phase = ((IPTR)dst_start & 7L);
   	if (phase > width) phase = width;
   	width -= phase;
   	
   	descending = TRUE;
    }
 
    /* NOTE: This can write LONGs to odd addresses, which might not work
       on some CPUs (MC68000) */

  if (!descending) {
    count=width>>3;
    rest =width & 7;
    p=phase;
  
    asm volatile(
    "       bra 7f                       \n"
    "0:                                  \n"
    "       move.l %[phase],%[count]     \n"
    "       bra 2f                       \n"
    "1:     move.b (%[src])+,(%[dst])+   \n"
    "2:     dbra   %[count],1b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       lsr.l  #4,%[count]           \n"
    "       bra 4f                       \n"
//    "3:     move.l (%[src])+,(%[dst])+   \n"
//    "       move.l (%[src])+,(%[dst])+   \n"
    "3:     move16 (%[src])+,(%[dst])+   \n"
    "4:     dbra   %[count],3b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       and.l  #15,%[count]           \n"
    "       bra 6f                       \n"
    "5:     move.b (%[src])+,(%[dst])+   \n"
    "6:     dbra   %[count],5b           \n"
    "                                    \n"
    "       adda.l %[srcmodulo],%[src]   \n"
    "       adda.l %[dstmodulo],%[dst]   \n"
    "                                    \n"
    "7:     dbra %[yloop],0b             \n"

    :[count]"+d"(count), [yloop]"+d"(height)
    :[dst]"a"(dst_start), [src]"a"(src_start), [width]"r"(width), [phase]"r"(phase), [srcmodulo]"r"(src_start_add), [dstmodulo]"r"(dst_start_add)
    :"cc");

  }else{
   	while(height--)
   	{
       	    w = width;
   	    p = phase;
   
   	    while(p--)
   	    {
   		*--dst_start = *--src_start;
   	    }
   	    while(w >= 4)
   	    {
   	        dst_start -= 4; src_start -= 4;
   		*(ULONG *)dst_start = *(ULONG *)src_start;
   		w -= 4;
   	    }
   	    while(w--)
   	    {
   		*--dst_start = *--src_start;
   	    }
   	    src_start -= src_start_add;
   	    dst_start -= dst_start_add;
   	}
  }
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyMemBox16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox16 *msg)
{
    UBYTE *src_start, *src_end, *dst_start;
    LONG phase, width, height, w;
    LONG p, count, rest;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 2;
    src_end   = msg->src + (msg->srcY+height) * msg->srcMod + (msg->srcX + msg->width)*2;
    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 2;
    src_start_add = msg->srcMod - width * 2;
    dst_start_add = msg->dstMod - width * 2;
        
    if( ((IPTR)src_start > (IPTR)dst_start) || ((IPTR)src_end < (IPTR)dst_start) ){
	if (phase = (((IPTR)dst_start & 7L)>>1)) {
    	    phase = 4 - phase;
	    if (phase > width) phase = width;
	    width -= phase;
	}
    	descending = FALSE;
    }else{
      	src_start += (height - 1) * msg->srcMod + width * 2;
   	dst_start += (height - 1) * msg->dstMod + width * 2;
   	
   	phase = (((IPTR)dst_start & 7L)>>1);
   	if (phase > width) phase = width;
   	width -= phase;
   	
   	descending = TRUE;
    }
 
 if (!descending){
    count=width>>2;
    rest =width &3;
    p=phase;

    asm volatile(
    "       bra 7f                       \n"
    "0:                                  \n"
    "       move.l %[phase],%[count]     \n"
    "       bra 2f                       \n"
    "1:     move.w (%[src])+,(%[dst])+   \n"
    "2:     dbra   %[count],1b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       lsr.l  #3,%[count]           \n"
    "       bra 4f                       \n"
//    "3:     move.l (%[src])+,(%[dst])+   \n"
//    "       move.l (%[src])+,(%[dst])+   \n"
    "3:     move16 (%[src])+,(%[dst])+   \n"
    "4:     dbra   %[count],3b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       and.l  #7,%[count]           \n"
    "       bra 6f                       \n"
    "5:     move.w (%[src])+,(%[dst])+   \n"
    "6:     dbra   %[count],5b           \n"
    "                                    \n"
    "       adda.l %[srcmodulo],%[src]   \n"
    "       adda.l %[dstmodulo],%[dst]   \n"
    "                                    \n"
    "7:     dbra %[yloop],0b             \n"

    :[count]"+d"(count), [yloop]"+d"(height)
    :[dst]"a"(dst_start), [src]"a"(src_start), [width]"r"(width), [phase]"r"(phase), [srcmodulo]"r"(src_start_add), [dstmodulo]"r"(dst_start_add)
    :"cc");


       } else {
   	while(height--)
   	{
       	    w = width;
   	    p = phase;
   
   	    while(p--)
   	    {
   	        dst_start -= 2; src_start -= 2;
   		*(UWORD *)dst_start = *(UWORD *)src_start;
  	    }
   	    while(w >= 2)
   	    {
   	        dst_start -= 4; src_start -= 4;
   		*(ULONG *)dst_start = *(ULONG *)src_start;
   		w -= 2;
   	    }
   	    while(w--)
   	    {
   	        dst_start -= 2; src_start -= 2;
   		*(UWORD *)dst_start = *(UWORD *)src_start;
   	    }
   	    src_start -= src_start_add;
   	    dst_start -= dst_start_add;
   	}
   }
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyMemBox24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox24 *msg)
{
    UBYTE *src_start, *src_end, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 3;
    src_end   = msg->src + (msg->srcY+height) * msg->srcMod + (msg->srcX + msg->width)*3;
    src_start_add = msg->srcMod - width * 3;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 3;
    dst_start_add = msg->dstMod - width * 3;
        
    if( ((IPTR)src_start > (IPTR)dst_start) || ((IPTR)src_end < (IPTR)dst_start) ){
       	descending = FALSE;
    }else{
      	src_start += (height - 1) * msg->srcMod + width * 3;
   	dst_start += (height - 1) * msg->dstMod + width * 3;
   	
   	descending = TRUE;
    }
    
    if (!descending){
       asm volatile(
       "       bra 7f                     \n"
       "0:                                \n"
       "       move.l %[width],%[count]   \n"
       "       lsr.l  #2,%[count]         \n" 
       "       bra 4f                     \n"
       "3:     move.l (%[src])+,(%[dst])+ \n"
       "       move.l (%[src])+,(%[dst])+ \n"
       "       move.l (%[src])+,(%[dst])+ \n"
       "4:     dbra   %[count],3b         \n"
       "                                  \n"
       "       move.l %[width],%[count]   \n"
       "       and.l  #3,%[count]         \n" 
       "       bra 6f                     \n"
       "5:     move.b (%[src])+,(%[dst])+ \n"
       "       move.w (%[src])+,(%[dst])+ \n"
       "6:     dbra   %[count],5b         \n"
       "                                  \n"
       "       adda.l %[src_modulo],%[src]\n"
       "       adda.l %[dst_modulo],%[dst]\n"
       "7:     dbra   %[yloop],0b         \n"
       :[count]"+d"(w), [yloop]"+d"(height)
       :[dst]"a"(dst_start),[src]"a"(src_start), [width]"r"(width), [src_modulo]"r"(src_start_add),[dst_modulo]"r"(dst_start_add)
 
       :"cc");


  } else {
   	while(height--)
   	{
       	    w = width;
   
   	    while(w--)
   	    {
   		*--dst_start = *--src_start;
   		*--dst_start = *--src_start;
   		*--dst_start = *--src_start;
   	    }
   	    
   	    src_start -= src_start_add;
   	    dst_start -= dst_start_add;
   	}
  }
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyMemBox32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyMemBox32 *msg)
{
    UBYTE *src_start, *src_end,  *dst_start;
    LONG phase, width, height, w;
    LONG p, count, rest;
    ULONG src_start_add, dst_start_add;
    BOOL descending;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 4;
    src_end   = msg->src + (msg->srcY+height) * msg->srcMod + (msg->srcX + msg->width)*4;
    src_start_add = msg->srcMod - width * 4;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 4;
    dst_start_add = msg->dstMod - width * 4;

        
    if( ((IPTR)src_start > (IPTR)dst_start) || ((IPTR)src_end < (IPTR)dst_start) ){
       	descending = FALSE;

       phase=0;
       if (((IPTR)dst_start & 7L)!=0) {
       	 phase = 1;
	 if (phase > width) phase = width;
 	 width -= phase;
       }

    }else{
       	src_start += (height - 1) * msg->srcMod + width * 4;
   	dst_start += (height - 1) * msg->dstMod + width * 4;
   	
   	descending = TRUE;
    }
   
 
 if (!descending) {


    count=width>>1;
    rest =width &1;
    p=phase;

    asm volatile(
    "       bra 7f                       \n"
    "0:                                  \n"
    "       move.l %[phase],%[count]     \n"
    "       bra 2f                       \n"
    "1:     move.l (%[src])+,(%[dst])+   \n"
    "2:     dbra   %[count],1b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       lsr.l  #1,%[count]           \n"
    "       bra 4f                       \n"
    "3:     move.l (%[src])+,(%[dst])+   \n"
    "       move.l (%[src])+,(%[dst])+   \n"
    "4:     dbra   %[count],3b           \n"
    "                                    \n"
    "       move.l %[width],%[count]     \n"
    "       and.l  #1,%[count]           \n"
    "       bra 6f                       \n"
    "5:     move.l (%[src])+,(%[dst])+   \n"
    "6:     dbra   %[count],5b           \n"
    "                                    \n"
    "       adda.l %[srcmodulo],%[src]   \n"
    "       adda.l %[dstmodulo],%[dst]   \n"
    "                                    \n"
    "7:     dbra %[yloop],0b             \n"

    :[count]"+d"(count), [yloop]"+d"(height)
    :[dst]"a"(dst_start), [src]"a"(src_start), [width]"r"(width), [phase]"r"(phase), [srcmodulo]"r"(src_start_add), [dstmodulo]"r"(dst_start_add)
    :"cc");


    } else {
   	while(height--)
   	{
       	    w = width;
   
   	    while(w--)
   	    {
   	        dst_start -= 4; src_start -= 4;
   		*(ULONG *)dst_start = *(ULONG *)src_start;
   	    }
   	    
   	    src_start -= src_start_add;
   	    dst_start -= dst_start_add;
   	}
    }
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyLUTMemBox16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyLUTMemBox16 *msg)
{
    HIDDT_Pixel *pixlut = msg->pixlut->pixels;
    UBYTE *src_start, *dst_start;
    LONG width, height, w, count=0;
    ULONG pix1=1,pix2=2,pix3=3,pix4=4;
    ULONG src_start_add, dst_start_add;
    
    if (!pixlut) return;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 2;
    dst_start_add = msg->dstMod - width * 2;
        
//          while(height--)
//          {
//          	w = width;
//      
//      	while(w--)
//      	{
//         	    *(UWORD *)dst_start = (UWORD)(pixlut[*src_start++]);
//         	    dst_start += 2;
//      	}
//      	src_start += src_start_add;
//      	dst_start += dst_start_add;
//          }

       asm volatile(
       "       clr.l  %[pix1]               \n"
       "       clr.l  %[pix2]               \n"
       "       clr.l  %[pix3]               \n"
       "       clr.l  %[pix4]               \n"
       "       bra 7f                       \n"
       "0:                                  \n"
             "       move.l %[width],%[count]     \n"
             "       lsr.l  #2,%[count]           \n"
             "       bra 4f                       \n"
             "3:     move.b (%[src])+,%[pix1]     \n"
             "       move.b (%[src])+,%[pix2]     \n"
             "       move.b (%[src])+,%[pix3]     \n"
             "       move.b (%[src])+,%[pix4]     \n"
             "       move.w 2(%[pixlut],%[pix1]*4),(%[dst])+\n"
             "       move.w 2(%[pixlut],%[pix2]*4),(%[dst])+\n"
             "       move.w 2(%[pixlut],%[pix3]*4),(%[dst])+\n"
             "       move.w 2(%[pixlut],%[pix4]*4),(%[dst])+\n"
             "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #3,%[count]           \n"
       "       bra 6f                       \n"
       "5:     move.b (%[src])+,%[pix1]     \n"
       "       move.w 2(%[pixlut],%[pix1]*4),(%[dst])+\n"
       "6:     dbra   %[count],5b           \n"
       "                                    \n"
       "       adda.l %[srcmodulo],%[src]   \n"
       "       adda.l %[dstmodulo],%[dst]   \n"
       "                                    \n"
       "7:     dbra %[yloop],0b             \n"
   
       :[count]"+d"(count), [yloop]"+d"(height), [pix1]"+d"(pix1), [pix2]"+d"(pix2), [pix3]"+d"(pix3), [pix4]"+d"(pix4)
       :[pixlut]"a"(pixlut), [dst]"a"(dst_start), [src]"a"(src_start), [width]"r"(width), [srcmodulo]"r"(src_start_add), [dstmodulo]"r"(dst_start_add)
       :"cc");

}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyLUTMemBox24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyLUTMemBox24 *msg)
{
    HIDDT_Pixel *pixlut = msg->pixlut->pixels;
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;

    if (!pixlut) return;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 3;
    dst_start_add = msg->dstMod - width * 3;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
   	    HIDDT_Pixel pix = pixlut[*src_start++];
   	    
   	#if AROS_BIG_ENDIAN
   	    *dst_start++ = (pix >> 16) & 0xFF;
   	    *dst_start++ = (pix >> 8) & 0xFF;
   	    *dst_start++ =  pix & 0xFF;
   	#else
   	    *dst_start++ =  pix & 0xFF;
   	    *dst_start++ = (pix >> 8) & 0xFF;
   	    *dst_start++ = (pix >> 16) & 0xFF;
   	#endif
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__CopyLUTMemBox32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_CopyLUTMemBox32 *msg)
{
    HIDDT_Pixel *pixlut = msg->pixlut->pixels;
    UBYTE *src_start, *dst_start;
    LONG width, height, w, count=0;
    ULONG src_start_add, dst_start_add;
    ULONG pix1=1,pix2=2,pix3=3,pix4=4;

    if (!pixlut) return;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 4;
    dst_start_add = msg->dstMod - width * 4;
        

       asm volatile(
       "       clr.l  %[pix1]               \n"
       "       clr.l  %[pix2]               \n"
       "       clr.l  %[pix3]               \n"
       "       clr.l  %[pix4]               \n"
       "       bra 7f                       \n"
       "0:                                  \n"
             "       move.l %[width],%[count]     \n"
             "       lsr.l  #2,%[count]           \n"
             "       bra 4f                       \n"
             "3:     move.b (%[src])+,%[pix1]     \n"
             "       move.b (%[src])+,%[pix2]     \n"
             "       move.b (%[src])+,%[pix3]     \n"
             "       move.b (%[src])+,%[pix4]     \n"
             "       move.l (%[pixlut],%[pix1]*4),(%[dst])+\n"
             "       move.l (%[pixlut],%[pix2]*4),(%[dst])+\n"
             "       move.l (%[pixlut],%[pix3]*4),(%[dst])+\n"
             "       move.l (%[pixlut],%[pix4]*4),(%[dst])+\n"
             "4:     dbra   %[count],3b           \n"
       "                                    \n"
       "       move.l %[width],%[count]     \n"
       "       and.l  #3,%[count]           \n"
       "       bra 6f                       \n"
       "5:     move.b (%[src])+,%[pix1]     \n"
       "       move.l (%[pixlut],%[pix1]*4),(%[dst])+\n"
       "6:     dbra   %[count],5b           \n"
       "                                    \n"
       "       adda.l %[srcmodulo],%[src]   \n"
       "       adda.l %[dstmodulo],%[dst]   \n"
       "                                    \n"
       "7:     dbra %[yloop],0b             \n"
   
       :[count]"+d"(count), [yloop]"+d"(height), [pix1]"+d"(pix1), [pix2]"+d"(pix2), [pix3]"+d"(pix3), [pix4]"+d"(pix4)
       :[pixlut]"a"(pixlut), [dst]"a"(dst_start), [src]"a"(src_start), [width]"r"(width), [srcmodulo]"r"(src_start_add), [dstmodulo]"r"(dst_start_add)
       :"cc");


}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMem32Image8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMem32Image8 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src;
    src_start_add = msg->srcMod - width * 4;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX;
    dst_start_add = msg->dstMod - width;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
  	    *dst_start++ = (UBYTE)(*(ULONG *)src_start);
   	    src_start += 4;
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMem32Image16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMem32Image16 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src;
    src_start_add = msg->srcMod - width * 4;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 2;
    dst_start_add = msg->dstMod - width * 2;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{

//7
   	    *(UWORD *)dst_start = (UWORD)(*(ULONG *)src_start);
   	    dst_start += 2; src_start += 4;
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMem32Image24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMem32Image24 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src;
    src_start_add = msg->srcMod - width * 4;

    dst_start = msg->dst + msg->dstY * msg->dstMod + msg->dstX * 3;
    dst_start_add = msg->dstMod - width * 3;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
	    ULONG pix = *(ULONG *)src_start;

	    src_start += 4;
	    
//7
   	#if AROS_BIG_ENDIAN
   	    *dst_start++ = (pix >> 16) & 0xFF;
   	    *dst_start++ = (pix >> 8) & 0xFF;
   	    *dst_start++ =  pix & 0xFF;
   	#else
   	    *dst_start++ =  pix & 0xFF;
   	    *dst_start++ = (pix >> 8) & 0xFF;
   	    *dst_start++ = (pix >> 16) & 0xFF;
   	#endif
   
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__GetMem32Image8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_GetMem32Image8 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX;
    src_start_add = msg->srcMod - width;

    dst_start = msg->dst;
    dst_start_add = msg->dstMod - width * 4;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
   	    *(ULONG *)dst_start = (ULONG)(*src_start++);
   	    dst_start += 4; 
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__GetMem32Image16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_GetMem32Image16 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 2;
    src_start_add = msg->srcMod - width * 2;

    dst_start = msg->dst;
    dst_start_add = msg->dstMod - width * 4;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
   	    *(ULONG *)dst_start = (ULONG)(*(UWORD *)src_start);
   	    dst_start += 4; src_start += 2;
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__GetMem32Image24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_GetMem32Image24 *msg)
{
    UBYTE *src_start, *dst_start;
    LONG width, height, w;
    ULONG src_start_add, dst_start_add;
    
    width = msg->width;
    height = msg->height;

    src_start = msg->src + msg->srcY * msg->srcMod + msg->srcX * 3;
    src_start_add = msg->srcMod - width * 3;

    dst_start = msg->dst;
    dst_start_add = msg->dstMod - width * 4;
        
    while(height--)
    {
    	w = width;

	while(w--)
	{
   	    UBYTE pix1 = *src_start++;
   	    UBYTE pix2 = *src_start++;
   	    UBYTE pix3 = *src_start++;
   	    
   	#if AROS_BIG_ENDIAN
   	    *(ULONG *)dst_start = (pix1 << 16) | (pix2 << 8) | pix3;
   	#else
   	    *(ULONG *)dst_start = (pix3 << 16) | (pix2 << 8) | pix1;
   	#endif
   	
	    dst_start += 4;
	}
	src_start += src_start_add;
	dst_start += dst_start_add;
    }
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemTemplate8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemTemplate8 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *bitarray, *buf;
    UWORD    	    	     bitmask;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
    {
    	type = 0;
    }
    else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
    {
    	type = 2;
    }
    else
    {
    	type = 4;
    }
    
    if (msg->inverttemplate) type++;
    
    bitarray = msg->masktemplate + ((msg->srcx / 16) * 2);
    bitmask = 0x8000 >> (msg->srcx & 0xF);
    
    buf = msg->dst + msg->y * msg->dstMod + msg->x;
    
    for(y = 0; y < msg->height; y++)
    {
	ULONG  mask = bitmask;
	UWORD *array = (UWORD *)bitarray;
	UWORD  bitword = AROS_BE2WORD(*array);
    	UBYTE *xbuf = buf;

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		 } /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? fg : bg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? bg : fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	bitarray += msg->modulo;

    } /* for(y = 0; y < msg->height; y++) */ 
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemTemplate16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemTemplate16 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *bitarray, *buf;
    UWORD   	    	     bitmask;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
    {
    	type = 0;
    }
    else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
    {
    	type = 2;
    }
    else
    {
    	type = 4;
    }
    
    if (msg->inverttemplate) type++;
    
    bitarray = msg->masktemplate + ((msg->srcx / 16) * 2);
    bitmask = 0x8000 >> (msg->srcx & 0xF);
    
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 2;

    for(y = 0; y < msg->height; y++)
    {
	ULONG  mask = bitmask;
	UWORD *array = (UWORD *)bitarray;
	UWORD  bitword = AROS_BE2WORD(*array);
    	UWORD *xbuf = (UWORD *)buf;

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		 } /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? fg : bg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? bg : fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	bitarray += msg->modulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemTemplate24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemTemplate24 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *bitarray, *buf;
    UWORD   	    	     bitmask;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    UBYTE   	    	     fg1, fg2, fg3;
    ULONG	    	     bg = GC_BG(gc);
    UBYTE   	    	     bg1, bg2, bg3;
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

#if AROS_BIG_ENDIAN        
    fg1 = (fg >> 16) & 0xFF;
    fg2 = (fg >> 8) & 0xFF;
    fg3 =  fg & 0xFF;

    bg1 = (bg >> 16) & 0xFF;
    bg2 = (bg >> 8) & 0xFF;
    bg3 =  bg & 0xFF;
#else
    fg1 =  fg & 0xFF;
    fg2 = (fg >> 8) & 0xFF;
    fg3 = (fg >> 16) & 0xFF;

    bg1 =  bg & 0xFF;
    bg2 = (bg >> 8) & 0xFF;
    bg3 = (bg >> 16) & 0xFF;
#endif

    if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
    {
    	type = 0;
    }
    else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
    {
    	type = 2;
    }
    else
    {
    	type = 4;
    }
    
    if (msg->inverttemplate) type++;
    
    bitarray = msg->masktemplate + ((msg->srcx / 16) * 2);
    bitmask = 0x8000 >> (msg->srcx & 0xF);
    
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 3;
    
    for(y = 0; y < msg->height; y++)
    {
	ULONG  mask = bitmask;
	UWORD *array = (UWORD *)bitarray;
	UWORD  bitword = AROS_BE2WORD(*array);
    	UBYTE *xbuf = buf;

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
    	    	    if (bitword & mask)
		    {
			xbuf[0] = fg1;
			xbuf[1] = fg2;
			xbuf[2] = fg3;
		    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
    	    	    if (!(bitword & mask))
		    {
			xbuf[0] = fg1;
			xbuf[1] = fg2;
			xbuf[2] = fg3;
    	    	    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
    	    	    if (bitword & mask)
		    {
			xbuf[0] = ~xbuf[0];
			xbuf[1] = ~xbuf[1];
			xbuf[2] = ~xbuf[2];
    	    	    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
    	    	    if (!(bitword & mask))
		    {
			xbuf[0] = ~xbuf[0];
			xbuf[1] = ~xbuf[1];
			xbuf[2] = ~xbuf[2];
		    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		 } /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (bitword & mask)
		    {
			*xbuf++ = fg1;
			*xbuf++ = fg2;
			*xbuf++ = fg3;
		    }
		    else
		    {
			*xbuf++ = bg1;
			*xbuf++ = bg2;
			*xbuf++ = bg3;
		    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (bitword & mask)
		    {
			*xbuf++ = bg1;
			*xbuf++ = bg2;
			*xbuf++ = bg3;
		    }
		    else
		    {
			*xbuf++ = fg1;
			*xbuf++ = fg2;
			*xbuf++ = fg3;
		    }

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	bitarray += msg->modulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemTemplate32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemTemplate32 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *bitarray, *buf;
    UWORD   	    	     bitmask;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
    {
    	type = 0;
    }
    else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
    {
    	type = 2;
    }
    else
    {
    	type = 4;
    }
    
    if (msg->inverttemplate) type++;
    
    bitarray = msg->masktemplate + ((msg->srcx / 16) * 2);
    bitmask = 0x8000 >> (msg->srcx & 0xF);
    
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 4;
    
    for(y = 0; y < msg->height; y++)
    {
	ULONG  mask = bitmask;
	UWORD *array = (UWORD *)bitarray;
	UWORD  bitword = AROS_BE2WORD(*array);
	ULONG *xbuf = (ULONG *)buf;

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (bitword & mask) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
    	    	    if (!(bitword & mask)) *xbuf = ~(*xbuf);

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		 } /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? fg : bg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
     	    	    *xbuf++ = (bitword & mask) ? bg : fg;

		    mask >>= 1;
		    if (!mask)
		    {
			mask = 0x8000;
			array++;
			bitword = AROS_BE2WORD(*array);
		    }
		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	bitarray += msg->modulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemPattern8(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemPattern8 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *patarray, *buf, *maskarray = 0;
    UWORD   	    	     patmask, maskmask = 0;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (msg->patterndepth > 1)
    {
    	type = 6;
    }
    else
    {
	if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
	{
    	    type = 0;
	}
	else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
	{
    	    type = 2;
	}
	else
	{
    	    type = 4;
	}

	if (msg->invertpattern) type++;
    }

    patarray = msg->pattern;
    patmask = 0x8000 >> (msg->patternsrcx & 0xF);

    if ((maskarray = msg->mask))
    {
    	maskarray += (msg->masksrcx / 16) * 2;
	maskmask = 0x8000 >> (msg->masksrcx & 0xF);
    }
        
    buf = msg->dst + msg->y * msg->dstMod + msg->x;
    
    for(y = 0; y < msg->height; y++)
    {
	UWORD  pmask = patmask;
	UWORD  mmask = maskmask;
	UWORD *parray = ((UWORD *)patarray) + ((y + msg->patternsrcy) % msg->patternheight);
	UWORD  patword = AROS_BE2WORD(*parray);
	UWORD *marray = NULL;
	UWORD  maskword = 0;
	UBYTE *xbuf = (UBYTE *)buf;

	if (maskarray)
	{
	    marray = (UWORD *)maskarray;
	    maskword = AROS_BE2WORD(*marray);
	}

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
 
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? fg : bg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? bg : fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 6: /* multi color pattern */
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
			WORD plane;
			ULONG pixel = (patword & pmask) ? 1 : 0;

			for(plane = 1; plane < msg->patterndepth; plane++)
			{
			    UWORD *_parray = parray + plane * msg->patternheight;
			    UWORD _patword = AROS_BE2WORD(*_parray);

			    if (_patword & pmask) pixel |= 1L << plane;				
			}

			if (msg->patternlut) pixel = msg->patternlut->pixels[pixel];

    	    	    	xbuf[x] = pixel;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;

		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	if (maskarray) maskarray += msg->maskmodulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemPattern16(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemPattern16 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *patarray, *buf, *maskarray = 0;
    UWORD   	    	     patmask, maskmask = 0;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (msg->patterndepth > 1)
    {
    	type = 6;
    }
    else
    {
	if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
	{
    	    type = 0;
	}
	else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
	{
    	    type = 2;
	}
	else
	{
    	    type = 4;
	}

	if (msg->invertpattern) type++;
    }

    patarray = msg->pattern;
    patmask = 0x8000 >> (msg->patternsrcx & 0xF);

    if ((maskarray = msg->mask))
    {
    	maskarray += (msg->masksrcx / 16) * 2;
	maskmask = 0x8000 >> (msg->masksrcx & 0xF);
    }
        
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 2;
    
    for(y = 0; y < msg->height; y++)
    {
	UWORD  pmask = patmask;
	UWORD  mmask = maskmask;
	UWORD *parray = ((UWORD *)patarray) + ((y + msg->patternsrcy) % msg->patternheight);
	UWORD  patword = AROS_BE2WORD(*parray);
	UWORD *marray = NULL;
	UWORD  maskword = 0;
	UWORD *xbuf = (UWORD *)buf;

	if (maskarray)
	{
	    marray = (UWORD *)maskarray;
	    maskword = AROS_BE2WORD(*marray);
	}

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
 
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? fg : bg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? bg : fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 6: /* multi color pattern */
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
			WORD plane;
			ULONG pixel = (patword & pmask) ? 1 : 0;

			for(plane = 1; plane < msg->patterndepth; plane++)
			{
			    UWORD *_parray = parray + plane * msg->patternheight;
			    UWORD _patword = AROS_BE2WORD(*_parray);

			    if (_patword & pmask) pixel |= 1L << plane;				
			}

			if (msg->patternlut) pixel = msg->patternlut->pixels[pixel];

    	    	    	xbuf[x] = pixel;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;

		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	if (maskarray) maskarray += msg->maskmodulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}
/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemPattern24(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemPattern24 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *patarray, *buf, *maskarray = 0;
    UWORD   	    	     patmask, maskmask = 0;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    UBYTE   	    	     fg1, fg2, fg3;
    ULONG	    	     bg = GC_BG(gc);
    UBYTE   	    	     bg1, bg2, bg3;
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

#if AROS_BIG_ENDIAN        
    fg1 = (fg >> 16) & 0xFF;
    fg2 = (fg >> 8) & 0xFF;
    fg3 =  fg & 0xFF;

    bg1 = (bg >> 16) & 0xFF;
    bg2 = (bg >> 8) & 0xFF;
    bg3 =  bg & 0xFF;
#else
    fg1 =  fg & 0xFF;
    fg2 = (fg >> 8) & 0xFF;
    fg3 = (fg >> 16) & 0xFF;

    bg1 =  bg & 0xFF;
    bg2 = (bg >> 8) & 0xFF;
    bg3 = (bg >> 16) & 0xFF;
#endif

    if (msg->patterndepth > 1)
    {
    	type = 6;
    }
    else
    {
	if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
	{
    	    type = 0;
	}
	else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
	{
    	    type = 2;
	}
	else
	{
    	    type = 4;
	}

	if (msg->invertpattern) type++;
    }

    patarray = msg->pattern;
    patmask = 0x8000 >> (msg->patternsrcx & 0xF);

    if ((maskarray = msg->mask))
    {
    	maskarray += (msg->masksrcx / 16) * 2;
	maskmask = 0x8000 >> (msg->masksrcx & 0xF);
    }
        
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 3;
    
    for(y = 0; y < msg->height; y++)
    {
	UWORD  pmask = patmask;
	UWORD  mmask = maskmask;
	UWORD *parray = ((UWORD *)patarray) + ((y + msg->patternsrcy) % msg->patternheight);
	UWORD  patword = AROS_BE2WORD(*parray);
	UWORD *marray = NULL;
	UWORD  maskword = 0;
	UBYTE *xbuf = (UBYTE *)buf;

	if (maskarray)
	{
	    marray = (UWORD *)maskarray;
	    maskword = AROS_BE2WORD(*marray);
	}

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask)
			{
			    xbuf[0] = fg1;
			    xbuf[1] = fg2;
			    xbuf[2] = fg3;
			}
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
 
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask))
			{
			    xbuf[0] = fg1;
			    xbuf[1] = fg2;
			    xbuf[2] = fg3;
			}

		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask)
			{
			    xbuf[0] = ~xbuf[0];
			    xbuf[1] = ~xbuf[1];
			    xbuf[2] = ~xbuf[2];
			}
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask))
			{
			    xbuf[0] = ~xbuf[0];
			    xbuf[1] = ~xbuf[1];
			    xbuf[2] = ~xbuf[2];
			}
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask)
			{
			    xbuf[0] = fg1;
			    xbuf[1] = fg2;
			    xbuf[2] = fg3;
			}
			else
			{
			    xbuf[0] = bg1;
			    xbuf[1] = bg2;
			    xbuf[2] = bg3;
			}
			
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask)
			{
			    xbuf[0] = bg1;
			    xbuf[1] = bg2;
			    xbuf[2] = bg3;
			}
			else
			{
			    xbuf[0] = fg1;
			    xbuf[1] = fg2;
			    xbuf[2] = fg3;
			}
			
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 6: /* multi color pattern */
		for(x = 0; x < msg->width; x++, xbuf += 3)
		{
		    if (!maskarray || (maskword & mmask))
		    {
			WORD plane;
			ULONG pixel = (patword & pmask) ? 1 : 0;

			for(plane = 1; plane < msg->patterndepth; plane++)
			{
			    UWORD *_parray = parray + plane * msg->patternheight;
			    UWORD _patword = AROS_BE2WORD(*_parray);

			    if (_patword & pmask) pixel |= 1L << plane;				
			}

			if (msg->patternlut) pixel = msg->patternlut->pixels[pixel];

    	    	    #if AROS_BIG_ENDIAN
		    	xbuf[0] = (pixel >> 16) & 0xFF;
			xbuf[1] = (pixel >> 8) & 0xFF;
    	    	    	xbuf[2] = (pixel) & 0xFF;
		    #else
    	    	    	xbuf[0] = (pixel) & 0xFF;
			xbuf[1] = (pixel >> 8) & 0xFF;
		    	xbuf[2] = (pixel >> 16) & 0xFF;
		    #endif
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;

		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	if (maskarray) maskarray += msg->maskmodulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/

VOID BM__Hidd_BitMap__PutMemPattern32(OOP_Class *cl, OOP_Object *o, struct pHidd_BitMap_PutMemPattern32 *msg)
{
    WORD    	    	     x, y;
    UBYTE   	    	    *patarray, *buf, *maskarray = 0;
    UWORD   	    	     patmask, maskmask = 0;
    OOP_Object	    	    *gc = msg->gc;
    ULONG	     	     fg = GC_FG(gc);
    ULONG	    	     bg = GC_BG(gc);
    WORD    	    	     type = 0;
    
    if (msg->width <= 0 || msg->height <= 0)
	return;

    if (msg->patterndepth > 1)
    {
    	type = 6;
    }
    else
    {
	if (GC_COLEXP(gc) == vHidd_GC_ColExp_Transparent)
	{
    	    type = 0;
	}
	else if (GC_DRMD(gc) == vHidd_GC_DrawMode_Invert)
	{
    	    type = 2;
	}
	else
	{
    	    type = 4;
	}

	if (msg->invertpattern) type++;
    }

    patarray = msg->pattern;
    patmask = 0x8000 >> (msg->patternsrcx & 0xF);

    if ((maskarray = msg->mask))
    {
    	maskarray += (msg->masksrcx / 16) * 2;
	maskmask = 0x8000 >> (msg->masksrcx & 0xF);
    }
        
    buf = msg->dst + msg->y * msg->dstMod + msg->x * 4;
    
    for(y = 0; y < msg->height; y++)
    {
	UWORD  pmask = patmask;
	UWORD  mmask = maskmask;
	UWORD *parray = ((UWORD *)patarray) + ((y + msg->patternsrcy) % msg->patternheight);
	UWORD  patword = AROS_BE2WORD(*parray);
	UWORD *marray = NULL;
	UWORD  maskword = 0;
	ULONG *xbuf = (ULONG *)buf;

	if (maskarray)
	{
	    marray = (UWORD *)maskarray;
	    maskword = AROS_BE2WORD(*marray);
	}

	switch(type)
	{
	    case 0:	/* JAM1 */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
 
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 1:	/* JAM1 | INVERSVID */	    
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
		} /* for(x = 0; x < msg->width; x++) */
		break;

    	    case 2: /* COMPLEMENT */
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (patword & pmask) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 3: /* COMPLEMENT | INVERSVID*/
		for(x = 0; x < msg->width; x++, xbuf++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	if (!(patword & pmask)) *xbuf = ~(*xbuf);
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;
			
		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 4:	/* JAM2 */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? fg : bg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 5:	/* JAM2 | INVERSVID */	    
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
    	    	    	xbuf[x] = (patword & pmask) ? bg : fg;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;			

		} /* for(x = 0; x < msg->width; x++) */
		break;

	    case 6: /* multi color pattern */
		for(x = 0; x < msg->width; x++)
		{
		    if (!maskarray || (maskword & mmask))
		    {
			WORD plane;
			ULONG pixel = (patword & pmask) ? 1 : 0;

			for(plane = 1; plane < msg->patterndepth; plane++)
			{
			    UWORD *_parray = parray + plane * msg->patternheight;
			    UWORD _patword = AROS_BE2WORD(*_parray);

			    if (_patword & pmask) pixel |= 1L << plane;				
			}

			if (msg->patternlut) pixel = msg->patternlut->pixels[pixel];

    	    	    	xbuf[x] = pixel;
		    }

		    if (maskarray)
		    {
			mmask >>= 1;
			if (!mmask)
			{
			    mmask = 0x8000;
			    marray++;
			    maskword = AROS_BE2WORD(*marray);
			}
		    }

		    pmask >>= 1;
		    if (!pmask) pmask = 0x8000;

		} /* for(x = 0; x < msg->width; x++) */
		break;

	} /* switch(type) */

	buf += msg->dstMod;			     
	if (maskarray) maskarray += msg->maskmodulo;

    } /* for(y = 0; y < msg->height; y++) */
    
}

/****************************************************************************************/
