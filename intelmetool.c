/* intelmetool  Dump interesting things about Management Engine even if hidden
 * Copyright (C) 2014  Damien Zammit <damien@zamaudio.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License,
 * or (at your option), any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <inttypes.h>
#include <pci/pci.h>
#include <sys/io.h>
#include <stdlib.h>
#include "me.h"
#include "mmap.h"
#include "intelmetool.h"

extern int fd_mem;
#define FD2 0x3428

void dumpmem(uint8_t *phys, uint32_t size)
{
	uint32_t i;
	printf("Dumping cloned ME memory:\n");
	for (i = 0; i < size; i++) {
		printf("%02X",*((uint8_t *) (phys + i)));
	}
	printf("\n");
}

void zeroit(uint8_t *phys, uint32_t size)
{
	uint32_t i;
	for (i = 0; i < size; i++) {
		*((uint8_t *) (phys + i)) = 0x00;
	}
}

void dumpmemfile(uint8_t *phys, uint32_t size)
{
	FILE *fp = fopen("medump.bin", "w");
	uint32_t i;
	for (i = 0; i < size; i++) {
		fprintf(fp, "%c", *((uint8_t *) (phys + i)));
	}
	fclose(fp);
}

int main(void)
{
	struct pci_access *pacc;
	struct pci_dev *dev;
	struct pci_dev *sb;
	uint32_t stat, stat2;
	char namebuf[1024], *name;
	int size = 0x4000;
	if (iopl(3)) {
		perror("iopl");
		printf("You need to be root\n");
		exit(1);
	}

        if ((fd_mem = open("/dev/mem", O_RDWR)) < 0) {
                perror("Can not open /dev/mem");
                exit(1);
        }

	volatile uint8_t *rcba;
	uint32_t rcba_phys;
	uint32_t fd2;
	uint8_t me;

	pacc = pci_alloc();
	pacc->method = PCI_ACCESS_I386_TYPE1;
	pci_init(pacc);
	pci_scan_bus(pacc);
	me = ME_FOUND_NOTHING;
	for (dev=pacc->devices; dev; dev=dev->next) {
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_SIZES | PCI_FILL_CLASS);
		name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), 
			PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
		if (dev->vendor_id == 0x8086) {
			if (PCI_DEV_NO_ME(dev->device_id)) {
				me = ME_NOT_PRESENT;
				break;
			}
			if (PCI_DEV_HAS_ME_DISABLE(dev->device_id)) {
				me = ME_PRESENT_CAN_DISABLE;
				break;
			}
			if (PCI_DEV_HAS_ME_DIFFICULT(dev->device_id)) {
				me = ME_PRESENT_CANNOT_DISABLE;
				break;
			}
			if (PCI_DEV_CAN_DISABLE_ME_IF_PRESENT(dev->device_id)) {
				me = ME_CAN_DISABLE_IF_PRESENT;
				break;
			}
			if (PCI_DEV_ME_NOT_SURE(dev->device_id)) {
				me = ME_FOUND_SOMETHING_NOT_SURE;
				break;
			}
		}
	}

	switch (me) {
		case ME_FOUND_NOTHING:
			printf("Hardware unsupported by intelmetool, exiting\n");
			pci_cleanup(pacc);
			exit(ME_FOUND_NOTHING);
			break;
		case ME_FOUND_SOMETHING_NOT_SURE:
			printf("Found `%s`. Not sure whether you have ME hardware, exiting\n", name);
			pci_cleanup(pacc);
			exit(ME_FOUND_SOMETHING_NOT_SURE);
			break;
		case ME_NOT_PRESENT:
			printf("ME is not present on your board because we found a `%s`, you are safe, exiting\n", name);
			pci_cleanup(pacc);
			exit(ME_NOT_PRESENT);
			break;
		case ME_CAN_DISABLE_IF_PRESENT:
			printf("Not sure if ME hardware is present because you have a `%s`, but it is possible to disable it if you do, continuing...\n", name);
			break;
		case ME_PRESENT_CANNOT_DISABLE:
			printf("Bad news, you have a `%s` so you have ME hardware on board and it is very difficult to remove, continuing...\n", name);
			break;
		case ME_PRESENT_CAN_DISABLE:
			printf("Good news, you have a `%s` so ME is present but can be disabled, continuing...\n", name);
			break;
		default:
			printf("Something horrible happened, exiting\n");
			pci_cleanup(pacc);
			exit(1);
			break;
	}


	pacc = pci_alloc();
	pacc->method = PCI_ACCESS_I386_TYPE1;
	pci_init(pacc);
	pci_scan_bus(pacc);
	sb = pci_get_dev(pacc, 0, 0, 0x1f, 0);
	if (!sb) {
		printf("Uh oh, southbridge not on BDF(0:31:0), please report this error, exiting.\n");
		pci_cleanup(pacc);
		exit(1);
	}
	pci_fill_info(sb, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_SIZES | PCI_FILL_CLASS);
	
	/* Enable MEI */
	rcba_phys = pci_read_long(sb, 0xf0) & 0xfffffffe;
	rcba = map_physical(rcba_phys, size);
	printf("RCBA at 0x%08" PRIx32 "\n", (uint32_t)rcba_phys);
	fd2 = *(uint32_t *)(rcba + FD2);
	*(uint32_t *)(rcba + FD2) = fd2 & ~0x2;
	if (fd2 & 0x2) {
		printf("MEI was hidden on PCI, now unlocked\n");
	} else {
		printf("MEI not hidden on PCI, checking if visible\n");
	}
	
	pci_cleanup(pacc);
	
	pacc = pci_alloc();
	pacc->method = PCI_ACCESS_I386_TYPE1;
	pci_init(pacc);
	pci_scan_bus(pacc);
	me = 0;
	for (dev=pacc->devices; dev; dev=dev->next) {
		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_SIZES | PCI_FILL_CLASS);
		name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), 
			PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
		if (dev->vendor_id == 0x8086) {
			switch (dev->device_id) {
				case 0x1C3A:  /* Cougar Point */
				case 0x1CBA:  /* Panther Point */
				case 0x1D3A:  /* C600/X79 Patsburg */
				case 0x1DBA:  /* Panther Point */
				case 0x1E3A:  /* Panther Point */
				case 0x2364:  /* Cave Creek */
				case 0x28B4:  /* Bearlake */
				case 0x28C4:  /* Bearlake */
				case 0x28D4:  /* Bearlake */
				case 0x28E4:  /* Bearlake */
				case 0x28F4:  /* Bearlake */
				case 0x2974:  /* 82946GZ/GL */
				case 0x2984:  /* 82G35 Express */
				case 0x2994:  /* 82Q963/Q965 */
				case 0x29A4:  /* 82P965/G965 */
				case 0x29B4:  /* 82Q35 Express */
				case 0x29C4:  /* 82G33/G31/P35/P31 Express */
				case 0x29D4:  /* 82Q33 Express */
				case 0x29E4:  /* 82X38/X48 Express */
				case 0x29F4:  /* 3200/3210 Server */
				case 0x2A04:  /* Mobile PM965/GM965 */
				case 0x2A14:  /* Mobile GME965/GLE960 */
				case 0x2A44:  /* Cantiga */
				case 0x2a50:  /* Cantiga */
				case 0x2A54:  /* Cantiga */
				case 0x2A64:  /* Cantiga */
				case 0x2A74:  /* Cantiga */
				case 0x2E04:  /* Eaglelake */
				case 0x2E14:  /* Eaglelake */
				case 0x2E24:  /* Eaglelake */
				case 0x2E34:  /* Eaglelake */
				case 0x3B64:  /* Calpella */
				case 0x3B65:  /* Calpella */
				case 0x8C3A:  /* Lynx Point H */
				case 0x8CBA:  /* Lynx Point H Refresh */
				case 0x8D3A:  /* Lynx Point - Wellsburg */
				case 0x9C3A:  /* Lynx Point LP */
				case 0x9CBA:  /* Wildcat Point LP */
				case 0x9CBB:  /* Wildcat Point LP 2 */
				case 0x9D3A:  /* Sunrise Point-LP */
				case 0xA13A:  /* Sunrise Point-H #1 */
				case 0xA13B:  /* Sunrise Point-H #2 */
				case 0xA13E:  /* Sunrise Point-H #3 */
				case 0xA1BA:  /* CSME Lewisburg #1 */
				case 0xA1BB:  /* CSME Lewisburg #2 */
				case 0xA1BE:  /* CSME Lewisburg #3 */
				case 0xA1F8:  /* IE Lewisburg #1 */
				case 0xA1F9:  /* IE Lewisburg #2 */
				case 0xA1FC:  /* IE Lewisburg #3 */
					me = 1;
					break;
				default:
					me = 0;
					break;
			}
		}
		if (me) break;
	}
	if (me == 0) {
		printf("MEI device not found, huh?\n");
		if (fd2 & 0x2) {
			printf("Re-hiding MEI device...");
			fd2 = *(uint32_t *)(rcba + FD2);
			*(uint32_t *)(rcba + FD2) = fd2 | 0x2;
			printf("done\n");
		}
		printf ("exiting\n");
		pci_cleanup(pacc);
		munmap((void*)rcba, size);
		return 0;
	}

	printf("MEI found: [%x:%x] %s\n\n", dev->vendor_id, dev->device_id, name);
	stat = pci_read_long(dev, 0x40);
	printf("ME Status   : 0x%x\n", stat);
	stat2 = pci_read_long(dev, 0x48);
	printf("ME Status 2 : 0x%x\n\n", stat2);

	intel_me_status(stat, stat2);
	printf("\n");
	intel_me_extend_valid(dev);
	printf("\n");

	if ((stat & 0xf000) >> 12 == 0) {
		printf("ME seems okay on this board\n");
	} else {
		printf("ME has a broken implementation on your board with this BIOS\n");
	}

	intel_mei_setup(dev);
	udelay(1000);
	mei_reset();
	udelay(10000);
	mkhi_get_fw_version();
	udelay(10000);
	mei_reset();
	udelay(10000);
	mkhi_get_fwcaps();
	udelay(10000);
	
	/* You need >4GB total ram, in kernel cmdline, use 'mem=1000m'
	 * then this code will clone to absolute memory address 0xe0000000
	 * which can be read using a mmap tool at that offset.
	 * Real ME memory is located around top of memory minus 64MB. (I think)
	 * so we avoid cloning to this part.
	 */
/*
	uint32_t me_clone = 0x60000000;
	volatile uint8_t *dump;
	dump = map_physical_exact(me_clone, me_clone, 0x2000000);
	zeroit(dump, 0x2000000);
	printf("Send magic command for memory clone\n");

	mei_reset();
	udelay(10000);
	int err = mkhi_debug_me_memory(me_clone);

	if (!err) {
		printf("Wait a second...");
		udelay(30000);
		printf("done\n\nHere are the first bytes:\n");
		dumpmemfile(dump, 0x2000000);
		printf("Try reading 0x%zx with other mmap tool...\n"
			"Press enter to quit, you only get one chance to run this tool before reboot required for some reason\n", me_clone);
		while (getc(stdin) != '\n') {};
		unmap_physical(dump, 0x2000000);
	}
*/
	intel_mei_unmap();

	pci_cleanup(pacc);

	if (fd2 & 0x2) {
		printf("Re-hiding MEI device...");
		fd2 = *(uint32_t *)(rcba + FD2);
		*(uint32_t *)(rcba + FD2) = fd2 | 0x2;
		printf("done, ");
	}
	printf("exiting\n");
	munmap((void*)rcba, size);
	return 0;
}
