#include "ahci.hpp"

#include "memory.hpp"
#include "terminal.hpp"

// Detect attached SATA devices
// 1) Which port is device attached
/* As specified in the AHCI specification, firmware (BIOS) should initialize the AHCI controller
into a minimal workable state. OS usually needn’t reinitialize it from the bottom.
Much information is already there when the OS boots.
The Port Implemented register (HBA_MEM.pi) is a 32 bit value and each bit represents a port.
If the bit is set, the according port has a device attached, otherwise the port is free. */

// 2) What kind of device is attached
/* There are four kinds of SATA devices, and their signatures are defined as below.
The Port Signature register (HBA_PORT.sig) contains the device signature, just read
this register to find which kind of device is attached at the port. Some buggy AHCI
controllers may not set the Signature register correctly. The most reliable way is to
judge from the Identify data read back from the device. */
#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier
 
// Check device type
/*static */int check_type(HBA_PORT *port)
{
	DWORD ssts = port->ssts;
 
	BYTE ipm = (ssts >> 8) & 0x0F;
	BYTE det = ssts & 0x0F;
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;
 
	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

#include "terminal.hpp"
#include "cstring.hpp"
void trace_ahci(const char* const str, const int num)
{
    term::write(str, false);
    term::writeline((size_t)num);
}

void trace_ahci(const char* const str)
{
    term::writeline(str, false);
}
 
void probe_port(HBA_MEM *abar)
{
	// Search disk in impelemented ports
	DWORD pi = abar->pi;

	for (size_t i = 0; i < 32; i++)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);

			if (dt == AHCI_DEV_SATA)
			{
				trace_ahci("SATA drive found at port ", i);
			}
			else if (dt == (int)AHCI_DEV_SATAPI)
			{
				trace_ahci("SATAPI drive found at port ", i);
			}
			else if (dt == (int)AHCI_DEV_SEMB)
			{
				trace_ahci("SEMB drive found at port ", i);
			}
			else if (dt == (int)AHCI_DEV_PM)
			{
				trace_ahci("PM drive found at port ", i);
			}
			else
			{
				//trace_ahci("No drive found at port ", i);
			}
		}
 
		pi >>= 1;
	}
}

// AHCI port memory space initialization
/* BIOS may have already configured all the necessary AHCI memory spaces.
But the OS usually needs to reconfigure them to make them fit its requirements.
It should be noted that Command List must be located at 1K aligned memory address
and Received FIS be 256 bytes aligned.
Before rebasing Port memory space, OS must wait for current pending commands to finish
and tell HBA to stop receiving FIS from the port. Otherwise an accidently incoming FIS
may be written into a partially configured memory area. This is done by checking and
setting corresponding bits at the Port Command And Status register (HBA_PORT.cmd).
The example subroutines stop_cmd() and start_cmd() do the job.
The following example assumes that the HBA has 32 ports implemented and each port
contains 32 command slots, and will allocate 8 PRDTs for each command slot. */
#define	AHCI_BASE	0x400000	// 4M
 
// Start command engine
void start_cmd(HBA_PORT *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(HBA_PORT *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	//while(port->cmd & HBA_PxCMD_FR || port->cmd & HBA_PxCMD_CR);
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
}

void* memset(void* str, char c, size_t n)
{
    char* cptr = (char*)str;
    
    for (size_t i = 0; i < n; i++)
    {
        cptr[i] = c;
    }

    return str;
}
 
void port_rebase(HBA_PORT *port, int portno)
{
	stop_cmd(port);	// Stop command engine	

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset(phystovirt(port->clb), 0, 1024);

	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset(phystovirt(port->fb), 0, 256);
 
	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)phystovirt(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset(phystovirt(cmdheader[i].ctba), 0, 256);
	}
 
	start_cmd(port);	// Start command engine
}

// Example - Read hard disk sectors
/* The code example shows how to read "count" sectors from sector offset "starth:startl"
to "buf" with LBA48 mode from HBA "port". Every PRDT entry contains 8K bytes data payload at most. */
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
 
// Find a free command list slot
int find_cmdslot(HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	DWORD slots = (port->sact | port->ci);

	for (int i = 0; i < 32; i++)
	{
		if ((slots & 1) == 0)
		{
			return i;
		}

		slots >>= 1;
	}

	trace_ahci("Cannot find free command list entry");
	return -1;
}
 
BOOL read(HBA_PORT *port, QWORD start, DWORD count, WORD *buf)
{
	port->is = (DWORD)-1; // Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int cmdslot = find_cmdslot(port);
	if (cmdslot == -1)
	{
		return FALSE;
	}
 
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)phystovirt(port->clb);
	cmdheader += cmdslot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(DWORD);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (WORD)((count-1)>>4) + 1;	// PRDT entries count
 
	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)phystovirt(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
 
	// 8K bytes (16 sectors) per PRDT
	for (int i = 0; i < cmdheader->prdtl-1; i++)	
	{
		cmdtbl->prdt_entry[i].dba = (DWORD)virttophys(buf);
		cmdtbl->prdt_entry[i].dbau = 0;
		cmdtbl->prdt_entry[i].dbc = 1<<13; // 8K bytes
		cmdtbl->prdt_entry[i].i = 1;
		buf += 1<<13; // 4K words
		count -= 16; // 16 sectors
	}

	// Last entry
	cmdtbl->prdt_entry[cmdheader->prdtl-1].dba = (DWORD)virttophys(buf);
	cmdtbl->prdt_entry[cmdheader->prdtl-1].dbau = 0;
	cmdtbl->prdt_entry[cmdheader->prdtl-1].dbc = count<<9;	// 512 bytes per sector
	cmdtbl->prdt_entry[cmdheader->prdtl-1].i = 1;
 
	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)phystovirt(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1; // Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
 
	cmdfis->lba0 = (BYTE)start;
	cmdfis->lba1 = (BYTE)(start>>8);
	cmdfis->lba2 = (BYTE)(start>>16);
	cmdfis->device = 1<<6; // LBA mode
 
	cmdfis->lba3 = (BYTE)(start>>24);
	cmdfis->lba4 = (BYTE)(start>>32);
	cmdfis->lba5 = (BYTE)(start>>40);
 
    cmdfis->countl = (BYTE)count;
	cmdfis->counth = (BYTE)(count>>8);
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		trace_ahci("Port is hung");
		return FALSE;
	}
 
	port->ci = 1<<cmdslot; // Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<cmdslot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			trace_ahci("Read disk error");
			return FALSE;
		}
	}
 
	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		trace_ahci("Read disk error");
		return FALSE;
	}
 
	return TRUE;
}
