/*
    Copyright © 1995-2011, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Draw the list of gels
    Lang: english
*/

#include <aros/debug.h>
#include <proto/graphics.h>

#include "graphics_intern.h"
#include "gfxfuncsupport.h"
#include "gels_internal.h"

#define CMOVE(c,a,b) {(c)->OpCode=COPPER_MOVE;(c)->DESTADDR=(int)(&a)&0xFFF;(c)->DESTDATA=b;}
#define CWAIT(c,a,b) {(c)->OpCode=COPPER_WAIT;(c)->VWAITPOS=a;(c)->HWAITPOS=b;}
#define CBUMP(acl) {++ci; ++((*acl)->Count);}
#define CEND(c) {CWAIT(c,10000,255);}

/*****************************************************************************

    NAME */
	#include <clib/graphics_protos.h>

	AROS_LH2(void, DrawGList,

/*  SYNOPSIS */
	AROS_LHA(struct RastPort *, rp, A1),
	AROS_LHA(struct ViewPort *, vp, A0),

/*  LOCATION */
	struct GfxBase *, GfxBase, 19, Graphics)

/*  FUNCTION
	Process the gel list, draw VSprites and Bobs.

    INPUTS
	rp - RastPort where Bobs will be drawn
	vp - ViewPort for VSprites

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

*****************************************************************************/
{
	AROS_LIBFUNC_INIT

	struct VSprite * CurVSprite = rp->GelsInfo->gelHead;
	struct VSprite * AfterPathVSprite = NULL;
	int              followdrawpath;
	struct VSprite * PrevVSprite = NULL;
	struct VSprite * OldVSprite = NULL;

	struct CopIns *ci = NULL;
	struct CopList *cl = NULL;
	struct CopIns *cic = NULL;
	struct CopList *clc = NULL;
	
	/*
	 * CurVSprite is not a valid VSprite but the "boundary vsprite"
	 * Should I follow the DrawPath?
	 */
	if (NULL != CurVSprite->IntVSprite && 
	    NULL != CurVSprite->IntVSprite->DrawPath) {
		followdrawpath = TRUE;
		AfterPathVSprite = CurVSprite->NextVSprite;
		CurVSprite = CurVSprite->IntVSprite->DrawPath;
	} else {
		followdrawpath = FALSE;
		CurVSprite = CurVSprite->NextVSprite;
	}
	OldVSprite = CurVSprite;
        
	// if we have real sprites, prepare the CL
	//-------------------------------------------
	while (CurVSprite) {
		if(CurVSprite->Flags & VSPRITE)
		{
			// Do we have old entries? Delete them
			if(vp->SprIns){
				if(vp->SprIns->CopIns)
				{
					FreeMem(vp->SprIns->CopIns, sizeof(struct CopIns)*32);
				}
				FreeMem(vp->SprIns, sizeof(struct CopList));
				vp->SprIns = NULL;
			}
			if(vp->ClrIns){
				if(vp->ClrIns->CopIns)
			  	{
					FreeMem(vp->ClrIns->CopIns, sizeof(struct CopIns)*32);
				}
				FreeMem(vp->ClrIns, sizeof(struct CopList));
				vp->ClrIns = NULL;
			}
			// New entry
			if(vp->SprIns == NULL)
			{
				vp->SprIns = (struct CopList *)AllocMem(sizeof(struct CopList), MEMF_ANY|MEMF_CLEAR);
				vp->SprIns->MaxCount = 64;
				vp->SprIns->CopIns = (struct CopIns*)AllocMem(sizeof(struct CopIns)*64, MEMF_ANY|MEMF_CLEAR);
				vp->SprIns->CopPtr = vp->SprIns->CopIns;
				vp->SprIns->_ViewPort = vp;
				vp->SprIns->Count = 0;
				vp->SprIns->Flags = 0;
				vp->SprIns->Next = 0;
				vp->SprIns->SLRepeat = 1;
			}

			if(vp->ClrIns == NULL)
			{
				vp->ClrIns = (struct CopList *)AllocMem(sizeof(struct CopList), MEMF_ANY|MEMF_CLEAR);
				vp->ClrIns->MaxCount = 32;
				vp->ClrIns->CopIns = (struct CopIns*)AllocMem(sizeof(struct CopIns)*32, MEMF_ANY|MEMF_CLEAR);
				vp->ClrIns->CopPtr = vp->ClrIns->CopIns;
				vp->ClrIns->_ViewPort = vp;
				vp->ClrIns->Count = 0;
				vp->ClrIns->Flags = 0;
				vp->ClrIns->Next = 0;
				vp->ClrIns->SLRepeat = 1;
			}
			vp->SprIns->Count = 0;
			vp->ClrIns->Count = 0;
			break;
		}
		CurVSprite = CurVSprite->NextVSprite;
	}
	CurVSprite = OldVSprite;

	cl = vp->SprIns;
	if(cl) ci = cl->CopPtr;

	clc = vp->ClrIns;
	if(clc) cic = clc->CopPtr;

	int picklist = 0;
	WORD *nl;

	// Set all nextlines to -1
	if(rp->GelsInfo->nextLine)
	{
		nl = rp->GelsInfo->nextLine;
		for(int i=0; i<8; i++)
			*(nl + i) = -1;
	}	

	/*
	 * As long as I don't step on the last boundary vsprite
	 */
	while (NULL != CurVSprite->NextVSprite) {
		/*
		 * Check a few flags that must not be set.
		 * The Bob should not be out of the rastport or already be drawn
		 */
		if ( 0 == (CurVSprite->Flags & (GELGONE)) ||
		   (NULL != (CurVSprite->VSBob) && 0 == (CurVSprite->VSBob->Flags & BDRAWN))) {
			/*
			 * If this VSprite was overlapping with other VSprites
			 * the follow the ClearPath first and clear all the
			 * VSprites on that path. That routine will also reset
			 * the ClearPath variable on all encountered VSprites.
			 * If it was not overlapping with other VSprites but
			 * was visible before it will either clear or back up
			 * the previous background.
			 */

			if (NULL != CurVSprite->VSBob)
				_ClearBobAndFollowClearPath(CurVSprite,rp,GfxBase);

			/*
			 * If I am supposed to back up the background then
			 * I have to do it now!!
			 * This unfortunatley has also to be done if the
			 * VSprite/Bob did not move since it could have
			 * changed its appearance.
			 */
			if (0 != (CurVSprite->Flags & SAVEBACK) && NULL != CurVSprite->VSBob)
			{
				BltRastPortBitMap(rp,
		                        	  CurVSprite->X,
		                        	  CurVSprite->Y,
		                        	  CurVSprite->IntVSprite->SaveBuffer,
		                        	  0,
		                        	  0,
		                        	  CurVSprite->Width << 4,
		                        	  CurVSprite->Height,
		                        	  0x0c0, GfxBase);

				CurVSprite->Flags |= BACKSAVED;
			}
			else kprintf("NOT SAVING BACKGROUND!\n");

			if (0 == (CurVSprite->Flags & VSPRITE) &&
			    BOBSAWAY == (CurVSprite->VSBob->Flags & BOBSAWAY)) {
				/*
				 * This Bob is to be removed...
				 */
				CurVSprite->PrevVSprite->NextVSprite = CurVSprite->NextVSprite;
				CurVSprite->NextVSprite->PrevVSprite = CurVSprite->PrevVSprite;
				/*
				 * This does not damage the drawpath and clearpath since
				 * the structure is not freed.
				 */
			} else {
				/*
				 * Now draw the VSprite/Bob at its current location.
				 */
				_ValidateIntVSprite(CurVSprite->IntVSprite, 
				                    rp, 
				                    FALSE,
				                    GfxBase);

				/* FIXME: Wrong minterm is used.
				 * Need to implement mintern '0xe0'.
				 */
                            
#if (0)
#define VS_MINTERM      0x0e0
#else
/* should be 0xe0! */
#define VS_MINTERM      0x0c0
#endif
                                if ((CurVSprite->VSBob) && (CurVSprite->VSBob->ImageShadow))
				{
                                    BltMaskBitMapRastPort(CurVSprite->IntVSprite->ImageData,
				                  0,
		                  		  0,
				                  rp,
				                  CurVSprite->X,
				                  CurVSprite->Y,
				                  CurVSprite->Width << 4,
				                  CurVSprite->Height,
				                  VS_MINTERM,
                                                  (PLANEPTR)CurVSprite->VSBob->ImageShadow);
				}
                                if (CurVSprite->VSBob)
                                {
                                    BltBitMapRastPort(CurVSprite->IntVSprite->ImageData,
				                  0,
		                  		  0,
				                  rp,
				                  CurVSprite->X,
				                  CurVSprite->Y,
				                  CurVSprite->Width << 4,
				                  CurVSprite->Height,
				                  VS_MINTERM );
				}
                                
#undef VS_MINTERM
				/*
				 * I will need to know the vsprite's coordinates
				 * that it has now the next time as well for clearing
				 * purposes.
				 */
				CurVSprite->OldX = CurVSprite->X;
				CurVSprite->OldY = CurVSprite->Y;

				if (CurVSprite->VSBob) {
					/*
					 * it's a bob! mark it as drawn.
					 */
					CurVSprite->VSBob->Flags |= BDRAWN;
					CurVSprite->VSBob->Flags &= ~BOBNIX;
				}
			}
		}
		// Real Sprite! No BOB
	        if(CurVSprite->Flags == VSPRITE)
		{
	            	struct Spr
	            	{
	                	UWORD sprpt[16];
	            	}; 

		   	int pickmask = 1;
	        	int pick = -1;
			for(int i=0; i<8; i++)
			{
		        	if((GfxBase->SpriteReserved & (pickmask<<i)) == 0)
				{
					// Take sprite and take care of mask
			    		if((rp->GelsInfo->sprRsrvd & (pickmask << i)) != 0)
			    		{
						if((picklist & (pickmask << i)) ==0)
						{
							picklist |= (pickmask << i);
							pick = i;
							break;
						}
					}
				}
			}

	        	if(pick > -1)
			{
		     	   	// 1. SPR (Sprite)
				volatile struct Custom *custom = (struct Custom *)0xdff000;

				struct Spr *spr = (struct Spr *)0xdff120;

				WORD tx, ty, t, dy, dx, h;
				UWORD pos, ctl, offset;
				WORD *nextline, *image;
		
				nextline = rp->GelsInfo->nextLine + pick;
				tx = GfxBase->ActiView->DxOffset;
				ty = GfxBase->ActiView->DyOffset;
			    
				if(CurVSprite->X < 0)dx = 0; else dx = CurVSprite->X;
				tx = tx + dx; // Should we do Hires >>1, SHIRES >> 2 ??
			    
				if(CurVSprite->Y < 0) dy = 0; else dy = CurVSprite->Y;
				ty = ty + dy;
				h = CurVSprite->Height - (dy-CurVSprite->Y);
				if((h+dy) > rp->BitMap->Rows)
			   		h = rp->BitMap->Rows - dy;
				t = ty + h;
				if((CurVSprite->X-(CurVSprite->Width<<4)>0) && (h > 0))
				{
					// PosCtl
			 		pos = (ty<<8) | ((tx >> 1) & 0xff); //(y<<15) | (x >> 1);
			   		ctl = (t << 8) | ((ty>>6)&0x4) | ((t>>7)&0x2) | (tx&0x1);
					
					// image clipping
			   		image = CurVSprite->ImageData + ((UWORD)CurVSprite->Depth * (UWORD)(dy-CurVSprite->Y));
			    		CWAIT(ci, *nextline, 0);
			   		CBUMP(&cl);
		 	    		CMOVE(ci, spr->sprpt[(pick<<1)],(WORD)(((LONG)image & 0xFFFF0000) >> 16));
			    		CBUMP(&cl);
			    		CMOVE(ci, spr->sprpt[(pick<<1)],(WORD)((LONG)image & 0xFFFF));
			    		ci->u3.u4.u1.DestAddr += 2;
			    		CBUMP(&cl);
					CMOVE(ci, custom->spr[pick].pos,pos);
					CBUMP(&cl);
			  		CMOVE(ci, custom->spr[pick].ctl,ctl);
			   		CBUMP(&cl);
			    		*nextline = CurVSprite->Y + (CurVSprite->Height - (dy-CurVSprite->Y)) + 2;
				
			   		 // 2. CLR (Color)
			    		if(clc)
			    		{
						if(CurVSprite->SprColors)
						{
							*(rp->GelsInfo->lastColor + pick) = CurVSprite->SprColors;
							CWAIT(ci, dy-1, 0);
							CBUMP(&cl);
							t = ((pick&0x6)<<1)+17;
							CMOVE(ci, custom->color[t],CurVSprite->SprColors[0]);
							CBUMP(&cl);
							CMOVE(ci, custom->color[++t],CurVSprite->SprColors[1]);
							CBUMP(&cl);
				    			CMOVE(ci, custom->color[++t],CurVSprite->SprColors[2]);
							CBUMP(&cl);
						}
					}
				}
			}		
		}
		
		/*
		 * Am I supposed to follow the drawpath.
		 * If yes then follow it to its end.
		 */
	 
		if (followdrawpath) {
			if (NULL != PrevVSprite)
				PrevVSprite->ClearPath = CurVSprite;
			PrevVSprite = CurVSprite;

			CurVSprite = CurVSprite->IntVSprite->DrawPath;
			if (NULL == CurVSprite) {
				followdrawpath = FALSE;
				PrevVSprite = NULL;
				CurVSprite = AfterPathVSprite;
			}
		} else {	
			if (NULL != PrevVSprite)
				PrevVSprite->ClearPath = CurVSprite;
			PrevVSprite = CurVSprite;
			CurVSprite = CurVSprite->NextVSprite;
			/*
			 * Does a DrawPath start here?
			 * If yes, then I will follow the DrawPath 
			 * after I am done with this VSprite.
			 */
			if (NULL != CurVSprite->IntVSprite->DrawPath) {
				followdrawpath = TRUE;
				AfterPathVSprite = CurVSprite->NextVSprite;
			}
		}
	} /* while not all bobs/vsprites are drawn */

	// Finish Copperlists
	if(ci)
	  CEND(ci);
	if(cic)
	  CEND(cic);
	
	AROS_LIBFUNC_EXIT
    
} /* DrawGList */
