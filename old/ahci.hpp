#pragma once
// http://wiki.osdev.org/AHCI

#include <stddef.h>
#include <stdint.h>

typedef bool BOOL;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

#define FALSE false
#define TRUE true

typedef struct tagFIS_DEV_BITS
{
	BYTE type;
	
	BYTE receive:5;
	BYTE reserved0:1;
	BYTE i:1;
	BYTE reserved1:1;
	
	BYTE statusl:3;
	BYTE reserved2:1;
	BYTE statush:3;
	BYTE reserved3:1;
	
	BYTE error;
	DWORD s_active;
} FIS_DEV_BITS;

enum ahci_dev
{
	AHCI_DEV_NULL = 0,
	AHCI_DEV_SATA = 0x101,
	AHCI_DEV_SATAPI = 0xeb140101,
	AHCI_DEV_SEMB = 0xc33c0101,
	AHCI_DEV_PM = 0x96690101,
};

#define HBA_PORT_DET_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1

#define HBA_PxCMD_ST 1
#define HBA_PxCMD_FRE (1<<4)
#define HBA_PxCMD_FR (1<<14)
#define HBA_PxCMD_CR (1<<15)

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define HBA_PxIS_TFES (1<<30)

// SATA basic
/* There are at least two SATA standards maintained respectively by T13 and SATA-IO.
The SATA-IO focuses on serial ATA and T13 encompasses traditional parallel ATA specifications as well.
To a software developer, the biggest difference between SATA and parallel ATA
is that SATA uses FIS (Frame Information Structure) packet to transport data between
host and device, though at hardware level they differ much. An FIS can be viewed as
a data set of traditional task files, or an encapsulation of ATA commands. SATA uses
the same command set as parallel ATA. */
// 1) FIS types
/* Following code defines different kinds of FIS specified in Serial ATA Revision 3.0. */
typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

// 2) Register FIS – Host to Device
/* A host to device register FIS is used by the host to send command or control to a device.
As illustrated in the following data structure, it contains the IDE registers such as command,
LBA, device, feature, count and control. An ATA command is constructed in this structure and
issued to the device. All reserved fields in an FIS should be cleared to zero. */
typedef struct tagFIS_REG_H2D
{
	// DWORD 0
	BYTE	fis_type;	// FIS_TYPE_REG_H2D
 
	BYTE	pmport:4;	// Port multiplier
	BYTE	rsv0:3;		// Reserved
	BYTE	c:1;		// 1: Command, 0: Control
 
	BYTE	command;	// Command register
	BYTE	featurel;	// Feature register, 7:0
 
	// DWORD 1
	BYTE	lba0;		// LBA low register, 7:0
	BYTE	lba1;		// LBA mid register, 15:8
	BYTE	lba2;		// LBA high register, 23:16
	BYTE	device;		// Device register
 
	// DWORD 2
	BYTE	lba3;		// LBA register, 31:24
	BYTE	lba4;		// LBA register, 39:32
	BYTE	lba5;		// LBA register, 47:40
	BYTE	featureh;	// Feature register, 15:8
 
	// DWORD 3
	BYTE	countl;		// Count register, 7:0
	BYTE	counth;		// Count register, 15:8
	BYTE	icc;		// Isochronous command completion
	BYTE	control;	// Control register
 
	// DWORD 4
	BYTE	rsv1[4];	// Reserved
} FIS_REG_H2D;

// 3) Register FIS – Device to Host
/* A device to host register FIS is used by the device to notify the host that some ATA register has changed.
It contains the updated task files such as status, error and other registers. */
typedef struct tagFIS_REG_D2H
{
	// DWORD 0
	BYTE	fis_type;    // FIS_TYPE_REG_D2H
 
	BYTE	pmport:4;    // Port multiplier
	BYTE	rsv0:2;      // Reserved
	BYTE	i:1;         // Interrupt bit
	BYTE	rsv1:1;      // Reserved
 
	BYTE	status;      // Status register
	BYTE	error;       // Error register
 
	// DWORD 1
	BYTE	lba0;        // LBA low register, 7:0
	BYTE	lba1;        // LBA mid register, 15:8
	BYTE	lba2;        // LBA high register, 23:16
	BYTE	device;      // Device register
 
	// DWORD 2
	BYTE	lba3;        // LBA register, 31:24
	BYTE	lba4;        // LBA register, 39:32
	BYTE	lba5;        // LBA register, 47:40
	BYTE	rsv2;        // Reserved
 
	// DWORD 3
	BYTE	countl;      // Count register, 7:0
	BYTE	counth;      // Count register, 15:8
	BYTE	rsv3[2];     // Reserved
 
	// DWORD 4
	BYTE	rsv4[4];     // Reserved
} FIS_REG_D2H;

// 4) Data FIS – Bidirectional
/* This FIS is used by the host or device to send data payload. The data size can be varied. */
typedef struct tagFIS_DATA
{
	// DWORD 0
	BYTE	fis_type;	// FIS_TYPE_DATA
 
	BYTE	pmport:4;	// Port multiplier
	BYTE	rsv0:4;		// Reserved
 
	BYTE	rsv1[2];	// Reserved
 
	// DWORD 1 ~ N
	DWORD	data[1];	// Payload
} FIS_DATA;

