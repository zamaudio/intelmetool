/* 
 * notme  Damagement Engine debug driver
 * Copyright (C) 2014-2018  Damien Zammit <damien@zamaudio.com>
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/pci.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include "me.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Damien Zammit");
MODULE_DESCRIPTION("notme Damagement Engine debug");

static struct pci_driver notme_driver;
//static int oldarc = 0;
//module_param(oldarc, int, 0000);
//MODULE_PARM_DESC(oldarc, "If set to 1, assume older 1.5MB Damagement Engine firmware");

static struct pci_device_id notme_ids[ ] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1A9A) },  /* Broxton M */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1C3A) },  /* Cougar Point */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1CBA) },  /* Panther Point */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1D3A) },  /* C600/X79 Patsburg */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1DBA) },  /* Panther Point */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x1E3A) },  /* Panther Point */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2364) },  /* Cave Creek */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x28B4) },  /* Bearlake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x28C4) },  /* Bearlake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x28D4) },  /* Bearlake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x28E4) },  /* Bearlake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x28F4) },  /* Bearlake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2974) },  /* 82946GZ/GL */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2984) },  /* 82G35 Express */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2994) },  /* 82Q963/Q965 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29A4) },  /* 82P965/G965 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29B4) },  /* 82Q35 Express */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29C4) },  /* 82G33/G31/P35/P31 Express */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29D4) },  /* 82Q33 Express */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29E4) },  /* 82X38/X48 Express */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29F4) },  /* 3200/3210 Server */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A04) },  /* Mobile PM965/GM965 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A14) },  /* Mobile GME965/GLE960 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A44) },  /* Cantiga */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A50) },  /* Cantiga */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A54) },  /* Cantiga */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A64) },  /* Cantiga */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2A74) },  /* Cantiga */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2E04) },  /* Eaglelake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2E14) },  /* Eaglelake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2E24) },  /* Eaglelake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2E34) },  /* Eaglelake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x319A) },  /* Gemini Lake */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x3B64) },  /* Calpella */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x3B65) },  /* Calpella */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x5A9A) },  /* Apollo Lake I */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x8C3A) },  /* Lynx Point H */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x8CBA) },  /* Lynx Point H Refresh */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x8D3A) },  /* Lynx Point - Wellsburg */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x9C3A) },  /* Lynx Point LP */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x9CBA) },  /* Wildcat Point LP */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x9CBB) },  /* Wildcat Point LP 2 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x9D3A) },  /* Sunrise Point #1 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x9D3B) },  /* Sunrise Point #2 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA13A) },  /* Sunrise Point-H #1 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA13B) },  /* Sunrise Point-H #2 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA13E) },  /* Sunrise Point-H #3 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1BA) },  /* CSME Lewisburg #1 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1BB) },  /* CSME Lewisburg #2 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1BE) },  /* CSME Lewisburg #3 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1F8) },  /* IE Lewisburg #1 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1F9) },  /* IE Lewisburg #2 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA1FC) },  /* IE Lewisburg #3 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA2BA) },  /* Kaby Point #1 */
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0xA2BB) },  /* Kaby Point #2 */
	{ 0, },
};


MODULE_DEVICE_TABLE(pci, notme_ids);

int notme_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
	int err;
	int stat, stat2;

	err = pci_enable_device(dev);
	if (!err) {
		printk(KERN_INFO "notme: probe: Found Damagement Engine\n");

		printk(KERN_INFO "notme: MEI found: [%x:%x]\n", dev->vendor, dev->device);
		pci_read_config_dword(dev, 0x40, &stat);
		printk(KERN_INFO "notme: ME Status   : 0x%x\n", stat);
		pci_read_config_dword(dev, 0x48, &stat2);
		printk(KERN_INFO "notme: ME Status 2 : 0x%x\n", stat2);

		intel_me_status(dev, stat, stat2);
		//intel_me_extend_valid(dev);

		if ((stat & 0xf000) >> 12 != 0) {
			printk(KERN_ERR "notme: ME is crippled on this board\n");
		} else {
			printk(KERN_ERR "notme: ME is active on this board\n");

			// Initialize HECI
			printk(KERN_INFO "notme: probe: Initializing HECI\n");
			intel_mei_setup(dev);

			// Reset
			mei_reset(dev);

			// Get version to kernel log
			printk(KERN_INFO "notme: probe: Get version\n");
			mkhi_get_fw_version(dev);

			// Reset
			mei_reset(dev);

			// Get firmware capability to kernel log
			printk(KERN_INFO "notme: probe: Get fw capabilities\n");
			mkhi_get_fwcaps(dev);
		}
	} else {
		printk(KERN_INFO "notme: probe: failed (%d)\n", err);
		return -ENODEV;
	}

	// Always fail to probe: module never fully inserts...
	pci_disable_device(dev);
	return -ENODEV;
}

static struct pci_driver notme_driver = {
	.name = "notme",
	.id_table = notme_ids,
	.probe = notme_probe,
};

static int __init notme_init(void)
{
	int err;
	printk(KERN_INFO "notme: === Damagement Engine driver loaded ===\n");
	err = pci_register_driver(&notme_driver);
	pci_unregister_driver(&notme_driver);
	printk(KERN_INFO "notme: === Damagement Engine driver unloaded ===\n");
	return -ENODEV;
}

module_init(notme_init);
