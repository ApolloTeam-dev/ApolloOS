/*
   Copyright � 1995-2020, The AROS Development Team. All rights reserved.
   $Id$

   Desc: Main bootmenu code
   Lang: english
*/

#define __OOP_NOLIBBASE__

#include <aros/config.h>
#include <aros/debug.h>

#include <proto/bootloader.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/expansion.h>
#include <proto/oop.h>

#include <devices/keyboard.h>
#include <devices/rawkeycodes.h>
#include <devices/timer.h>
#include <exec/memory.h>
#include <graphics/driver.h>
#include <libraries/expansionbase.h>
#include <libraries/configvars.h>
#include <dos/filehandler.h>
#include <devices/trackdisk.h>
#include <exec/rawfmt.h>
#include <aros/bootloader.h>
#include <aros/symbolsets.h>

#include LC_LIBDEFS_FILE

#include "dosboot_intern.h"
#include "menu.h"

#define PAGE_MAIN 1
#define PAGE_BOOT 2
#define PAGE_DISPLAY 3
#define EXIT_BOOT 5
#define EXIT_BOOT_WNSS 6

#if (AROS_FLAVOUR & AROS_FLAVOUR_STANDALONE)
#ifdef __ppc__

#endif
#endif


static LONG centerx(LIBBASETYPEPTR DOSBootBase, LONG width)
{
    return (DOSBootBase->bm_Screen->Width - width) / 2;
}

static LONG rightto(LIBBASETYPEPTR DOSBootBase, LONG width, LONG right)
{
    return DOSBootBase->bm_Screen->Width - width - right;
}

static struct Gadget *createGadgetsBoot(LIBBASETYPEPTR DOSBootBase) 
{
    LONG cx = centerx((struct DOSBootBase *)DOSBootBase, 280);

