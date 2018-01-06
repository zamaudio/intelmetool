/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 * Copyright (C) 2014 Damien Zammit <damien@zamaudio.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "me.h"

//#define read32(addr, off) ( *((__u32 *) (addr + off)) )
//#define write32(addr, off, val) ( *((__u32 *) (addr + off)) = val)

// FIXnotme: define this for old firmware only
//#define OLDARC 1

/* Path that the BIOS should take based on ME state */
/*
static const char *me_bios_path_values[] = {
	[ME_NORMAL_BIOS_PATH]		= "Normal",
	[ME_S3WAKE_BIOS_PATH]		= "S3 Wake",
	[ME_ERROR_BIOS_PATH]		= "Error",
	[ME_RECOVERY_BIOS_PATH]		= "Recovery",
	[ME_DISABLE_BIOS_PATH]		= "Disable",
	[ME_FIRMWARE_UPDATE_BIOS_PATH]	= "Firmware Update",
};
*/

#if 1
static void mei_dump(void *ptr, int dword, int offset, const char *type)
{
//	struct mei_csr *csr;


	switch (offset) {
	case MEI_H_CSR:
	case MEI_ME_CSR_HA:
/*		csr = ptr;
		if (!csr) {
			printk(KERN_INFO "notme: %-9s[%02x] : ERROR: 0x%08x\n", type, offset, dword);
			break;
		}
		printk(KERN_INFO "notme: %-9s[%02x] : depth=%u read=%02u write=%02u ready=%u "
		       "reset=%u intgen=%u intstatus=%u intenable=%u\n", type, offset, csr->buffer_depth,
		       csr->buffer_read_ptr, csr->buffer_write_ptr,
		       csr->ready, csr->reset, csr->interrupt_generate,
		       csr->interrupt_status, csr->interrupt_enable);
*/		break;
	case MEI_ME_CB_RW:
	case MEI_H_CB_WW:
		printk(KERN_INFO "notme: %-9s[%02x] : CB: 0x%08x\n", type, offset, dword);
		break;
	default:
		printk(KERN_INFO "notme: %-9s[%02x] : 0x%08x\n", type, offset, dword);
		break;
	}
}
#else
# define mei_dump(ptr,dword,offset,type) do {} while (0)
#endif

/*
 * ME/MEI access helpers using memcpy to avoid aliasing.
 */

static inline void pci_write_dword_ptr(struct pci_dev *dev, void *ptr, __u32 offset)
{
	__u32 dword = 0;
	memcpy(&dword, ptr, sizeof(dword));
	pci_write_config_dword(dev, offset, dword);
	mei_dump(ptr, dword, offset, "PCI WRITE");
}

static inline void pci_read_dword_ptr(struct pci_dev *dev, void *ptr, __u32 offset)
{
	__u32 dword;
	pci_read_config_dword(dev, offset, &dword);
	memcpy(ptr, &dword, sizeof(dword));
	mei_dump(ptr, dword, offset, "PCI READ");
}

static inline void read_host_csr(struct pci_dev *dev, struct mei_csr *csr)
{
	pci_read_dword_ptr(dev, csr, MEI_H_CSR);
}

static inline void write_host_csr(struct pci_dev *dev, struct mei_csr *csr)
{
	pci_write_dword_ptr(dev, csr, MEI_H_CSR);
}

static inline void read_me_csr(struct pci_dev *dev, struct mei_csr *csr)
{
	pci_read_dword_ptr(dev, csr, MEI_ME_CSR_HA);
}

static inline void write_cb(struct pci_dev *dev, __u32 dword)
{
	pci_write_dword_ptr(dev, &dword, MEI_H_CB_WW);
	mei_dump(NULL, dword, MEI_H_CB_WW, "PCI WRITE");
}

static inline __u32 read_cb(struct pci_dev *dev)
{
	__u32 dword;
	pci_read_dword_ptr(dev, &dword, MEI_ME_CB_RW);
	mei_dump(NULL, dword, MEI_ME_CB_RW, "PCI READ");
	return dword;
}

/* Wait for ME ready bit to be asserted */
static int mei_wait_for_me_ready(struct pci_dev *dev)
{
	struct mei_csr me;
	unsigned try = ME_RETRY;

	while (try--) {
		read_me_csr(dev, &me);
		if (me.ready)
			return 0;
		udelay(ME_DELAY);
	}

	printk(KERN_INFO "notme: failed to become ready\n");
	return -1;
}