// 5) PIO Setup – Device to Host
/* This FIS is used by the device to tell the host that it’s about to send or ready to receive a PIO data payload. */
typedef struct tagFIS_PIO_SETUP
{
	// DWORD 0
	BYTE	fis_type;	// FIS_TYPE_PIO_SETUP
 
	BYTE	pmport:4;	// Port multiplier
	BYTE	rsv0:1;		// Reserved
	BYTE	d:1;		// Data transfer direction, 1 - device to host
	BYTE	i:1;		// Interrupt bit
	BYTE	rsv1:1;
 
	BYTE	status;		// Status register
	BYTE	error;		// Error register
 
	// DWORD 1
	BYTE	lba0;		// LBA low register, 7:0
	BYTE	lba1;		// LBA mid register, 15:8
	BYTE	lba2;		// LBA high register, 23:16
	BYTE	device;		// Device register
 
	// DWORD 2
	BYTE	lba3;		// LBA register, 31:24
	BYTE	lba4;		// LBA register, 39:32
	BYTE	lba5;		// LBA register, 47:40
	BYTE	rsv2;		// Reserved
 
	// DWORD 3
	BYTE	countl;		// Count register, 7:0
	BYTE	counth;		// Count register, 15:8
	BYTE	rsv3;		// Reserved
	BYTE	e_status;	// New value of status register
 
	// DWORD 4
	WORD	tc;		// Transfer count
	BYTE	rsv4[2];	// Reserved
} FIS_PIO_SETUP;

// 6) DMA Setup – Device to Host
typedef struct tagFIS_DMA_SETUP
{
	// DWORD 0
	BYTE	fis_type;	// FIS_TYPE_DMA_SETUP
 
	BYTE	pmport:4;	// Port multiplier
	BYTE	rsv0:1;		// Reserved
	BYTE	d:1;		// Data transfer direction, 1 - device to host
	BYTE	i:1;		// Interrupt bit
	BYTE	a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed
 
        BYTE    rsved[2];       // Reserved
 
	//DWORD 1&2
 
        QWORD   DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
 
        //DWORD 3
        DWORD   rsvd;           //More reserved
 
        //DWORD 4
        DWORD   DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0
 
        //DWORD 5
        DWORD   TransferCount;  //Number of bytes to transfer. Bit 0 must be 0
 
        //DWORD 6
        DWORD   resvd;          //Reserved
 
} FIS_DMA_SETUP;

// AHCI Registers and Memory Structures
// 1) HBA memory registers
/* An AHCI controller can support up to 32 ports. HBA memory registers can be
divided into two parts: Generic Host Control registers and Port Control registers.
Generic Host Control registers controls the behavior of the whole controller,
while each port owns its own set of Port Control registers. The actual ports
an AHCI controller supported and implemented can be calculated from the Capacity register
(HBA_MEM.cap) and the Port Implemented register (HBA_MEM.pi). */
 
