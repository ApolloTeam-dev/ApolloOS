/* Partition parser primary structs (partparse.h)
   (C) 2020 Alynna Trypnotk, license APL
 */

#include <dos/filehandler.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/types.h>
#include <exec/io.h>
#include <libraries/expansion.h>
#include <proto/exec.h>
#include <proto/disk.h>
#include <proto/expansion.h>
#include <dos/filehandler.h>

// Macros to directly access offsets in a block
#define db(x) *(UBYTE)data[x]
#define dw(x) *(UWORD)data[x]
#define dl(x) *(ULONG)data[x]

// MBR/RDB decision macros
#define ISLBA (0xffffff || 0xfeffff)
#define CYLINDER(x) x / (LBA_HEADS*LBA_SECTORS)

#if 0		// Fast inline endian change
#define be.l(x) register ULONG _l080_ = x; asm inline ("movex.l %0,%0" : "r" (_l080_)); _l080_;
#define be.w(x) register UWORD _w080_ = x; asm inline ("movex.w %0,%0" : "r" (_w080_)); _w080_;
#else				// Not so fast inline endian change
#define be.l(x) __builtin_bswap32(x)
#define be.w(x) __builtin_bswap16(x)
#endif

// The type for the struct below.
struct MBRTable {
	UBYTE htype;
	UBYTE flags;
	char[4] atype;
	char *handler;
};

// Map MBR partition numbers to Amiga types and handlers
static const MBRTable fsinfo[] = {
//  TYPE  FLAG  Amiga Type          Amiga Handler
	0x01, 0x50,	"MSD"+(char)0, 		"fat-handler",		// Flags
	0x04, 0xD0,	"MSD"+(char)0, 		"fat-handler",		// Bit 7: Check for LBA
	0x05, 0x20,	"EBR"+(char)0,	 	"",					// Bit 6: Check for CHS
	0x06, 0xC0,	"FAT"+(char)95, 	"fat95",			// Bit 5: Extended partition type
	0x07, 0x80,	"NTFS", 			"ntfs-handler",		// Bit 4: Check for Atari FATFS
	0x08, 0x40,	"C=FT", 			"fat-handler",		// // Commodore used this in their own roll of MS-DOS
	0x0B, 0xC0,	"FAT"+(char)95, 	"fat95",			// Bit 3: Amiga Native type
	0x0C, 0x80,	"FAT"+(char)95, 	"fat95",            // Bit 2: Declare Bootable
	0x0E, 0x80,	"MSD"+(char)1,	 	"fat-handler",      // Bit 1: Seek partition for filesystems
	0x0F, 0x80,	"EBR"+(char)1,	 	"",                 // Bit 0: 
	0x27, 0x80,	"MSW"+(char)10, 	"ntfs-handler",		// Recovery partition
	0x30, 0x8C,	"RDSK",	 			"",					// RDB disk inside partition (Amithlon, WinUAE)
	0x42, 0x8C,	"DOS"+(char)0,	 	"sfs-handler",		// Amiga SFS on MBR, some emulators
	0x76, 0xAC,	"RDSK",				"",					// RDB disk inside partition (Amithlon, WinUAE)
	0x82, 0xC0,	"SWAP",			 	"",					// Linux Swap
	0x83, 0xC0,	"EXT"+(char)2,	 	"ext-handler",		// Linux Ext. Filesystem
	0x9C, 0x8C,	"DOS"+(char)0,	 	"afs-handler",		// Amiga FFS on MBR  	// These are set up so
	0x9D, 0x8C,	"SFS"+(char)0,	 	"sfs-handler",		// Amiga SFS on MBR  	// that, in theory, you
	0x9E, 0x8C,	"PFS"+(char)3,	 	"pfs3aio",	     	// Amiga PFS on MBR  	// can boot from MBR
	0xAF, 0x80,	"HFS+",			 	"hfs-handler",		// Apple HFS
	0xD8, 0x40,	"CPM"+char(86),	 	"cpm-handler",		// CP/M-86 or CP/M-68k
	0xDA, 0xC0,	"DATA",				"",					// Non-FS data	
	0xEF, 0x80,	"EFI"+char(0),		"",					// EFI 	
	0xFB, 0xC0,	"RAID",				"",					// Linux RAID	
	0x00, 0x00, "NULL",				"",					// End of List
};
// MBR decision table
#define DosTypeOf(type) UBYTE _type_=0; while (fsinfo[_type_].htype) if (type == fsinfo[_type_].htype) { fsinfo[_type_].atype; break; }
#define DosHandlerOf(type) UBYTE _type_=0; while (fsinfo[_type_].htype) if ((char[4])type == fsinfo[_type_].htype) { fsinfo[_type_].handler; break; }
#define AmigaHandlerOf(type) UBYTE _type_=0; while (fsinfo[_type_].htype) if ((char[4])type == fsinfo[_type_].atype) { fsinfo[_type_].handler; break; }