void mei_reset(struct pci_dev *dev)
{
	struct mei_csr host;

	if (mei_wait_for_me_ready(dev) < 0)
		return;

	/* Reset host and ME circular buffers for next message */
	read_host_csr(dev, &host);
	host.reset = 1;
	host.interrupt_generate = 1;
	write_host_csr(dev, &host);

	if (mei_wait_for_me_ready(dev) < 0)
		return;

	/* Re-init and indicate host is ready */
	read_host_csr(dev, &host);
	host.interrupt_generate = 1;
	host.ready = 1;
	host.reset = 0;
	write_host_csr(dev, &host);
}

static int mei_send_msg(struct pci_dev *dev, struct mei_header *mei, struct mkhi_header *mkhi,
			void *req_data)
{
	struct mei_csr host;
	unsigned ndata, n;
	__u32 *data;

	/* Number of dwords to write, ignoring MKHI */
	ndata = (mei->length) >> 2;

	/* Pad non-dword aligned request message length */
	if (mei->length & 3)
		ndata++;
	if (!ndata) {
		printk(KERN_INFO "notme: request does not include MKHI\n");
		return -1;
	}
	ndata++; /* Add MEI header */

	/*
	 * Make sure there is still room left in the circular buffer.
	 * Reset the buffer pointers if the requested message will not fit.
	 */
	read_host_csr(dev, &host);
	if ((host.buffer_depth - host.buffer_write_ptr) < ndata) {
		printk(KERN_INFO "notme: circular buffer full, resetting...\n");
		mei_reset(dev);
		read_host_csr(dev, &host);
	}

	/*
	 * This implementation does not handle splitting large messages
	 * across multiple transactions.  Ensure the requested length
	 * will fit in the available circular buffer depth.
	 */
	if ((host.buffer_depth - host.buffer_write_ptr) < ndata) {
		printk(KERN_INFO "notme: message (%u) too large for buffer (%u)\n",
		       ndata + 2, host.buffer_depth);
		return -1;
	}

	/* Write MEI header */
	pci_write_dword_ptr(dev, mei, MEI_H_CB_WW);
	ndata--;

	/* Write MKHI header */
	pci_write_dword_ptr(dev, mkhi, MEI_H_CB_WW);
	ndata--;

	/* Write message data */
	data = req_data;
	for (n = 0; n < ndata; ++n)
		write_cb(dev, *data++);

	/* Generate interrupt to the ME */
	read_host_csr(dev, &host);
	host.interrupt_generate = 1;
	write_host_csr(dev, &host);

	/* Make sure ME is ready after sending request data */
	return mei_wait_for_me_ready(dev);
}

