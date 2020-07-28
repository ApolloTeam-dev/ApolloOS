/* Partition parser main routines (partparse.c)
   (C) 2020 Alynna Trypnotk, license APL
 */


// Configuration section
#define DEVICE_NAME 	"sagasd.device"				// Exec device name
#define DEVICE_TEMPLATE "SD@P$"						// DOS device template:  @=unit number, $=Partition #
#define LBA_HEADS 		16							// Should match your driver, or LBA spec
#define LBA_SECTORS 	64							// Should match your driver, or LBA spec

// For porting to other drivers:  Put a routine that reads 512 bytes from a disk block into a buffer here.
#define READBLOCK(block, buffer) sdcmd_read_blocks(&sdu->sdu_SDCmd, block, buffer, 512);
// You also probably have to change all the SAGASDBase references to your own class.

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
#include "partparser.h"

static BOOL ParseRDB(struct SAGASDBase * SAGASDBase, ULONG unit, UBYTE data[], struct DosPacket *dos, UBYTE blk) {
	struct Library *SysBase = SAGASDBase->sd_ExecBase;
	struct SAGASDUnit *sdu = &SAGASDBase->sd_Unit[unit];
	UBYTE i=0;
	if (db(0x1c) == 0xffffffff) return FALSE;	// RDB without partitions 
	
	while (TRUE) {
		sderr = READBLOCK(blk, data);
		if (sderr) return FALSE;
		
		if ((ULONG)data[0x0x14] & (1<<2)) continue;  	// Skip drives marked no automount
		data[0x24+db(0x24)+1] = (UBYTE)0;           	// BSTR to string		
		// Finish naming unit
		for (j=0; db(0x24+j) == 0;, j++)
			if (db(0x24+j) == '$') dos->dp_DosName[j] = 48+i;
			else dos->dp_DosName[j] = db(0x24+j);
		dos->dp_DosName[j+1] = 0;
		
		// Fill in DosPacket
		dos->dp_ExecName = DEVICE_NAME;						// Exec device
		dos->dp_Flags |= dl(0x20);							// Merge requested DosFlags with ones on partition
		memcpy(&dos->env,&data[0x80],sizeof(DosEnvec));	// Copy partitions DosEnvec into our packet.
		// Add DevNode with a ConfigDev
		struct DeviceNode *devnode = MakeDosNode(dos);
		if (devnode) {
			struct ConfigDev *configdev = AllocConfigDev();
			configdev->cd_Flags = 0x9;
			configdev->cd_Driver = SAGASDBase.sd_Device;
			AddBootNode(d, 0 & ADNF_STARTPROC, devnode, configdev);
		}
		// If Next = -1 then no more partitions ...
		if (dl(0x10) == 0xffffffff) break;
			else blk = dl(0x10);  // else its the next block.
		i++;
	}
}

