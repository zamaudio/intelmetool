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

#ifndef ME_H
#define ME_H

#include <linux/kernel.h>
#include <linux/pci.h>
#include <asm/types.h>
#include <asm/io.h>
#include <asm/page.h>

#define ME_RETRY                100000  /* 1 second */
#define ME_DELAY                10      /* 10 us */

#pragma pack(1)

/*
 * Management Engine PCI registers
 */

#define PCI_ME_HFS		0x40
#define  ME_HFS_CWS_RESET	0
#define  ME_HFS_CWS_INIT	1
#define  ME_HFS_CWS_REC		2
#define  ME_HFS_CWS_NORMAL	5
#define  ME_HFS_CWS_WAIT	6
#define  ME_HFS_CWS_TRANS	7
#define  ME_HFS_CWS_INVALID	8
#define  ME_HFS_STATE_PREBOOT	0
#define  ME_HFS_STATE_M0_UMA	1
#define  ME_HFS_STATE_M3	4
#define  ME_HFS_STATE_M0	5
#define  ME_HFS_STATE_BRINGUP	6
#define  ME_HFS_STATE_ERROR	7
#define  ME_HFS_ERROR_NONE	0
#define  ME_HFS_ERROR_UNCAT	1
#define  ME_HFS_ERROR_IMAGE	3
#define  ME_HFS_ERROR_DEBUG	4
#define  ME_HFS_MODE_NORMAL	0
#define  ME_HFS_MODE_DEBUG	2
#define  ME_HFS_MODE_DIS	3
#define  ME_HFS_MODE_OVER_JMPR	4
#define  ME_HFS_MODE_OVER_MEI	5
#define  ME_HFS_BIOS_DRAM_ACK	1
#define  ME_HFS_ACK_NO_DID	0
#define  ME_HFS_ACK_RESET	1
#define  ME_HFS_ACK_PWR_CYCLE	2
#define  ME_HFS_ACK_S3		3
#define  ME_HFS_ACK_S4		4
#define  ME_HFS_ACK_S5		5
#define  ME_HFS_ACK_GBL_RESET	6
#define  ME_HFS_ACK_CONTINUE	7

struct me_hfs {
	__u32 working_state: 4;
	__u32 mfg_mode: 1;
	__u32 fpt_bad: 1;
	__u32 operation_state: 3;
	__u32 fw_init_complete: 1;
	__u32 ft_bup_ld_flr: 1;
	__u32 update_in_progress: 1;
	__u32 error_code: 4;
	__u32 operation_mode: 4;
	__u32 reserved: 4;
	__u32 boot_options_present: 1;
	__u32 ack_data: 3;
	__u32 bios_msg_ack: 4;
} __attribute__ ((packed));

#define PCI_ME_UMA		0x44

struct me_uma {
	__u32 size: 6;
	__u32 reserved_1: 10;
	__u32 valid: 1;
	__u32 reserved_0: 14;
	__u32 set_to_one: 1;
} __attribute__ ((packed));

#define PCI_ME_H_GS		0x4c
#define  ME_INIT_DONE		1
#define  ME_INIT_STATUS_SUCCESS	0
#define  ME_INIT_STATUS_NOMEM	1
#define  ME_INIT_STATUS_ERROR	2

struct me_did {
	__u32 uma_base: 16;
	__u32 reserved: 8;
	__u32 status: 4;
	__u32 init_done: 4;
} __attribute__ ((packed));

#define PCI_ME_GMES		0x48
#define  ME_GMES_PHASE_ROM	0
#define  ME_GMES_PHASE_BUP	1
#define  ME_GMES_PHASE_UKERNEL	2
#define  ME_GMES_PHASE_POLICY	3
#define  ME_GMES_PHASE_MODULE	4
#define  ME_GMES_PHASE_UNKNOWN	5
#define  ME_GMES_PHASE_HOST	6

struct me_gmes {
	__u32 bist_in_prog : 1;
	__u32 icc_prog_sts : 2;
	__u32 invoke_mebx : 1;
	__u32 cpu_replaced_sts : 1;
	__u32 mbp_rdy : 1;
	__u32 mfs_failure : 1;
	__u32 warm_rst_req_for_df : 1;
	__u32 cpu_replaced_valid : 1;
	__u32 reserved_1 : 2;
	__u32 fw_upd_ipu : 1;
	__u32 reserved_2 : 4;
	__u32 current_state: 8;
	__u32 current_pmevent: 4;
	__u32 progress_code: 4;
} __attribute__ ((packed));

#define PCI_ME_HERES		0xbc
#define  PCI_ME_EXT_SHA1	0x00
#define  PCI_ME_EXT_SHA256	0x02
#define PCI_ME_HER(x)		(0xc0+(4*(x)))

struct me_heres {
	__u32 extend_reg_algorithm: 4;
	__u32 reserved: 26;
	__u32 extend_feature_present: 1;
	__u32 extend_reg_valid: 1;
} __attribute__ ((packed));