static int mei_recv_msg(struct pci_dev *dev, struct mei_header *mei, struct mkhi_header *mkhi,
			void *rsp_data, __u32 rsp_bytes)
{
	struct mei_header mei_rsp;
	struct mkhi_header mkhi_rsp;
	struct mei_csr me, host;
	unsigned ndata, n;
	unsigned expected;
	__u32 *data;

	/* Total number of dwords to read from circular buffer */
	expected = (rsp_bytes + sizeof(mei_rsp) + sizeof(mkhi_rsp)) >> 2;
	if (rsp_bytes & 3)
		expected++;

	//printk(KERN_INFO "expected u32 = %d\n", expected);
	/*
	 * The interrupt status bit does not appear to indicate that the
	 * message has actually been received.  Instead we wait until the
	 * expected number of dwords are present in the circular buffer.
	 */
	for (n = ME_RETRY; n; --n) {
		read_me_csr(dev, &me);
		if ((me.buffer_write_ptr - me.buffer_read_ptr) >= expected)
		//if (me.interrupt_generate && !me.interrupt_status)
		//if (me.interrupt_status)
			break;
		udelay(ME_DELAY);
	}
	if (!n) {
		printk(KERN_INFO "notme: timeout waiting for data: expected "
		       "%u, available %u\n", expected,
		       me.buffer_write_ptr - me.buffer_read_ptr);
		return -1;
	}
	/* Read and verify MEI response header from the ME */
	pci_read_dword_ptr(dev, &mei_rsp, MEI_ME_CB_RW);
	if (!mei_rsp.is_complete) {
		printk(KERN_INFO "notme: response is not complete\n");
		return -1;
	}

	/* Handle non-dword responses and expect at least MKHI header */
	ndata = mei_rsp.length >> 2;
	if (mei_rsp.length & 3)
		ndata++;
	if (ndata != (expected - 1)) {  //XXX
		printk(KERN_INFO "notme: response is missing data\n");
		//return -1;
	}

	/* Read and verify MKHI response header from the ME */
	pci_read_dword_ptr(dev, &mkhi_rsp, MEI_ME_CB_RW);
	if (!mkhi_rsp.is_response ||
	    mkhi->group_id != mkhi_rsp.group_id ||
	    mkhi->command != mkhi_rsp.command) {
		printk(KERN_INFO "notme: invalid response, group %u ?= %u, "
		       "command %u ?= %u, is_response %u\n", mkhi->group_id,
		       mkhi_rsp.group_id, mkhi->command, mkhi_rsp.command,
		       mkhi_rsp.is_response);
		//return -1;
	}
	ndata--; /* MKHI header has been read */

	/* Make sure caller passed a buffer with enough space */
	if (ndata != (rsp_bytes >> 2)) {
		printk(KERN_INFO "notme: not enough room in response buffer: "
		       "%u != %u\n", ndata, rsp_bytes >> 2);
		//return -1;
	}

	/* Read response data from the circular buffer */
	data = rsp_data;
	for (n = 0; n < ndata; ++n)
		*data++ = read_cb(dev);

	/* Tell the ME that we have consumed the response */
	read_host_csr(dev, &host);
	host.interrupt_status = 1;
	host.interrupt_generate = 1;
	write_host_csr(dev, &host);

	return mei_wait_for_me_ready(dev);
}

static inline int mei_sendrecv(struct pci_dev *dev, struct mei_header *mei, struct mkhi_header *mkhi,
			       void *req_data, void *rsp_data, __u32 rsp_bytes)
{
	if (mei_send_msg(dev, mei, mkhi, req_data) < 0)
		return -1;
	if (mei_recv_msg(dev, mei, mkhi, rsp_data, rsp_bytes) < 0)
		return -1;
	return 0;
}

/* Send END OF POST message to the ME */
/*
static int mkhi_end_of_post(void)
{
	struct mkhi_header mkhi = {
		.group_id	= MKHI_GROUP_ID_GEN,
		.command	= MKHI_END_OF_POST,
	};
	struct mei_header mei = {
		.is_complete	= 1,
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_MKHI,
		.length		= sizeof(mkhi),
	};

	if (mei_sendrecv(&mei, &mkhi, NULL, NULL, 0) < 0) {
		printk(KERN_INFO "notme: END OF POST message failed\n");
		return -1;
	}

	printk(KERN_INFO "notme: END OF POST message successful\n");
	return 0;
}
*/

/* Get ME firmware version */
int mkhi_get_fw_version(struct pci_dev *dev)
{
	__u32 data = 0;
	struct me_fw_version version = {0};
	
	struct mkhi_header mkhi = {
		.group_id	= MKHI_GROUP_ID_GEN,
		.command	= GEN_GET_FW_VERSION,
		.is_response 	= 0,
	};

	struct mei_header mei = {
		.is_complete	= 1,
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_MKHI,
		.length		= sizeof(mkhi),
	};

#ifndef OLDARC
	/* Send request and wait for response */
	if (mei_sendrecv(dev, &mei, &mkhi, &data, &version, sizeof(version) ) < 0) {
		printk(KERN_INFO "notme: GET FW VERSION message failed\n");
		return -1;
	}
	printk(KERN_INFO "notme: Firmware Version %u.%u.%u.%u (code) "
	       "%u.%u.%u.%u (recovery) "
	       "%u.%u.%u.%u (fitc)\n",
	       version.code_major, version.code_minor,
	       version.code_build_number, version.code_hot_fix,
	       version.recovery_major, version.recovery_minor, 
	       version.recovery_build_number, version.recovery_hot_fix,
	       version.fitcmajor, version.fitcminor, 
	       version.fitcbuildno, version.fitchotfix);
#else
	/* Send request and wait for response */
	if (mei_sendrecv(dev, &mei, &mkhi, &data, &version, 2*sizeof(__u32) ) < 0) {
		printk(KERN_INFO "notme: GET FW VERSION message failed\n");
		return -1;
	}
	printk(KERN_INFO "notme: Firmware Version %u.%u (code)\n"
	       version.code_major, version.code_minor);
#endif
	return 0;
}

