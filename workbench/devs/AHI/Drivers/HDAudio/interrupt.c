/*
The contents of this file are subject to the AROS Public License Version 1.1 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
http://www.aros.org/license.html

Software distributed under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
ANY KIND, either express or implied. See the License for the specific language governing rights and
limitations under the License.

(C) Copyright 2010-2021 The AROS Dev Team
(C) Copyright 2009-2010 Stephen Jones.
(C) Copyright xxxx-2009 Davy Wentzler.

The Initial Developer of the Original Code is Davy Wentzler.

All Rights Reserved.
*/

#ifdef __AROS__
#include <aros/debug.h>
#endif

#include <config.h>

#include <proto/expansion.h>
#include <libraries/ahi_sub.h>
#include <proto/exec.h>
#include <stddef.h>
#include "library.h"
#include "regs.h"
#include "interrupt.h"
#include "misc.h"
#include "pci_wrapper.h"

#define min(a,b) ((a)<(b)?(a):(b))

int z = 0;
ULONG timer = 0; // for demo/test

#define TIME_LIMIT 150 // 150 irq's

/******************************************************************************
** Hardware interrupt handler *************************************************
******************************************************************************/


#ifdef __AMIGAOS4__
ULONG
CardInterrupt( struct ExceptionContext *pContext, struct ExecBase *SysBase, struct HDAudioChip* card )
#else
ULONG
CardInterrupt( struct HDAudioChip* card )
#endif
{
    ULONG intreq;
    LONG  handled = 0;
    D(UWORD statests;)
    UBYTE rirb_status;
    int i;

    intreq = pci_inl(HD_INTSTS, card);

    D(bug("[HDAudio] %s(%08x)\n", __func__, intreq);)
    if (intreq & HD_INTSTS_GIS)
    {
        D(bug("[HDAudio] %s: Global Interrupt\n", __func__);)
        if (intreq & 0x3fffffff) // stream interrupt
        {
//            ULONG position;
            BOOL playback = FALSE;
            BOOL recording = FALSE;

            //bug("Stream irq\n");
            for (i = 0; i < card->nr_of_streams; i++)
            {
                if (intreq & (1 << card->streams[i].index))
                {
                    // acknowledge stream interrupt
                    pci_outb(0x1C, card->streams[i].sd_reg_offset + HD_SD_OFFSET_STATUS, card);

                    if (i < card->nr_of_input_streams)
                    {
                        recording = TRUE;
                    }
                    else
                    {
                        playback = TRUE;
                    }
                }
            }
            
            pci_outb(0xFF, HD_INTSTS, card);

            z++;
#ifdef TIME_LIMITED
            timer++;

            if (timer > TIME_LIMIT) // stop playback
            {
                outb_clearbits(HD_SD_CONTROL_STREAM_RUN, card->streams[card->nr_of_input_streams].sd_reg_offset + HD_SD_OFFSET_CONTROL, card);
            }
#endif

            //bug("SIRQ\n");

            if (playback)
            {
              //  bug("PB\n");
//                position = pci_inl(card->streams[card->nr_of_input_streams].sd_reg_offset + HD_SD_OFFSET_LINKPOS, card);

                if (card->flip == 1) //position <= card->current_bytesize + 64)
                {
                   if (card->flip == 0)
                   {
                      D(bug("[HDAudio] Lost IRQ!\n"));
                   }
                   card->flip = 0;
                   card->current_buffer = card->playback_buffer1;
                }
                else
                {
                   if (card->flip == 1)
                   {
                      D(bug("[HDAudio] Lost IRQ!\n"));
                   }

                   card->flip = 1;
                   card->current_buffer = card->playback_buffer2;
                }

                Cause(&card->playback_interrupt);
            }

            if (recording)
            {
//                position = pci_inl(card->streams[0].sd_reg_offset + HD_SD_OFFSET_LINKPOS, card);

                if (card->recflip == 1) //position <= card->current_record_bytesize + 64)
                {
                   if (card->recflip == 0)
                   {
                      D(bug("[HDAudio] Lost rec IRQ!\n"));
                   }
                   card->recflip = 0;
                   card->current_record_buffer = card->record_buffer1;
                }
                else
                {
                   if (card->recflip == 1)
                   {
                      D(bug("[HDAudio] Lost rec IRQ!\n"));
                   }

                   card->recflip = 1;
                   card->current_record_buffer = card->record_buffer2;
                }

                Cause(&card->record_interrupt);
            }
        }

        if (intreq & HD_INTSTS_CIS)
        {
            D(bug("[HDAudio] %s: Controller Interrupt\n", __func__);)
            pci_outb(0x4, HD_INTSTS + 3, card); // only byte access allowed

            // check for RIRB status
            rirb_status = pci_inb(HD_RIRBSTS, card);
            D(bug("[HDAudio] %s: RIRB = %02x\n", __func__, rirb_status);)
            if (rirb_status & 0x5)
            {
                if (rirb_status & 0x4) // RIRBOIS
                {
//                    D(bug("[HDAudio] RIRB overrun!\n"));
                }

                if (rirb_status & 0x1) // RINTFL
                {
                    card->rirb_irq++;

                    /*if (card->rirb_irq > 1)
                    {
                       D(bug("[HDAudio] IRQ: rirb_irq = %d\n", card->rirb_irq));
                    }*/
                    //D(bug("[HDAudio] RIRB IRQ!\n"));
                }

                pci_outb(rirb_status, HD_RIRBSTS, card);
            }
            D(
              statests = pci_inw(HD_STATESTS, card);
              bug("[HDAudio] %s: STATESTS = %04x\n", __func__, statests);
            )
        }
        
        handled = 1;
    }

    return handled;
}


