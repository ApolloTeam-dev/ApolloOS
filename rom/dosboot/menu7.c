/*
   Copyright (c) 2021 Vampires and Co LtD 
 
   Desc: Main bootmenu code
   Lang: english
*/

// removed ALL PC STUFF 2021.03.22
// reqtools is crashing - to fix

#define __OOP_NOLIBBASE__

#include <aros/config.h>
#include <aros/debug.h>

#include <proto/bootloader.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <devices/keyboard.h>
#include <devices/rawkeycodes.h>
#include <devices/timer.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <libraries/configvars.h>
#include <dos/filehandler.h>
#include <devices/trackdisk.h>
#include <exec/rawfmt.h>
#include <aros/bootloader.h>
#include <aros/symbolsets.h>

#include <libraries/reqtools.h>  //My
#include <proto/reqtools.h>    //my

#include <stdio.h>  //my 
#include <string.h>  //my
#include <stdlib.h> //my


#include LC_LIBDEFS_FILE

#include "dosboot_intern.h"
#include "menu.h"

#define PAGE_MAIN 1
#define PAGE_BOOT 2
#define PAGE_DISPLAY 3
#define EXIT_BOOT 5
#define EXIT_BOOT_WNSS 6

#define PROGNAME "rtfile"  //my

#if (AROS_FLAVOUR & AROS_FLAVOUR_STANDALONE)

#endif

 

static LONG centerx(LIBBASETYPEPTR DOSBootBase, LONG width)
{
    return (DOSBootBase->bm_Screen->Width - width) / 2;
}

static LONG rightto(LIBBASETYPEPTR DOSBootBase, LONG width, LONG right)
{
    return DOSBootBase->bm_Screen->Width - width - right;
}

// REQ test
static char s[300];

static void action(void)
{
    struct rtFileRequester *req;
    
    struct TagItem tags[] =
    {
        { RTFI_Flags, FREQF_PATGAD },
    	{ TAG_DONE }
    };
    
    if ((req = rtAllocRequestA(RT_FILEREQ, tags)))
    {
        rtFileRequestA(req, s, "FILE Name", tags);
	rtFreeRequest(req);
    }
}


static struct Gadget *createGadgetsBoot(LIBBASETYPEPTR DOSBootBase)
{
    LONG cx = centerx((struct DOSBootBase *)DOSBootBase, 280);

    /* Create Option Gadgets */
    DOSBootBase->bm_MainGadgets.bootopt = createButton(
                                                        345, 220, 100, 10, 
                                                        NULL, "HELP...",
                                                        BUTTON_BOOT_OPTIONS, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.boot = createButton(
                                                        100, 63, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Vampire Bootloader",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 75, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "ApolloOS",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 87, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Coffin",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 99, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "AmigaOS 1.3.3",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 111, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "AmiKit-XE",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);    
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 123, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "MacOS",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 135, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "EmuTOS",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 147, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Aros Vision",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 159, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Aros Base",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase); 
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 171, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "AMIGA OS 3.9",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase); 
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        100, 183, 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "AMIGA OS 3.1.4.1",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase); 

    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        330, 63 , 150, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Boot from Floppy",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);

    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        380, 75, 50, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "YES",
                                                        BUTTON_BOOT_WNSS, (struct DOSBootBase *)DOSBootBase);
     DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        380, 87, 50, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "NO",
                                                        BUTTON_BOOT_WNSS, (struct DOSBootBase *)DOSBootBase);                                                    
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 113, 120, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Click to Load",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 125, 120, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Click to Load",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase); 
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 137, 120, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Click to Load",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
     DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 149, 120, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Click to Load",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 161, 120, 10,
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Click to Load",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);
    DOSBootBase->bm_MainGadgets.displayopt = createButton(
                                                        345, 178, 120, 10, 
                                                        DOSBootBase->bm_MainGadgets.bootopt->gadget, "Turtle mode ON",
                                                        BUTTON_BOOT, (struct DOSBootBase *)DOSBootBase);                                                    
    DOSBootBase->bm_MainGadgets.bootnss = createButton(
                                                       cx, 200, 180, 10, 
                                                       DOSBootBase->bm_MainGadgets.displayopt->gadget, "Clean Boot",
                                                       BUTTON_BOOT_WNSS, (struct DOSBootBase *)DOSBootBase);


    /*DOSBootBase->bm_MainGadgets.bootnss = createButton(
                                                    rightto((struct DOSBootBase *)DOSBootBase, 10, 16), DOSBootBase->bottomY, 200, 14,
                                                    DOSBootBase->bm_MainGadgets.boot->gadget, "HELP",
                                                    BUTTON_BOOT_WNSS, (struct DOSBootBase *)DOSBootBase);*/


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
                           
                        //    exit = PAGE_BOOT;
                            break;
                        }
                        
                    }
                    else
                        toggleMode(DOSBootBase);


                } else if (msg->Class == IDCMP_GADGETUP)
                {
                    g = msg->IAddress;
                    if (DOSBootBase->bm_BootNode != NULL)
                            DOSBootBase->db_BootNode = DOSBootBase->bm_BootNode; //SAVE VALUES

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
                    }                               
                            
                }
                ReplyMsg((struct Message *)msg);
            }
        }
        else
        {
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

        if (y >= DOSBootBase->bottomY + 10) //???
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
        Move(win->RPort, 0 + xoff, y);
        //Text(win->RPort, text, strlen(text));
        SetAPen(win->RPort, 3);
        Text(win->RPort, text, 12);

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
                //if (dostype[i] < 9) oryginał
                if (dostype[i] < 12)
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
            Move(win->RPort, 255+ xoff, y);
            Text(win->RPort, textp, 4);
            //Text(win->RPort, textp, strlen(textp));
        }

        y += 12;
               
    }
}