struct me_thermal_reporting {
	__u32 polling_timeout: 8;
	__u32 smbus_ec_msglen: 8;
	__u32 smbus_ec_msgpec: 8;
	__u32 dimmnumber: 8;
} __attribute__ ((packed));

/*
 * Management Engine MEI registers
 */

#define MEI_H_CB_WW		0x00
#define MEI_H_CSR		0x04
#define MEI_ME_CB_RW		0x08
#define MEI_ME_CSR_HA		0x0c

struct mei_csr {
	__u32 interrupt_enable: 1;
	__u32 interrupt_status: 1;
	__u32 interrupt_generate: 1;
	__u32 ready: 1;
	__u32 reset: 1;
	__u32 reserved: 3;
	__u32 buffer_read_ptr: 8;
	__u32 buffer_write_ptr: 8;
	__u32 buffer_depth: 8;
} __attribute__ ((packed));

#define MEI_ADDRESS_HBM		0x00
#define MEI_ADDRESS_CORE_WD	0x01
#define MEI_ADDRESS_AMT		0x02
#define MEI_ADDRESS_RESERVED	0x03
#define MEI_ADDRESS_WDT		0x04
#define MEI_ADDRESS_POLICY	0x05
#define MEI_ADDRESS_PASSWORD	0x06
#define MEI_ADDRESS_MKHI	0x07
#define MEI_ADDRESS_ICC		0x08
#define MEI_ADDRESS_THERMAL	0x09
#define MEI_ADDRESS_SPI		0x0a

#define MEI_HOST_ADDRESS	0

struct mei_header {
	__u32 client_address: 8;
	__u32 host_address: 8;
	__u32 length: 9;
	__u32 reserved: 6;
	__u32 is_complete: 1;
} __attribute__ ((packed));

#define MKHI_GROUP_ID_CBM	0x00
#define MKHI_GROUP_ID_PM	0x01
#define MKHI_GROUP_ID_PWD	0x02
#define MKHI_GROUP_ID_FWCAPS	0x03
#define MKHI_GROUP_ID_APP	0x04
#define MKHI_GROUP_ID_SPI	0x05
#define MKHI_GROUP_ID_MDES	0x08
#define MKHI_GROUP_ID_MAX	0x09
#define MKHI_GROUP_ID_GEN	0xff

#define MKHI_FWCAPS_GET_RULE	0x02
#define MKHI_FWCAPS_SET_RULE	0x03
#define MKHI_GLOBAL_RESET	0x0b

#define GEN_GET_MKHI_VERSION	0x01
#define GEN_GET_FW_VERSION	0x02
#define GEN_UNCONFIG_NO_PWD	0x0d
#define GEN_SET_DEBUG_MEM	0x11

#define FWCAPS_ME_FWU_RULE	0x2e
#define FWCAPS_OVERRIDE		0x14

#define MKHI_THERMAL_REPORTING  0x00
#define MKHI_GET_FW_VERSION	0x02
#define MKHI_MDES_ENABLE	0x09
#define MKHI_END_OF_POST	0x0c
#define MKHI_FEATURE_OVERRIDE	0x14

#define HBM_HOST_START_REQ_CMD                  0x01
#define HBM_HOST_STOP_REQ_CMD                   0x02
#define HBM_ME_STOP_REQ_CMD                     0x03
#define HBM_HOST_ENUM_REQ_CMD                   0x04
#define HBM_HOST_CLIENT_PROPERTIES_REQ_CMD      0x05
#define HBM_CLIENT_CONNECT_REQ_CMD              0x06
#define HBM_CLIENT_DISCONNECT_REQ_CMD           0x07

struct mkhi_header {
	__u32 group_id: 8;
	__u32 command: 7;
	__u32 is_response: 1;
	__u32 reserved: 8;
	__u32 result: 8;
} __attribute__ ((packed));

struct me_fw_version {
	__u16 code_minor;
	__u16 code_major;
	__u16 code_build_number;
	__u16 code_hot_fix;
	__u16 recovery_minor;
	__u16 recovery_major;
	__u16 recovery_build_number;
	__u16 recovery_hot_fix;
	__u16 fitcminor;
	__u16 fitcmajor;
	__u16 fitcbuildno;
	__u16 fitchotfix;
} __attribute__ ((packed));


#define HECI_EOP_STATUS_SUCCESS       0x0
#define HECI_EOP_PERFORM_GLOBAL_RESET 0x1

#define CBM_RR_GLOBAL_RESET	0x01

#define GLOBAL_RESET_BIOS_MRC	0x01
#define GLOBAL_RESET_BIOS_POST	0x02
#define GLOBAL_RESET_MEBX	0x03

struct me_global_reset {
	__u8 request_origin;
	__u8 reset_type;
} __attribute__ ((packed));