static BOOL ParseMBR(struct SAGASDBase * SAGASDBase, ULONG unit, UBYTE data[], struct DosPacket *dos, UBYTE parttype) {
	struct Library *SysBase = SAGASDBase->sd_ExecBase;
	struct SAGASDUnit *sdu = &SAGASDBase->sd_Unit[unit];
	
	// Get partition data .. not going to bother with extended partitions for now.  Maybe later.
	MBR part[4];
	part[0] = *(char[16] *)data[0x1be];
	part[1] = *(char[16] *)data[0x1ce];
	part[2] = *(char[16] *)data[0x1de];
	part[3] = *(char[16] *)data[0x1ee];
	
    struct Library *ExpansionBase = TaggedOpenLibrary(TAGGEDOPEN_EXPANSION);
	
	// Read all 4 partitions
	for(UBYTE j=0; j<4; j++) {
		if (part[j].status) {  // Has partition ID?
		if ((ULONG)(char[3](h1)) & ISLBA) { // With LBA we define our own values for sectors/heads
				dos->dp_BlocksPerTrack = 	(IPTR) LBA_SECTORS;
				dos->dp_Surfaces = 			(IPTR) LBA_HEADS;
				dos->dp_LowCyl = 			(IPTR) CYLINDER(part[j].lbas);
				dos->dp_HighCyl = 			(IPTR) CYLINDER(part[j].lbac) + dos->dp_LowCyl;				
			} else {	// For CHS we should base it on whats in the partition table
				dos->dp_BlocksPerTrack = 	(IPTR) CHS_HISECT(j)+1;
				dos->dp_Surfaces = 			(IPTR) CHS_HIHEAD(j)+1;
				dos->dp_LowCyl = 			(IPTR) CHS_LOCYLN(j);
				dos->dp_HighCyl = 			(IPTR) CHS_HICYLN(j);
			}
			dos->dp_DosType =			(IPTR) DosTypeOf(part[j].type);  // Make MBR type into Amiga type
			dos->dp_SectorsPerBlock = 	(IPTR) 1;				// All the rest of this stuff pretty standard
			dos->dp_SizeBlock = 		(IPTR) 512;
			dos->dp_PreAlloc = 			(IPTR) 0;
			dos->dp_Interleave = 		(IPTR) 0;
			dos->dp_SecOrg = 			(IPTR) 0;
			dos->dp_NumBuffers = 		(IPTR) MEMF_PUBLIC;
			dos->dp_Mask = 				(IPTR) 0xFFFFFFFE;
			dos->dp_BootPri = 			(IPTR) -128;
			dos->dp_Baud =				(IPTR) 0;
			dos->dp_Control =			(IPTR) 0;
			dos->dp_BootBlocks =		(IPTR) 2;
			// Finish naming unit
			for (j=0; (UBYTE)dos->dp_DosName[j] == 0;, j++)
				if (dos->dp_DosName[i] == '$') dos->dp_DosName[j] = 48+i;
			dos->dp_DosName[j+1] = 0;
			struct DeviceNode *devnode = MakeDosNode(dos);
			if (devnode) {
				struct ConfigDev *configdev = AllocConfigDev();
				// Fill in DosPacket
				dos->dp_ExecName = DEVICE_NAME;						// Exec device
				dos->dp_Flags |= dl(0x20);					// Merge requested DosFlags with ones on partition
				configdev->cd_Flags = 0x9;
				configdev->cd_Driver = SAGASDBase.sd_Device;
				// status & 0x80 = AmigaOS should presume its bootable
				// HaX, if you have in your partition table 0x82 as the status (first byte), it will give it a boot priority of 2.  It should still be valid to other OSes.
				AddBootNode((part[j].status & 0x80 ? (part[j].status & 0x7f)-j : -128), ADNF_STARTPROC, devnode, configdev);
			}
		}
	}
	CloseLibrary(ExpansionBase);
}

static BOOL Part2DOSList(struct SAGASDBase * SAGASDBase, ULONG unit, UBYTE data[], ULONG flags, struct DosPacket *dos) {
	struct Library *SysBase = SAGASDBase->sd_ExecBase;
	struct SAGASDUnit *sdu = &SAGASDBase->sd_Unit[unit];

	// Set up stuff we already know.    
    dos.dp_DosName = (IPTR)DEVICE_TEMPLATE;
    dos.dp_ExecName = (IPTR)DEVICE_NAME;
    dos.dp_Unit = unit;
    dos.dp_Flags = 0;

	// Set the unit number in the template
	for (UBYTE i=0; (UBYTE)dos.dp_DosName[i] == 0;, i++)
		if (dos.dp_DosNamet[i] == '@') dos.dp_Unit[i] = 48+unit;
	
	sderr = READBLK(0, data);
	if (sderr) return FALSE;
 
	if (dw(0x1fe) == 0x55aa) { // Some kind of MBR
		if (dw(0x1da) == 0x0000 && (dw(0x1bc) == 0x0000 || dw(0x1bc) == 0x5a5a) 
			if (*(char[6])(data[0x02]) == "NEWLDR") 
				return ParseMBR(SAGASDBase, unit, data[], dos, 2); // NEWLDR MBR
			else 
				return ParseMBR(SAGASDBase, unit, data[], dos, 1); // Modern MBR
		else
			return ParseMBR(SAGASDBase, unit, data[], dos, 0); // Old MBR
	} else {
		for (j=0; j<16, j++) { // RDB Seek
			sderr = READBLOCK(j, data);
			if ((char[4])data[0] == "RDSK") break;
		}
	}
	if (j == 16) 
		return FALSE;	// No RDB found
	else
		return ParseRDSK(struct SAGASDBase, unit, data[], dos, j) // Rigid disk block
}

static BOOL MkDosDevices(struct SAGASDBase * SAGASDBase, UBYTE unit, ULONG flags) {
	// Allocate memory for essential structs
	char data[512];
	struct DosPacket dos;
	// Start processing
	return Part2DosList(SAGASDBase, unit, data, flags, dos);
}