static inline void print_cap(const char *name, int state)
{
	printk(KERN_INFO "ME Capability: %-30s : %s\n",
	       name, state ? "ON" : "OFF");
}

/* Get ME Firmware Capabilities */
int mkhi_get_fwcaps(struct pci_dev *dev)
{
	struct {
		__u32 rule_id;
		__u32 rule_len;
		struct me_fwcaps cap;
	} fwcaps;

	struct mkhi_header mkhi;

	struct mei_header mei = {
		.is_complete	= 1,
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_MKHI,
		.length		= sizeof(mkhi) + sizeof(fwcaps.rule_id),
	};

	fwcaps.rule_id = 0;
	fwcaps.rule_len = 0;
	
	mkhi.group_id	= MKHI_GROUP_ID_FWCAPS;
	mkhi.command	= MKHI_FWCAPS_GET_RULE;
	mkhi.is_response = 0;

	/* Send request and wait for response */
	if (mei_sendrecv(dev, &mei, &mkhi, &fwcaps.rule_id, &fwcaps.cap, sizeof(fwcaps.cap)) < 0) {
		printk(KERN_INFO "notme: GET FWCAPS message failed\n");
		return -1;
	}

	print_cap("Full Network manageability                ", fwcaps.cap.caps_sku.full_net);
	print_cap("Regular Network manageability             ", fwcaps.cap.caps_sku.std_net);
	print_cap("Manageability                             ", fwcaps.cap.caps_sku.manageability);
	print_cap("Small business technology                 ", fwcaps.cap.caps_sku.small_business);
	print_cap("Level III manageability                   ", fwcaps.cap.caps_sku.l3manageability);
	print_cap("IntelR Anti-Theft (AT)                    ", fwcaps.cap.caps_sku.intel_at);
	print_cap("IntelR Capability Licensing Service (CLS) ",
		  fwcaps.cap.caps_sku.intel_cls);
	print_cap("IntelR Power Sharing Technology (MPC)     ",
		  fwcaps.cap.caps_sku.intel_mpc);
	print_cap("ICC Over Clocking                         ", fwcaps.cap.caps_sku.icc_over_clocking);
        print_cap("Protected Audio Video Path (PAVP)         ", fwcaps.cap.caps_sku.pavp);
	print_cap("IPV6                                      ", fwcaps.cap.caps_sku.ipv6);
	print_cap("KVM Remote Control (KVM)                  ", fwcaps.cap.caps_sku.kvm);
	print_cap("Outbreak Containment Heuristic (OCH)      ", fwcaps.cap.caps_sku.och);
	print_cap("Virtual LAN (VLAN)                        ", fwcaps.cap.caps_sku.vlan);
	print_cap("TLS                                       ", fwcaps.cap.caps_sku.tls);
	print_cap("Wireless LAN (WLAN)                       ", fwcaps.cap.caps_sku.wlan);

	return 0;
}

/* Tell ME to issue a global reset */
__u32 mkhi_global_reset(struct pci_dev *dev)
{
	struct me_global_reset reset = {
		.request_origin	= GLOBAL_RESET_BIOS_POST,
		.reset_type	= CBM_RR_GLOBAL_RESET,
	};
	struct mkhi_header mkhi = {
		.group_id	= MKHI_GROUP_ID_CBM,
		.command	= MKHI_GLOBAL_RESET,
	};
	struct mei_header mei = {
		.is_complete	= 1,
		.length		= sizeof(mkhi) + sizeof(reset),
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_MKHI,
	};

	printk(KERN_INFO "notme: Requesting global reset\n");

	/* Send request and wait for response */
	if (mei_sendrecv(dev, &mei, &mkhi, &reset, NULL, 0) < 0) {
		/* No response means reset will happen shortly... */
		asm("hlt");
	}

	/* If the ME responded it rejected the reset request */
	printk(KERN_INFO "notme: Global Reset failed\n");
	return -1;
}