typedef enum {
	ME_NORMAL_BIOS_PATH,
	ME_S3WAKE_BIOS_PATH,
	ME_ERROR_BIOS_PATH,
	ME_RECOVERY_BIOS_PATH,
	ME_DISABLE_BIOS_PATH,
	ME_FIRMWARE_UPDATE_BIOS_PATH,
} me_bios_path;

typedef struct {
	__u32       major_version  : 16;
	__u32       minor_version  : 16;
	__u32       hotfix_version : 16;
	__u32       build_version  : 16;
} __attribute__ ((packed)) mbp_fw_version_name;

typedef struct {
	__u8        num_icc_profiles;
	__u8        icc_profile_soft_strap;
	__u8        icc_profile_index;
	__u8        reserved;
	__u32       register_lock_mask[3];
} __attribute__ ((packed)) mbp_icc_profile;

typedef struct {
	__u32  full_net		: 1;
	__u32  std_net		: 1;
	__u32  manageability	: 1;
	__u32  small_business	: 1;
	__u32  l3manageability	: 1;
	__u32  intel_at		: 1;
	__u32  intel_cls		: 1;
	__u32  reserved		: 3;
	__u32  intel_mpc		: 1;
	__u32  icc_over_clocking	: 1;
	__u32  pavp		: 1;
	__u32  reserved_1		: 4;
	__u32  ipv6		: 1;
	__u32  kvm		: 1;
	__u32  och		: 1;
	__u32  vlan		: 1;
	__u32  tls		: 1;
	__u32  reserved_4		: 1;
	__u32  wlan		: 1;
	__u32  reserved_5		: 8;
} __attribute__ ((packed)) mefwcaps_sku;

typedef struct {
	__u16  lock_state		     : 1;
	__u16  authenticate_module     : 1;
	__u16  s3authentication  	     : 1;
	__u16  flash_wear_out          : 1;
	__u16  flash_variable_security : 1;
	__u16  wwan3gpresent	     : 1;
	__u16  wwan3goob		     : 1;
	__u16  reserved		     : 9;
} __attribute__ ((packed)) tdt_state_flag;

typedef struct {
	__u8           state;
	__u8           last_theft_trigger;
	tdt_state_flag  flags;
}  __attribute__ ((packed)) tdt_state_info;

typedef struct {
	__u32  platform_target_usage_type	 : 4;
	__u32  platform_target_market_type : 2;
	__u32  super_sku			 : 1;
	__u32  reserved			 : 1;
	__u32  intel_me_fw_image_type	 : 4;
	__u32  platform_brand		 : 4;
	__u32  reserved_1			 : 16;
}  __attribute__ ((packed)) platform_type_rule_data;

typedef struct {
	mefwcaps_sku fw_capabilities;
	__u8      available;
} mbp_fw_caps;

typedef struct {
	__u16        device_id;
	__u16        fuse_test_flags;
	__u32        umchid[4];
}  __attribute__ ((packed)) mbp_rom_bist_data;

typedef struct {
	__u32        key[8];
} mbp_platform_key;

typedef struct {
	platform_type_rule_data rule_data;
	__u8	          available;
} mbp_plat_type;

typedef struct {
	mbp_fw_version_name fw_version_name;
	mbp_fw_caps	    fw_caps_sku;
	mbp_rom_bist_data   rom_bist_data;
	mbp_platform_key    platform_key;
	mbp_plat_type	    fw_plat_type;
	mbp_icc_profile	    icc_profile;
	tdt_state_info	    at_state;
	__u32		    mfsintegrity;
} me_bios_payload;

typedef  struct {
	__u32  mbp_size	 : 8;
	__u32  num_entries : 8;
	__u32  rsvd      	 : 16;
} __attribute__ ((packed)) mbp_header;

typedef struct {
	__u32  app_id  : 8;
	__u32  item_id : 8;
	__u32  length  : 8;
	__u32  rsvd    : 8;
}  __attribute__ ((packed)) mbp_item_header;

struct me_fwcaps {
	__u32 id;
	__u8 length;
	mefwcaps_sku caps_sku;
	__u8 reserved[3];
} __attribute__ ((packed));

struct me_debug_mem {
	__u32 debug_phys;
        __u32 debug_size;
        __u32 me_phys;
        __u32 me_size;
} __attribute__ ((packed));

void intel_me_status(struct pci_dev *dev, __u32 hfs, __u32 gmes);
void mkhi_thermal(struct pci_dev *dev);
__u32 intel_mei_setup(struct pci_dev *dev);
int mkhi_get_fwcaps(struct pci_dev *dev);
int mkhi_get_fw_version(struct pci_dev *dev);
int mkhi_debug_me_memory(struct pci_dev *dev, void *addr);
void mei_reset(struct pci_dev *dev);
int intel_me_extend_valid(struct pci_dev *dev);

#endif