static void centertext(LIBBASETYPEPTR DOSBootBase, BYTE pen, WORD y, const char *text)
{
    struct Window *win = DOSBootBase->bm_Window;
    SetAPen(win->RPort, pen);
    Move(win->RPort, win->Width / 2 - TextLength(win->RPort, text, strlen(text)) / 2, y);
    Text(win->RPort, text, strlen(text));
}
static void writetext(LIBBASETYPEPTR DOSBootBase, BYTE pen, WORD x, WORD y, const char *text)
{
    struct Window *win = DOSBootBase->bm_Window;
    SetAPen(win->RPort, pen);
    Move(win->RPort, x , y);
    Text(win->RPort, text, strlen(text));
}

static void initPage(LIBBASETYPEPTR DOSBootBase, WORD page)
{
    UBYTE *text;

    text = "Vampire Bootloader Early Startup Control";
    centertext(DOSBootBase, 2, 10, text);
  
    if (page == PAGE_MAIN && (GfxBase->DisplayFlags & (NTSC | PAL))) {
            ULONG modeid = GetVPModeID(&DOSBootBase->bm_Screen->ViewPort);
            if (modeid != INVALID_ID && (((modeid & MONITOR_ID_MASK) == NTSC_MONITOR_ID) || ((modeid & MONITOR_ID_MASK) == PAL_MONITOR_ID))) {
           // centertext(DOSBootBase, 1, 22, "(Core Release 6 build 8923, 2021-03-28)");
            //writetext(DOSBootBase, 3, 0, 65, "BOOT:");
            //writetext(DOSBootBase, 3, 0, 77, "DH0:");
            //writetext(DOSBootBase, 3, 0, 89, "DH2:");
            //writetext(DOSBootBase, 3, 0, 101, "DH4:");
            //writetext(DOSBootBase, 3, 0, 113, "DH5:");
            //writetext(DOSBootBase, 3, 0, 125, "DH6:");
            //writetext(DOSBootBase, 3, 0, 137, "DH7:");
            //writetext(DOSBootBase, 3, 0, 149, "DH8:");
            //writetext(DOSBootBase, 3, 0, 161, "DH9:");
            //writetext(DOSBootBase, 3, 0, 173, "DH10:");
            //writetext(DOSBootBase, 3, 0, 185, "SD CARD:");
            writetext(DOSBootBase, 3, 310, 120, "DF0:");
            writetext(DOSBootBase, 3, 310, 132, "DF1:");
            writetext(DOSBootBase, 3, 310, 144, "DF2:");
            writetext(DOSBootBase, 3, 310, 156, "DF3:");
            writetext(DOSBootBase, 3, 310, 168, "ROM:");
            writetext(DOSBootBase, 3, 130, 240, "SpaceBar toggle between PAL and NTSC");

        initPageBoot(DOSBootBase);
        centertext(DOSBootBase, 1, 34, "Press A-J to select boot device");
        centertext(DOSBootBase, 3, 46, "\"+\" are bootable, \"*\" => selected for boot");
            //centertext(DOSBootBase, 3, 240, "SpaceBar toggle between PAL and NTSC:");        
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
            0, 7,                            /* DetailPen, BlockPen */
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

        if ((DOSBootBase->bm_Window = OpenWindow(&nw)) != NULL)
        {
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