typedef volatile struct tagHBA_PORT
{
	DWORD	clb;		// 0x00, command list base address, 1K-byte aligned
	DWORD	clbu;		// 0x04, command list base address upper 32 bits
	DWORD	fb;		// 0x08, FIS base address, 256-byte aligned
	DWORD	fbu;		// 0x0C, FIS base address upper 32 bits
	DWORD	is;		// 0x10, interrupt status
	DWORD	ie;		// 0x14, interrupt enable
	DWORD	cmd;		// 0x18, command and status
	DWORD	rsv0;		// 0x1C, Reserved
	DWORD	tfd;		// 0x20, task file data
	DWORD	sig;		// 0x24, signature
	DWORD	ssts;		// 0x28, SATA status (SCR0:SStatus)
	DWORD	sctl;		// 0x2C, SATA control (SCR2:SControl)
	DWORD	serr;		// 0x30, SATA error (SCR1:SError)
	DWORD	sact;		// 0x34, SATA active (SCR3:SActive)
	DWORD	ci;		// 0x38, command issue
	DWORD	sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	DWORD	fbs;		// 0x40, FIS-based switch control
	DWORD	rsv1[11];	// 0x44 ~ 0x6F, Reserved
	DWORD	vendor[4];	// 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_MEM
{
	// 0x00 - 0x2B, Generic Host Control
	DWORD	cap;		// 0x00, Host capability
	DWORD	ghc;		// 0x04, Global host control
	DWORD	is;		// 0x08, Interrupt status
	DWORD	pi;		// 0x0C, Port implemented
	DWORD	vs;		// 0x10, Version
	DWORD	ccc_ctl;	// 0x14, Command completion coalescing control
	DWORD	ccc_pts;	// 0x18, Command completion coalescing ports
	DWORD	em_loc;		// 0x1C, Enclosure management location
	DWORD	em_ctl;		// 0x20, Enclosure management control
	DWORD	cap2;		// 0x24, Host capabilities extended
	DWORD	bohc;		// 0x28, BIOS/OS handoff control and status
 
	// 0x2C - 0x9F, Reserved
	BYTE	rsv[0xA0-0x2C];
 
	// 0xA0 - 0xFF, Vendor specific registers
	BYTE	vendor[0x100-0xA0];
 
	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[1];	// 1 ~ 32
} HBA_MEM;

// 2) Port Received FIS and Command List Memory
/* Each port can attach a single SATA device. Host sends commands to the device
using Command List and device delivers information to the host using Received FIS structure.
They are located at HBA_PORT.clb/clbu, and HBA_PORT.fb/fbu. The most important part of AHCI
initialization is to set correctly these two pointers and the data structures they point to. */

// 3) Received FIS
/* There are four kinds of FIS which may be sent to the host by the device as indicated
in the following structure declaration. When an FIS has been copied into the host specified memory,
an according bit will be set in the Port Interrupt Status register (HBA_PORT.is).
Data FIS – Device to Host is not copied to this structure. Data payload is sent and received
through PRDT (Physical Region Descriptor Table) in Command List, as will be introduced later. */
typedef volatile struct tagHBA_FIS
{
	// 0x00
	FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
	BYTE		pad0[4];
 
	// 0x20
	FIS_PIO_SETUP	psfis;		// PIO Setup FIS
	BYTE		pad1[12];
 
	// 0x40
	FIS_REG_D2H	rfis;		    // Register – Device to Host FIS
	BYTE		pad2[4];
 
	// 0x58
	FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS
 
	// 0x60
	BYTE		ufis[64];
 
	// 0xA0
	BYTE		rsv[0x100-0xA0];
} HBA_FIS;

// 4) Command List
/* Host sends commands to the device through Command List. Command List
consists of 1 to 32 command headers, each one is called a slot. Each command
header describes an ATA or ATAPI command, including a Command FIS, an ATAPI command buffer
and a bunch of Physical Region Descriptor Tables specifying the data payload address and size.
To send a command, the host constructs a command header, and set the according bit in the
Port Command Issue register (HBA_PORT.ci). The AHCI controller will automatically send the
command to the device and wait for response. If there are some errors, error bits in the Port
Interrupt register (HBA_PORT.is) will be set and additional information can be retrieved from
the Port Task File register (HBA_PORT.tfd), SStatus register (HBA_PORT.ssts) and SError register
(HBA_PORT.serr). If it succeeds, the Command Issue register bit will be cleared and the received
data payload, if any, will be copied from the device to the host memory by the AHCI controller.
How many slots a Command List holds can be got from the Host capability register (HBA_MEM.cap).
It must be within 1 and 32. SATA supports queued commands to increase throughput. Unlike traditional
parallel ATA drive; a SATA drive can process a new command when an old one is still running. With AHCI,
a host can send up to 32 commands to device simultaneously. */
typedef struct tagHBA_CMD_HEADER
{
	// DW0
	BYTE	cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
	BYTE	a:1;		// ATAPI
	BYTE	w:1;		// Write, 1: H2D, 0: D2H
	BYTE	p:1;		// Prefetchable
 
	BYTE	r:1;		// Reset
	BYTE	b:1;		// BIST
	BYTE	c:1;		// Clear busy upon R_OK
	BYTE	rsv0:1;		// Reserved
	BYTE	pmp:4;		// Port multiplier port
 
	WORD	prdtl;		// Physical region descriptor table length in entries
 
	// DW1
	volatile
	DWORD	prdbc;		// Physical region descriptor byte count transferred
 
	// DW2, 3
	DWORD	ctba;		// Command table descriptor base address
	DWORD	ctbau;		// Command table descriptor base address upper 32 bits
 
	// DW4 - 7
	DWORD	rsv1[4];	// Reserved
} HBA_CMD_HEADER;

// 5) Command Table and Physical Region Descriptor Table
/* As described above, a command table contains an ATA command FIS, an ATAPI command
buffer and a bunch of PRDT (Physical Region Descriptor Table) specifying the data payload address and size.
A command table may have 0 to 65535 PRDT entries. The actual PRDT entries count is set
in the command header (HBA_CMD_HEADER.prdtl). As an example, if a host wants to read 100K
bytes continuously from a disk, the first half to memory address A1, and the second half
to address A2. It must set two PRDT entries, the first PRDT.DBA = A1, and the second PRDT.DBA = A2.
An AHCI controller acts as a PCI bus master to transfer data payload directly between device and system memory. */ 
typedef struct tagHBA_PRDT_ENTRY
{
	DWORD	dba;		// Data base address
	DWORD	dbau;		// Data base address upper 32 bits
	DWORD	rsv0;		// Reserved
 
	// DW3
	DWORD	dbc:22;		// Byte count, 4M max
	DWORD	rsv1:9;		// Reserved
	DWORD	i:1;		// Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
	// 0x00
	BYTE	cfis[64];	// Command FIS
 
	// 0x40
	BYTE	acmd[16];	// ATAPI command, 12 or 16 bytes
 
	// 0x50
	BYTE	rsv[48];	// Reserved
 
	// 0x80
	HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;
