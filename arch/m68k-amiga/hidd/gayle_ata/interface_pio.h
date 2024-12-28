
#ifndef INTERFACE_PIO_H
#define INTERFACE_PIO_H

#include <asm/io.h>
#include <exec/types.h>
/*
 * Standard Amiga Gayle Definitions
 */
#if VAMPIRECARDSERIES==2
#define GAYLE_BASE_1200         0xda0000                        /* 0xda0000.W, 0xda0004.B, 0xda0008.B ...               */
#define GAYLE_IRQ_1200          0xda9000
#define GAYLE_INT_1200          0xdaa000
#else
#define GAYLE_BASE_1200         0xdd0000                        /* 0xdd0000.W, 0xdd0004.B, 0xdd0008.B ...               */
#define GAYLE_IRQ_1200          0xdd9000
#define GAYLE_INT_1200          0xdda000
#endif

#define GAYLE_BASE_4000         0xdd2022                        /* 0xdd2020.W, 0xdd2026.B, 0xdd202a.B ... (argh!)       */
#define GAYLE_IRQ_4000          0xdd3020

#define GAYLE_BASE_500          0xda0000                         /* Let's call it A500 even if it's used in others as well */
#define GAYLE_IRQ_500           0xda9000
#define GAYLE_INT_500           0xdaa000

#define GAYLE_IRQ_IDE           0x80
#define GAYLE_INT_IDE           0x80

struct pio_data
{
    UBYTE *dataport;
    UBYTE *port;
    UBYTE *altport;
    BOOL a500;
};

extern const APTR bus_FuncTable[];
extern const APTR pio_FuncTable[];

#endif /* !INTERFACE_PIO_H */
