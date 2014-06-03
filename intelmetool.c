/* intelmetool  Dump interesting things about Management Engine even if hidden
 * Copyright (C) 2014  Damien Zammit <damien@zamaudio.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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

extern int fd_mem;
#define FD2 0x3428

void dumpmem(void *phys, uint32_t size)
{
	uint32_t i;
	printf("Dumping cloned ME memory:\n");
	for (i = 0; i < size; i++) {
		printf("%02X",*((uint8_t *) (phys + i)));
	}
	printf("\n");
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
	sb = pci_get_dev(pacc, 0, 0, 0x1f, 0);
	
	pci_fill_info(sb, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_SIZES | PCI_FILL_CLASS);
	
	name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), 
		PCI_LOOKUP_DEVICE, sb->vendor_id, sb->device_id);
	printf("Southbridge: %s\n", name);
	if ((sb->vendor_id == 0x8086) && (
		((sb->device_id & 0x00f0) == 0x0000) ||
		((sb->device_id & 0x00f0) == 0x0010) ||
		((sb->device_id & 0x00f0) == 0x0040) ||
		((sb->device_id & 0x00f0) == 0x0050))) {
		printf("Chipset supported and has ME\n");
	} else {
		printf("Chipset unsupported, exiting\n");
		pci_cleanup(pacc);
		exit(1);
	}
	/* Enable MEI */
	rcba_phys = pci_read_long(sb, 0xf0) & 0xfffffffe;
	rcba = map_physical(rcba_phys, size);
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
				case 0x1c3a:
				case 0x1d3a:
				case 0x1e3a:
				case 0x2364:
				case 0x29b4:
				case 0x29c4:
				case 0x29d4:
				case 0x29e4:
				case 0x29f4:
				case 0x2a04:
				case 0x2a14:
				case 0x2a44:
				case 0x2a50:
				case 0x8c3a:
				case 0x8d3a:
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
		printf("MEI device not found\n");
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

	mkhi_get_fw_version();
	
	mei_reset();
	mkhi_get_fwcaps();
	
	void *me_clone = malloc(0x2000000);
	if (me_clone != NULL) {
		printf("Send magic command for memory clone\n");
		
		mei_reset();
		int err = mkhi_debug_me_memory(me_clone);
		
		if (!err) {
			printf("Wait a second...");
			udelay(1000000);
			printf("done\n\n");
			dumpmem(me_clone, 0x1000);
		}
	}
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
	free(me_clone);
	return 0;
}
