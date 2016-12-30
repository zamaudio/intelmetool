/*
 * intelmetool
 *
 * Copyright (C) 2008-2010 by coresystems GmbH
 * Copyright (C) 2009 Carl-Daniel Hailfinger
 * Copyright (C) 2015 Damien Zammit
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define ME_NOT_PRESENT 0
#define ME_FOUND_NOTHING 1
#define ME_FOUND_SOMETHING_NOT_SURE 2
#define ME_CAN_DISABLE_IF_PRESENT 3
#define ME_PRESENT_CAN_DISABLE 4
#define ME_PRESENT_CANNOT_DISABLE 5

// If any of the following pci device IDs are found,
// the heading they fall under is the corresponding result.

// Chipset does not have ME
#define PCI_DEVICE_ID_INTEL_82810		0x7120
#define PCI_DEVICE_ID_INTEL_82810_DC		0x7122
#define PCI_DEVICE_ID_INTEL_82810E_DC		0x7124
#define PCI_DEVICE_ID_INTEL_82830M		0x3575
#define PCI_DEVICE_ID_INTEL_82845		0x1a30
#define PCI_DEVICE_ID_INTEL_82865		0x2570
#define PCI_DEVICE_ID_INTEL_82915		0x2580
#define PCI_DEVICE_ID_INTEL_82945P		0x2770
#define PCI_DEVICE_ID_INTEL_82945GM		0x27a0
#define PCI_DEVICE_ID_INTEL_82945GSE		0x27ac
#define PCI_DEVICE_ID_INTEL_82X58		0x3405
#define PCI_DEVICE_ID_INTEL_ATOM_DXXX		0xa000
#define PCI_DEVICE_ID_INTEL_I63XX		0x2670
#define PCI_DEVICE_ID_INTEL_I5000X		0x25c0
#define PCI_DEVICE_ID_INTEL_I5000Z		0x25d0
#define PCI_DEVICE_ID_INTEL_I5000V		0x25d4
#define PCI_DEVICE_ID_INTEL_I5000P		0x25d8
#define PCI_DEVICE_ID_INTEL_82443LX		0x7180
#define PCI_DEVICE_ID_INTEL_82443BX		0x7190
#define PCI_DEVICE_ID_INTEL_82443BX_NO_AGP	0x7192
#define PCI_DEVICE_ID_INTEL_82371XX		0x7110
#define PCI_DEVICE_ID_INTEL_ICH			0x2410
#define PCI_DEVICE_ID_INTEL_ICH0		0x2420
#define PCI_DEVICE_ID_INTEL_ICH2		0x2440
#define PCI_DEVICE_ID_INTEL_ICH4		0x24c0
#define PCI_DEVICE_ID_INTEL_ICH4M		0x24cc
#define PCI_DEVICE_ID_INTEL_ICH5		0x24d0
#define PCI_DEVICE_ID_INTEL_ICH6		0x2640
#define PCI_DEVICE_ID_INTEL_ICH7DH		0x27b0
#define PCI_DEVICE_ID_INTEL_ICH7		0x27b8
#define PCI_DEVICE_ID_INTEL_ICH7M		0x27b9
#define PCI_DEVICE_ID_INTEL_ICH7MDH		0x27bd
#define PCI_DEVICE_ID_INTEL_NM10		0x27bc

#define PCI_DEV_NO_ME(x) ( \
	( (x) == PCI_DEVICE_ID_INTEL_82810	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82810_DC	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82810E_DC	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82830M	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82845	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82865	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82915	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82945P	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82945GM	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82945GSE	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82X58	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ATOM_DXXX	) || \
	( (x) == PCI_DEVICE_ID_INTEL_I63XX	) || \
	( (x) == PCI_DEVICE_ID_INTEL_I5000X	) || \
	( (x) == PCI_DEVICE_ID_INTEL_I5000Z	) || \
	( (x) == PCI_DEVICE_ID_INTEL_I5000V	) || \
	( (x) == PCI_DEVICE_ID_INTEL_I5000P	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82443LX	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82443BX	) || \
	( (x) == PCI_DEVICE_ID_INTEL_82443BX_NO_AGP) || \
	( (x) == PCI_DEVICE_ID_INTEL_82371XX	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH0	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH2	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH4	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH4M	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH5	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH6	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH7DH	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH7	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH7M	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH7MDH	) || \
	( (x) == PCI_DEVICE_ID_INTEL_NM10	))
			

// Definitely has ME and can be disabled
#define PCI_DEVICE_ID_INTEL_ICH8ME		0x2811
#define PCI_DEVICE_ID_INTEL_ICH9ME		0x2917
#define PCI_DEVICE_ID_INTEL_ICH9M		0x2919

#define PCI_DEV_HAS_ME_DISABLE(x) ( \
	( (x) == PCI_DEVICE_ID_INTEL_ICH8ME ) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9ME ) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9M  ))

// Definitely has ME and is very difficult to remove
#define PCI_DEVICE_ID_INTEL_ICH10R		0x3a16
#define PCI_DEVICE_ID_INTEL_3400_DESKTOP	0x3b00
#define PCI_DEVICE_ID_INTEL_3400_MOBILE		0x3b01
#define PCI_DEVICE_ID_INTEL_P55			0x3b02
#define PCI_DEVICE_ID_INTEL_PM55		0x3b03
#define PCI_DEVICE_ID_INTEL_H55			0x3b06
#define PCI_DEVICE_ID_INTEL_QM57		0x3b07
#define PCI_DEVICE_ID_INTEL_H57			0x3b08
#define PCI_DEVICE_ID_INTEL_HM55		0x3b09
#define PCI_DEVICE_ID_INTEL_Q57			0x3b0a
#define PCI_DEVICE_ID_INTEL_HM57		0x3b0b
#define PCI_DEVICE_ID_INTEL_3400_MOBILE_SFF	0x3b0d
#define PCI_DEVICE_ID_INTEL_B55_A		0x3b0e
#define PCI_DEVICE_ID_INTEL_QS57		0x3b0f
#define PCI_DEVICE_ID_INTEL_3400		0x3b12
#define PCI_DEVICE_ID_INTEL_3420		0x3b14
#define PCI_DEVICE_ID_INTEL_3450		0x3b16
#define PCI_DEVICE_ID_INTEL_B55_B		0x3b1e
#define PCI_DEVICE_ID_INTEL_Z68			0x1c44
#define PCI_DEVICE_ID_INTEL_P67			0x1c46
#define PCI_DEVICE_ID_INTEL_UM67		0x1c47
#define PCI_DEVICE_ID_INTEL_HM65		0x1c49
#define PCI_DEVICE_ID_INTEL_H67			0x1c4a
#define PCI_DEVICE_ID_INTEL_HM67		0x1c4b
#define PCI_DEVICE_ID_INTEL_Q65			0x1c4c
#define PCI_DEVICE_ID_INTEL_QS67		0x1c4d
#define PCI_DEVICE_ID_INTEL_Q67			0x1c4e
#define PCI_DEVICE_ID_INTEL_QM67		0x1c4f
#define PCI_DEVICE_ID_INTEL_B65			0x1c50
#define PCI_DEVICE_ID_INTEL_C202		0x1c52
#define PCI_DEVICE_ID_INTEL_C204		0x1c54
#define PCI_DEVICE_ID_INTEL_C206		0x1c56
#define PCI_DEVICE_ID_INTEL_H61			0x1c5c
#define PCI_DEVICE_ID_INTEL_Z77			0x1e44
#define PCI_DEVICE_ID_INTEL_Z75			0x1e46
#define PCI_DEVICE_ID_INTEL_Q77			0x1e47
#define PCI_DEVICE_ID_INTEL_Q75			0x1e48
#define PCI_DEVICE_ID_INTEL_B75			0x1e49
#define PCI_DEVICE_ID_INTEL_H77			0x1e4a
#define PCI_DEVICE_ID_INTEL_C216		0x1e53
#define PCI_DEVICE_ID_INTEL_QM77		0x1e55
#define PCI_DEVICE_ID_INTEL_QS77		0x1e56
#define PCI_DEVICE_ID_INTEL_HM77		0x1e57
#define PCI_DEVICE_ID_INTEL_UM77		0x1e58
#define PCI_DEVICE_ID_INTEL_HM76		0x1e59
#define PCI_DEVICE_ID_INTEL_HM75		0x1e5d
#define PCI_DEVICE_ID_INTEL_HM70		0x1e5e
#define PCI_DEVICE_ID_INTEL_NM70		0x1e5f
#define PCI_DEVICE_ID_INTEL_DH89XXCC		0x2310
#define PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_FULL	0x9c41
#define PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_PREM	0x9c43
#define PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_BASE	0x9c45

#define PCI_DEV_HAS_ME_DIFFICULT(x) ( \
	( (x) == PCI_DEVICE_ID_INTEL_ICH10R ) || \
	( (x) == PCI_DEVICE_ID_INTEL_3400_DESKTOP ) || \
	( (x) == PCI_DEVICE_ID_INTEL_3400_MOBILE ) || \
	( (x) == PCI_DEVICE_ID_INTEL_P55	) || \
	( (x) == PCI_DEVICE_ID_INTEL_PM55	) || \
	( (x) == PCI_DEVICE_ID_INTEL_H55	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QM57	) || \
	( (x) == PCI_DEVICE_ID_INTEL_H57	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM55	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Q57	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM57	) || \
	( (x) == PCI_DEVICE_ID_INTEL_3400_MOBILE_SFF ) || \
	( (x) == PCI_DEVICE_ID_INTEL_B55_A	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QS57	) || \
	( (x) == PCI_DEVICE_ID_INTEL_3400	) || \
	( (x) == PCI_DEVICE_ID_INTEL_3420	) || \
	( (x) == PCI_DEVICE_ID_INTEL_3450	) || \
	( (x) == PCI_DEVICE_ID_INTEL_B55_B	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Z68	) || \
	( (x) == PCI_DEVICE_ID_INTEL_P67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_UM67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM65	) || \
	( (x) == PCI_DEVICE_ID_INTEL_H67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Q65	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QS67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Q67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QM67	) || \
	( (x) == PCI_DEVICE_ID_INTEL_B65	) || \
	( (x) == PCI_DEVICE_ID_INTEL_C202	) || \
	( (x) == PCI_DEVICE_ID_INTEL_C204	) || \
	( (x) == PCI_DEVICE_ID_INTEL_C206	) || \
	( (x) == PCI_DEVICE_ID_INTEL_H61	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Z77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Z75	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Q77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_Q75	) || \
	( (x) == PCI_DEVICE_ID_INTEL_B75	) || \
	( (x) == PCI_DEVICE_ID_INTEL_H77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_C216	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QM77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_QS77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_UM77	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM76	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM75	) || \
	( (x) == PCI_DEVICE_ID_INTEL_HM70	) || \
	( (x) == PCI_DEVICE_ID_INTEL_NM70	) || \
	( (x) == PCI_DEVICE_ID_INTEL_DH89XXCC	) || \
	( (x) == PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_FULL ) || \
	( (x) == PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_PREM ) || \
	( (x) == PCI_DEVICE_ID_INTEL_LYNXPOINT_LP_BASE ))

// Not sure if ME present, but should be able to disable it easily
#define PCI_DEVICE_ID_INTEL_ICH8		0x2810
#define PCI_DEVICE_ID_INTEL_ICH8M		0x2815
#define PCI_DEVICE_ID_INTEL_ICH9DH		0x2912
#define PCI_DEVICE_ID_INTEL_ICH9DO		0x2914
#define PCI_DEVICE_ID_INTEL_ICH9R		0x2916
#define PCI_DEVICE_ID_INTEL_ICH9		0x2918
#define PCI_DEVICE_ID_INTEL_SUNRISE		0x9d3a

#define PCI_DEV_CAN_DISABLE_ME_IF_PRESENT(x) ( \
	( (x) == PCI_DEVICE_ID_INTEL_ICH8	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH8M	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9DH	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9DO	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9R	) || \
	( (x) == PCI_DEVICE_ID_INTEL_SUNRISE	) || \
	( (x) == PCI_DEVICE_ID_INTEL_ICH9	))

// Not sure at all
#define PCI_DEVICE_ID_INTEL_SCH_POULSBO_LPC	0x8119
#define PCI_DEVICE_ID_INTEL_SCH_POULSBO		0x8100

#define PCI_DEV_ME_NOT_SURE(x) ( \
	( (x) == PCI_DEVICE_ID_INTEL_SCH_POULSBO_LPC ) || \
	( (x) == PCI_DEVICE_ID_INTEL_SCH_POULSBO))