// Structure of an MBR
struct MBR {
	UBYTE status;
	UBYTE h1;
	UBYTE cs1;
	UBYTE c1;
	UBYTE type;
	UBYTE h2;
	UBYTE cs2;
	UBYTE c2;
	ULONG lbas;
	ULONG lbac;	
};
#define CHS_LOCYLN(x) (part[x].c1 | (part[x].cs1 & 0x03) << 8)
#define CHS_HICYLN(x) (part[x].c2 | (part[x].cs2 & 0x03) << 8)
#define CHS_LOSECT(x) (part[x].cs1 & 0xfc >> 2)
#define CHS_HISECT(x) (part[x].cs2 & 0xfc >> 2)
#define CHS_LOHEAD(x) (part[x].h1)
#define CHS_HIHEAD(x) (part[x].h2)


// Structure of an RDB
struct RDB {
	char[4] sign;
	ULONG size;
	ULONG checksum;
	ULONG hostid;
	ULONG blocksize;
	ULONG flags;
	ULONG badblkptr;
	ULONG partptr;
	ULONG fshdrptr;
	ULONG drvinitptr;
}; // we dont care about the drives physical characteristics

// Structure of a partition block with DosEnvec
struct PART {
	char[4] sign;
	ULONG size;
	ULONG checksum;
	ULONG hostid;
	ULONG partptr;
	ULONG flags;
	ULONG badblkptr;
	ULONG rsvd1;
	ULONG devflags;
	ULONG drvnamelen;
	UBYTE drvnamelen;
	char drvname[31];
	IPTR rsvd2[15];
	DosEnvec env;
};

struct DosPacket {
	IPTR dp_DosName;        /* DOS name (like DH0) */
	IPTR dp_ExecName;       /* Exec Name (like sagasd.device) */
	IPTR dp_Unit;           /* Unit # */
	IPTR dp_Flags;			/* For OpenDevice() */
  union { 					/* You'll see why I did this soon */
	DosEnvec env;           /* Start of DosEnvec */
	IPTR dp_TableSize;      /* Size of this structure. Must be at least 11 (DE_NUMBUFFERS). */
  }
    IPTR dp_SizeBlock;      /* Size in longwords of a block on the disk. */
    IPTR dp_SecOrg;         /* Unused. Must be 0 for now. */
    IPTR dp_Surfaces;       /* Number of heads/surfaces in drive. */
    IPTR dp_SectorPerBlock; /* Unused. Must be 1 for now. */
    IPTR dp_BlocksPerTrack; /* Number of blocks on a track. */
    IPTR dp_Reserved;       /* Number of reserved blocks at beginning of volume. */
    IPTR dp_PreAlloc;       /* Number of reserved blocks at end of volume. */
    IPTR dp_Interleave;
    IPTR dp_LowCyl;         /* First cylinder. */
    IPTR dp_HighCyl;        /* Last cylinder. */
    IPTR dp_NumBuffers;     /* Number of buffers for drive. */
    IPTR dp_BufMemType;     /* Type of memory for buffers. See <exec/memory.h>. */
    IPTR dp_MaxTransfer;    /* How many bytes may be transferred together? */
    IPTR dp_Mask;           /* Memory address mask for DMA devices. */
    SIPTR dp_BootPri;       /* Priority of Autoboot. */
    IPTR dp_DosType;        /* Type of disk. See <dos/dos.h> for definitions. */
    IPTR dp_Baud;           /* Baud rate to use. */
    IPTR dp_Control;        /* Control word. */
    IPTR dp_BootBlocks;     /* Size of bootblock. */
}