    /* Create Option Gadgets */
    DOSBootBase->bm_MainGadgets.bootopt = createButton(
                                                        cx, 63, 280, 14,
                                                        NULL, "Boot Options...",
                                                        BUTTON_BOOT_OPTIONS, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.boot = createButton(
                                                    16, DOSBootBase->bottomY, 280, 14,
                                                    DOSBootBase->bm_MainGadgets.bootopt->gadget, "Boot",
                                                    BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.bootnss = createButton(
                                                    rightto((struct DOSBootBase *)DOSBootBase, 280, 16), DOSBootBase->bottomY, 280, 14,
                                                    DOSBootBase->bm_MainGadgets.boot->gadget, "Boot With No Startup-Sequence",
                                                    BUTTON_BOOT_WNSS, (struct DOSBootBase *)DOSBootBase);
    if (!DOSBootBase->bm_MainGadgets.bootopt ||
        !DOSBootBase->bm_MainGadgets.boot ||
        !DOSBootBase->bm_MainGadgets.bootnss)
        return NULL;
    return DOSBootBase->bm_MainGadgets.bootopt->gadget;
}


static void freeGadgetsBoot(LIBBASETYPEPTR DOSBootBase)
{
    freeButtonGadget(DOSBootBase->bm_MainGadgets.boot, (struct DOSBootBase *)DOSBootBase);
    freeButtonGadget(DOSBootBase->bm_MainGadgets.bootnss, (struct DOSBootBase *)DOSBootBase);
    freeButtonGadget(DOSBootBase->bm_MainGadgets.bootopt, (struct DOSBootBase *)DOSBootBase);
}



static struct Gadget *createGadgets(LIBBASETYPEPTR DOSBootBase, WORD page) 
{
    if (page == PAGE_MAIN)
        return createGadgetsBoot(DOSBootBase);
    
}
static void freeGadgets(LIBBASETYPEPTR DOSBootBase, WORD page)
{
    if (page == PAGE_MAIN)
        freeGadgetsBoot(DOSBootBase);
    }

static void toggleMode(LIBBASETYPEPTR DOSBootBase)
{
  
    if (OpenResource("ciaa.resource")) {
        volatile UWORD *beamcon0 = (UWORD*)0xdff1dc;
        GfxBase->DisplayFlags ^= PAL | NTSC;
        *beamcon0 = (GfxBase->DisplayFlags & PAL) ? 0x0020 : 0x0000;
    }
}

static UWORD msgLoop(LIBBASETYPEPTR DOSBootBase, struct Window *win, WORD page)
{
    WORD exit = -1;
    struct IntuiMessage *msg;
    struct Gadget *g;

    D(bug("[BootMenu] msgLoop(DOSBootBase @ %p, Window @ %p)\n", DOSBootBase, win));

    do
    {
        if (win->UserPort)
        {
            WaitPort(win->UserPort);
            while ((msg = (struct IntuiMessage *)GetMsg(win->UserPort)))
            {
                if (msg->Class == IDCMP_VANILLAKEY) {
                    if (msg->Code == 27)
                            exit = PAGE_MAIN;
                    else if (msg->Code >= '1' && msg->Code <= '3')
                            exit = PAGE_MAIN + msg->Code - '0';
                    else if (msg->Code >= 'a' && msg->Code <='j') {
                        BYTE pos = msg->Code - 'a', i = 0;
                        struct BootNode *bn;
                        DOSBootBase->bm_BootNode = NULL;

                        Forbid(); /* .. access to ExpansionBase->MountList */
                        ForeachNode(&DOSBootBase->bm_ExpansionBase->MountList, bn)
                        {
                            if (i++ == pos)
                            {
                                DOSBootBase->bm_BootNode = bn;
                                break;
                            }
                        }
                        Permit();

                        if (DOSBootBase->bm_BootNode != NULL)
                        {
                            /* Refresh itself */
                            exit = PAGE_BOOT;
                            break;
                        }
                    }
                    else
                        toggleMode(DOSBootBase);
                } else if (msg->Class == IDCMP_GADGETUP)
                {
                    g = msg->IAddress;
                    switch (g->GadgetID)
                    {
                    case BUTTON_BOOT:
                        DOSBootBase->db_BootFlags &= ~BF_NO_STARTUP_SEQUENCE;
                        exit = EXIT_BOOT;
                        break;
                    case BUTTON_BOOT_WNSS:
                        DOSBootBase->db_BootFlags |= BF_NO_STARTUP_SEQUENCE;
                        exit = EXIT_BOOT_WNSS;
                        break;
                    case BUTTON_CANCEL:
                        if (page == PAGE_BOOT)
                            DOSBootBase->bm_BootNode = NULL;
                        exit = PAGE_MAIN;
                        break;
                    case BUTTON_USE:
                        /* Preserve selected value */
                        if (page == PAGE_BOOT)
                            if (DOSBootBase->bm_BootNode != NULL)
                                DOSBootBase->db_BootNode = DOSBootBase->bm_BootNode;
                        /* Fallthrough */
                    
                    case BUTTON_BOOT_OPTIONS:
                        exit = PAGE_BOOT;
                        break;
                    
                    case BUTTON_DISPLAY_OPTIONS:
                        exit = PAGE_DISPLAY;
                        break;
                    }
                }
                ReplyMsg((struct Message *)msg);
            }
        }
        else
        {
            bug("[BootMenu] msgLoop: Window lacks a userport!\n");
            Wait(0);
        }
    } while (exit < 0);

    while ((msg=(struct IntuiMessage *)GetMsg(win->UserPort)))
        ReplyMsg(&msg->ExecMessage);

    return exit;
}



static void initPageBoot(LIBBASETYPEPTR DOSBootBase)
{
    struct Window *win = DOSBootBase->bm_Window;
    struct BootNode *bn;
    WORD y = 70;
    WORD xoff = (win->Width - 640) / 2;
    char text[100], *textp;

    SetAPen(win->RPort, 1);

    ForeachNode(&DOSBootBase->bm_ExpansionBase->MountList, bn)
    {
        struct DeviceNode *dn = bn->bn_DeviceNode;
        struct FileSysStartupMsg *fssm = BADDR(dn->dn_Startup);
        struct DosEnvec *de = NULL;
        struct IOStdReq *io;
        struct MsgPort *port;
        char dostype[5];
        UBYTE i;
        ULONG size;
        BOOL devopen, ismedia;

        if (y >= DOSBootBase->bottomY - 20)
            break;
        if (!fssm || !fssm->fssm_Device)
            continue;
        if (fssm->fssm_Environ > (BPTR)0x64) {
            de = BADDR(fssm->fssm_Environ);
            if (de->de_TableSize < 15)
                de = NULL;
        }

        NewRawDoFmt("%c%10s: %4d %s-%ld", RAWFMTFUNC_STRING, text,
            (DOSBootBase->bm_BootNode == bn) ? '*' : IsBootableNode(bn) ? '+' : ' ',
            AROS_BSTR_ADDR(dn->dn_Name),
            bn->bn_Node.ln_Pri,
            AROS_BSTR_ADDR(fssm->fssm_Device),
            fssm->fssm_Unit);
        Move(win->RPort, 20 + xoff, y);
        Text(win->RPort, text, strlen(text));

        textp = NULL;
        devopen = ismedia = FALSE;
        if ((port = (struct MsgPort*)CreateMsgPort())) {
            if ((io = (struct IOStdReq*)CreateIORequest(port, sizeof(struct IOStdReq)))) {
                if (!OpenDevice(AROS_BSTR_ADDR(fssm->fssm_Device), fssm->fssm_Unit, (struct IORequest*)io, fssm->fssm_Flags)) {
                    devopen = TRUE;
                    io->io_Command = TD_CHANGESTATE;
                    io->io_Actual = 1;
                    DoIO((struct IORequest*)io);
                    if (!io->io_Error && io->io_Actual == 0)
                        ismedia = TRUE;
                    CloseDevice((struct IORequest*)io);
                }
                DeleteIORequest((struct IORequest*)io);
            }
            DeleteMsgPort(port);
        }

        if (de && ismedia) {
            STRPTR sunit = "kMGT";

            for (i = 0; i < 4; i++) {
                dostype[i] = (de->de_DosType >> ((3 - i) * 8)) & 0xff;
                if (dostype[i] < 9)
                    dostype[i] += '0';
                else if (dostype[i] < 32)
                    dostype[i] = '.';
            }
            dostype[4] = 0;

           size = (de->de_HighCyl - de->de_LowCyl + 1) * de->de_Surfaces * de->de_BlocksPerTrack;
           /* try to prevent ULONG overflow */
           if (de->de_SizeBlock <= 128)
               size /= 2;
           else
               size *= de->de_SizeBlock / 256;
           while(size > 1024 * 10) { /* Wrap on 10x unit to be more precise in displaying */
               size /= 1024;
               sunit++;
           }

            NewRawDoFmt("%s [%08lx] %ld%c", RAWFMTFUNC_STRING, text,
                dostype, de->de_DosType,
                size, (*sunit));
            textp = text;
        } else if (!devopen) {
            textp = "[device open error]";
        } else if (!ismedia) {
            textp = "[no media]";
        }
        if (textp) {
            Move(win->RPort, 400 + xoff, y);
            Text(win->RPort, textp, strlen(textp));
        }

        y += 16;
        
        
    }
}

static void centertext(LIBBASETYPEPTR DOSBootBase, BYTE pen, WORD y, const char *text)
{
    struct Window *win = DOSBootBase->bm_Window;
    SetAPen(win->RPort, pen);
    Move(win->RPort, win->Width / 2 - TextLength(win->RPort, text, strlen(text)) / 2, y);
    Text(win->RPort, text, strlen(text));
}

static void initPage(LIBBASETYPEPTR DOSBootBase, WORD page)
{
    UBYTE *text;

    if (page == PAGE_DISPLAY)
            text = "Diiiisplay Options";
       else if (page == PAGE_BOOT)
        text = "Boot Options";
    else
        text = "ApollOS Early Startup Control";
    centertext(DOSBootBase, 2, 10, text);
    
    if (page == PAGE_BOOT)
    {
        /* Set the default */
        if (DOSBootBase->bm_BootNode == NULL)
            DOSBootBase->bm_BootNode = DOSBootBase->db_BootNode;

        initPageBoot(DOSBootBase);
        centertext(DOSBootBase, 1, 30, "Press A-J to select boot device");
        centertext(DOSBootBase, 1, 45, "\"+\" => bootable, \"*\" => selected for boot");

    }
    

    if (page == PAGE_MAIN && (GfxBase->DisplayFlags & (NTSC | PAL))) {
            ULONG modeid = GetVPModeID(&DOSBootBase->bm_Screen->ViewPort);
            if (modeid != INVALID_ID && (((modeid & MONITOR_ID_MASK) == NTSC_MONITOR_ID) || ((modeid & MONITOR_ID_MASK) == PAL_MONITOR_ID))) {
            centertext(DOSBootBase, 1, 30, "(Release 5, 2020-12-12)");
        }
    }

}

static WORD initWindow(LIBBASETYPEPTR DOSBootBase, struct BootConfig *bcfg, WORD page)
{
    struct Gadget *first = NULL;
    WORD newpage = -1;

    if ((first = createGadgets(DOSBootBase, page)) != NULL)
    {
        struct NewWindow nw =
        {
            0, 0,                            /* Left, Top */
            DOSBootBase->bm_Screen->Width,   /* Width, Height */
            DOSBootBase->bm_Screen->Height,
            0, 1,                            /* DetailPen, BlockPen */
            IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_VANILLAKEY | IDCMP_GADGETUP | IDCMP_GADGETDOWN, /* IDCMPFlags */
            WFLG_SMART_REFRESH | WFLG_BORDERLESS | WFLG_ACTIVATE, /* Flags */
            first,                            /* FirstGadget */
            NULL,                            /* CheckMark */
            NULL,                            /* Title */
            DOSBootBase->bm_Screen,             /* Screen */
            NULL,                            /* BitMap */
            0, 0,                                /* MinWidth, MinHeight */
            0, 0,                            /* MaxWidth, MaxHeight */
            CUSTOMSCREEN,                    /* Type */
        };

        D(bug("[BootMenu] initPage: Gadgets created @ %p\n", first));

        if ((DOSBootBase->bm_Window = OpenWindow(&nw)) != NULL)
        {
            D(bug("[BootMenu] initScreen: Window opened @ %p\n", DOSBootBase->bm_Window));
            D(bug("[BootMenu] initScreen: Window RastPort @ %p\n", DOSBootBase->bm_Window->RPort));
            D(bug("[BootMenu] initScreen: Window UserPort @ %p\n", DOSBootBase->bm_Window->UserPort));
            initPage(DOSBootBase, page);
            newpage = msgLoop(DOSBootBase, DOSBootBase->bm_Window, page);
        }
        CloseWindow(DOSBootBase->bm_Window);
    }
    freeGadgets(DOSBootBase, page);
    
    return newpage;
}

static BOOL initScreen(LIBBASETYPEPTR DOSBootBase, struct BootConfig *bcfg)
{
    WORD page;

    D(bug("[BootMenu] initScreen()\n"));

    page = -1;
    DOSBootBase->bm_Screen = OpenBootScreen(DOSBootBase);
    if (DOSBootBase->bm_Screen)
    {
        DOSBootBase->bottomY = 190;

        page = PAGE_MAIN;
        do {
            page = initWindow(DOSBootBase, bcfg, page);
        } while (page != EXIT_BOOT && page != EXIT_BOOT_WNSS);
        CloseBootScreen(DOSBootBase->bm_Screen, DOSBootBase);
    }
    return page >= 0;
}

/* From keyboard.device/keyboard_intern.h */
#define KB_MAXKEYS     256
#define KB_MATRIXSIZE  (KB_MAXKEYS/(sizeof(UBYTE)*8))
#define ioStd(x) ((struct IOStdReq *)x)

static BOOL buttonsPressed(LIBBASETYPEPTR DOSBootBase) 
{
    BOOL success = FALSE;
    struct MsgPort *mp = NULL;
    UBYTE matrix[KB_MATRIXSIZE];

    if (OpenResource("ciaa.resource"))
    {
            volatile UBYTE *cia = (UBYTE*)0xbfe001;
            volatile UWORD *potinp = (UWORD*)0xdff016;

            /* check left + right mouse button state */
            if ((cia[0] & 0x40) == 0 && (potinp[0] & 0x0400) == 0)
                return TRUE;
    }


    if ((mp = CreateMsgPort()) != NULL)
    {
        struct IORequest *io = NULL;
        if ((io = CreateIORequest(mp, sizeof ( struct IOStdReq))) != NULL)
        {
            if (0 == OpenDevice("keyboard.device", 0, io, 0))
            {
                D(bug("[BootMenu] buttonsPressed: Checking KBD_READMATRIX\n"));
                ioStd(io)->io_Command = KBD_READMATRIX;
                ioStd(io)->io_Data = matrix;
                ioStd(io)->io_Length = sizeof(matrix);
                DoIO(io);
                if (0 == io->io_Error)
                {
                    D(
                        int i;
                        bug("[BootMenu] buttonsPressed: Matrix : ");
                        for (i = 0; i < ioStd(io)->io_Actual; i ++)
                        {
                                bug("%02x ", matrix[i]);
                        }
                        bug("\n");
                    );
                    if (matrix[RAWKEY_SPACE/8] & (1<<(RAWKEY_SPACE%8)))
                    {
                            D(bug("[BootMenu] SPACEBAR pressed\n"));
                            success = TRUE;
                    }
                }
                CloseDevice(io);
            }
            DeleteIORequest(io); 
        }
        DeleteMsgPort(mp);
    }
    return success;
}

int bootmenu_Init(LIBBASETYPEPTR LIBBASE, BOOL WantBootMenu)
{
    BOOL bmi_RetVal = FALSE;

    D(bug("[BootMenu] bootmenu_Init()\n"));

    /* check keyboard if needed */
    if (!WantBootMenu)
        WantBootMenu = buttonsPressed(LIBBASE);

    /* Bring up early startup menu if requested */
    if (WantBootMenu)
    {
        D(kprintf("[BootMenu] bootmenu_Init: Entering Boot Menu ...\n"));
        bmi_RetVal = initScreen(LIBBASE, &LIBBASE->bm_BootConfig);
    }

    return bmi_RetVal;
}
