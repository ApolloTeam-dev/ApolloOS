
#ifndef INTERFACE_PIO_H
#define INTERFACE_PIO_H

#include <asm/io.h>
#include <exec/types.h>

//Standard Amiga Gayle Definitions
#define GAYLE_BASE_DD   0xdd0000
#define GAYLE_IRQ_DD    0xdd9000
#define GAYLE_INT_DD    0xdda000

#define GAYLE_BASE_DA   0xda0000
#define GAYLE_IRQ_DA    0xda9000
#define GAYLE_INT_DA    0xdaa000

#define GAYLE_IRQ_IDE   0x80
#define GAYLE_INT_IDE   0x80

struct pio_data
{
    UBYTE   *dataport;
    UBYTE   *port;
    UBYTE   *altport;
    BOOL    da;
    BOOL    v4;
};

extern const APTR bus_FuncTable[];
extern const APTR pio_FuncTable[];

#endif /* !INTERFACE_PIO_H */