/******************************************************************************
** Playback interrupt handler *************************************************
******************************************************************************/

#ifdef __AMIGAOS4__
void
PlaybackInterrupt( struct ExceptionContext *pContext, struct ExecBase *SysBase, struct HDAudioChip* card )
#else
void
PlaybackInterrupt( struct HDAudioChip* card )
#endif
{
    struct AHIAudioCtrlDrv* AudioCtrl = card->audioctrl;
    struct DriverBase*  AHIsubBase = (struct DriverBase*) card->ahisubbase;

    if (card->mix_buffer != 0 && card->current_buffer != 0 && card->is_playing)
    {
        BOOL   skip_mix;

        int    i;
        LONG* srclong, *dstlong;
        int frames = card->current_frames;

        skip_mix = CallHookPkt(AudioCtrl->ahiac_PreTimerFunc, (Object*) AudioCtrl, 0);
        CallHookPkt(AudioCtrl->ahiac_PlayerFunc, (Object*) AudioCtrl, NULL);

        if (!skip_mix)
        {
#if defined(__AROS__) && (__WORDSIZE==64)
            CallHookPkt(AudioCtrl->ahiac_MixerFunc, (Object*) AudioCtrl, (APTR)(((IPTR)card->upper_mix_buffer << 32) | card->lower_mix_buffer));
#else
            CallHookPkt(AudioCtrl->ahiac_MixerFunc, (Object*) AudioCtrl, (APTR)card->lower_mix_buffer);
#endif
        }

        /* Now translate and transfer to the DMA buffer */
#if defined(__AROS__) && (__WORDSIZE==64)
        srclong = (LONG*) (((IPTR)card->upper_mix_buffer << 32) | card->lower_mix_buffer);
        dstlong = (LONG*) (((IPTR)card->upper_current_buffer << 32) | card->lower_current_buffer);
#else
        srclong = (LONG*) card->lower_mix_buffer;
        dstlong = (LONG*) card->lower_current_buffer;
#endif

        i = frames;

        if (AudioCtrl->ahiac_Flags & AHIACF_HIFI)
        {
            if (card->bitsizes[card->selected_bitsize_index] == 24)
            {
                /* 32-bit mixing buffer, 24-bit hardware format, stereo => frame size of 8 bytes */
                while(i > 0)
                {
                    *dstlong++ = *srclong++;
                    *dstlong++ = *srclong++;

                    --i;
                }
            }
        }

        if (!(AudioCtrl->ahiac_Flags & AHIACF_HIFI))
        {
            if (card->bitsizes[card->selected_bitsize_index] == 16)
            {
                /* 16-bit mixing buffer, 16-bit hardware format, stereo => frame size of 4 bytes */
                while(i > 0)
                {
                    *dstlong++ = *srclong++;

                    --i;
                }
            }
        }

        /* Note: other combinations of mixing buffer and hardware format are not supported at this moment */

        CallHookPkt(AudioCtrl->ahiac_PostTimerFunc, (Object*) AudioCtrl, 0);
    }
}


/******************************************************************************
** Record interrupt handler ***************************************************
******************************************************************************/

#ifdef __AMIGAOS4__
void
RecordInterrupt( struct ExceptionContext *pContext, struct ExecBase *SysBase, struct HDAudioChip* card )
#else
void
RecordInterrupt( struct HDAudioChip* card )
#endif
{
    struct AHIAudioCtrlDrv* AudioCtrl = card->audioctrl;
    struct DriverBase*  AHIsubBase = (struct DriverBase*) card->ahisubbase;
#ifdef __AMIGAOS4__
    int i = 0;
    int frames = card->current_record_bytesize / 2;
     WORD *src = card->lower_current_record_buffer;
     WORD* dst = card->lower_current_record_buffer;
#endif
    
    struct AHIRecordMessage rm =
    {
        AHIST_S16S,
#if defined(__AROS__) && (__WORDSIZE==64)
        (APTR)(((IPTR)card->upper_current_record_buffer << 32) | card->lower_current_record_buffer),
#else
        (APTR)card->lower_current_record_buffer,
#endif        
        RECORD_BUFFER_SAMPLES
    };

#ifdef __AMIGAOS4__
     while( i < frames )
     {
       *dst = ( ( *src & 0x00FF ) << 8 ) | ( ( *src & 0xFF00 ) >> 8 );

       ++i;
       ++src;
       ++dst;
     }
#else
     /*while( i < frames )
     {
       *dst = (*src);

       ++i;
       ++src;
       ++dst;
     }*/
#endif

    CallHookPkt(AudioCtrl->ahiac_SamplerFunc, (Object*) AudioCtrl, &rm);
}