/* Tell ME thermal reporting parameters */
/*
void mkhi_thermal(void)
{
	struct me_thermal_reporting thermal = {
		.polling_timeout = 2,
		.smbus_ec_msglen = 1,
		.smbus_ec_msgpec = 0,
		.dimmnumber = 4,
	};
	struct mkhi_header mkhi = {
		.group_id	= MKHI_GROUP_ID_CBM,
		.command	= MKHI_THERMAL_REPORTING,
	};
	struct mei_header mei = {
		.is_complete	= 1,
		.length		= sizeof(mkhi) + sizeof(thermal),
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_THERMAL,
	};

	printk(KERN_INFO "notme: Sending thermal reporting params\n");

	mei_sendrecv(&mei, &mkhi, &thermal, NULL, 0);
}
*/

/* Enable debug of internal ME memory */
int mkhi_debug_me_memory(struct pci_dev *dev, void *physaddr)
{
	__u32 data = 0;

	/* copy whole ME memory to a readable space */
	struct me_debug_mem memory = {
		.debug_phys = (uintptr_t)physaddr,  
		.debug_size = 0x1000000,
		.me_phys = 0x1000000,
		.me_size = 0x1000000,
	};
	struct mkhi_header mkhi = {
		.group_id	= MKHI_GROUP_ID_GEN,
		.command	= GEN_SET_DEBUG_MEM,
		.is_response	= 0,
	};
	struct mei_header mei = {
		.is_complete	= 1,
		.length		= sizeof(mkhi) + sizeof(memory),
		.host_address	= MEI_HOST_ADDRESS,
		.client_address	= MEI_ADDRESS_MKHI,
	};

	printk(KERN_INFO "notme: Debug memory to 0x%zx ...", (size_t)physaddr);
	if (mei_sendrecv(dev, &mei, &mkhi, &memory, &data, 0) < 0) {
		printk(KERN_INFO "failed\n");
		return -1;
	} else {
		printk(KERN_INFO "done\n");
	}
	return 0;
}

/* Prepare ME for MEI messages */
__u32 intel_mei_setup(struct pci_dev *dev)
{
	struct mei_csr host;
	__u32 reg32, err;
	__u8 myirq;

	err = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &myirq);
	if (err) {
		printk(KERN_INFO "notme: Cant read interrupt line #");
	} else {
		printk(KERN_INFO "notme: Disabling interrupt");
		disable_irq(myirq);
	}

	/* Ensure Memory and Bus Master bits are set */
	pci_read_config_dword(dev, PCI_COMMAND, &reg32);
	reg32 |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_dword(dev, PCI_COMMAND, reg32);

	/* Clean up status for next message */
	read_host_csr(dev, &host);
	host.interrupt_generate = 1;
	host.ready = 1;
	host.reset = 0;
	write_host_csr(dev, &host);

	return 0;
}

/* Read the Extend register hash of ME firmware */
int intel_me_extend_valid(struct pci_dev *dev)
{
	struct me_heres status;
	__u32 extend[8] = {0};
	int i, count = 0;

	pci_read_dword_ptr(dev, &status, PCI_ME_HERES);
	if (!status.extend_feature_present) {
		printk(KERN_INFO "notme: Extend Feature not present\n");
		return -1;
	}

	if (!status.extend_reg_valid) {
		printk(KERN_INFO "notme: Extend Register not valid\n");
		return -1;
	}

	switch (status.extend_reg_algorithm) {
	case PCI_ME_EXT_SHA1:
		count = 5;
		printk(KERN_INFO "notme: Extend SHA-1: ");
		break;
	case PCI_ME_EXT_SHA256:
		count = 8;
		printk(KERN_INFO "notme: Extend SHA-256: ");
		break;
	default:
		printk(KERN_INFO "notme: Extend Algorithm %d unknown\n",
		       status.extend_reg_algorithm);
		return -1;
	}

	for (i = 0; i < count; ++i) {
		pci_read_config_dword(dev, PCI_ME_HER(i), &extend[i]);
		printk(KERN_INFO "%08x", extend[i]);
	}
	printk(KERN_INFO "\n");

	return 0;
}
