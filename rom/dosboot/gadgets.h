#ifndef BOOTMENU_GADGETS_H
#define BOOTMENU_GADGETS_H

#include <intuition/classusr.h>
#include <intuition/intuition.h>

struct DOSBootBase;

struct ButtonGadget {
	struct Gadget *gadget;
	WORD XY1[6];
	WORD XY2[6];
	struct Border uborder1;
	struct Border uborder2;
	struct Border sborder1;
	struct Border sborder2;
};

struct MainGadgets {
	struct ButtonGadget *boot;
	struct ButtonGadget *bootnss;

};

#define BUTTON_BOOT            1
#define BUTTON_BOOT_WNSS       2


struct ButtonGadget *createButton(ULONG, ULONG, ULONG, ULONG, struct Gadget *, STRPTR, UWORD, struct DOSBootBase *);
void freeButtonGadget(struct ButtonGadget *, struct DOSBootBase *);
#endif
