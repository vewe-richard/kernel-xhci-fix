// SPDX-License-Identifier: GPL-2.0
/*
 * Simple stub driver to reserve a PCI device
 *
 * Copyright (C) 2008 Red Hat, Inc.
 * Author:
 *	Chris Wright
 *
 * Usage is simple, allocate a new id to the stub driver and bind the
 * device to it.  For example:
 *
 * # echo "8086 10f5" > /sys/bus/pci/drivers/pci-stub/new_id
 * # echo -n 0000:00:19.0 > /sys/bus/pci/drivers/e1000e/unbind
 * # echo -n 0000:00:19.0 > /sys/bus/pci/drivers/pci-stub/bind
 * # ls -l /sys/bus/pci/devices/0000:00:19.0/driver
 * .../0000:00:19.0/driver -> ../../../bus/pci/drivers/pci-stub
 */

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>

#include "xhci.h"

static char ids[1024] __initdata;

module_param_string(ids, ids, sizeof(ids), 0);
MODULE_PARM_DESC(ids, "Initial PCI IDs to add to the stub driver, format is "
		 "\"vendor:device[:subvendor[:subdevice[:class[:class_mask]]]]\""
		 " and multiple comma separated entries can be specified");

static const struct pci_device_id pci_ids[] = {
	{PCI_DEVICE(0x1234, 0xffe8)},
	{0,},
};

/*************** Sysfs functions **********************/
volatile int etx_value = 0;
struct kobject *kobj_ref;

static ssize_t  sysfs_show(struct kobject *kobj, 
		                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj, 
		                        struct kobj_attribute *attr, const char *buf, size_t count);
struct kobj_attribute etx_attr = __ATTR(etx_value, 0660, sysfs_show, sysfs_store);

static struct pci_dev *_myDev = NULL;

#define PCI_CONF1_ADDRESS(bus, devfn, reg) \
	(0x80000000 | ((reg & 0xF00) << 16) | (bus << 16) \
	| (devfn << 8) | (reg & 0xFC))


void pci_dev_access_analysis(void)
{
	u32 addr;
	int res;
	struct pci_dev *dev = _myDev;
	void __iomem *mm;
	u32 tmp;

#if 0
	outl(PCI_CONF1_ADDRESS(0, 40, 0x10), 0xCF8);
	res = 109;
	addr = inl(0xCFC);
#else
	res = dev->bus->ops->read(dev->bus, dev->devfn, PCI_BASE_ADDRESS_0, 4, &addr);
	if(pci_request_region(dev, addr, "qj") != 0)
	{
		pci_info(dev, "can not reserver region\n");
		return;
	}
	mm = pci_iomap(dev, 0, 100);
	if(mm == NULL)
	{
		pci_info(dev, "can not mmap\n");
		return;
	}
	for(res = 0; res < 100; res ++)
	{
		tmp = ioread32(mm + res);
		pci_info(dev, "%x: %x\n", res, tmp);
	}
#endif
	pci_info(dev, "bus read ret: %d, bar %x\n", res, (unsigned int)addr);
}


void xhci_test02(void)
{
}

void xhci_test01(void)
{
	struct pci_dev *dev;
	struct usb_hcd *hcd;
	struct xhci_hcd	*xhci;
	u32 params1;

	dev = pci_get_device(0x8086, 0xa36d, NULL);
	if(dev == NULL)
	{
		printk(KERN_ERR "can not get pci device!\n");
		for_each_pci_dev(dev)
		{
			printk(KERN_ERR "foreach %u %u", dev->vendor, dev->device);
		}
		return;
	}
	printk(KERN_ERR "dev %p\n", dev);
	hcd = dev_get_drvdata(&dev->dev);
	xhci = hcd_to_xhci(hcd);
	printk(KERN_ERR "xhci: %p %pK %px %pr\n", xhci->cap_regs,
			(void *)xhci->cap_regs, (void *)xhci->cap_regs, xhci);
	printk(KERN_ERR "hcd: %p %llx\n", hcd->regs, hcd->rsrc_start);

	params1 = readl(&xhci->cap_regs->hcs_params1);
	printk(KERN_ERR "readl %pK : %u\n", &xhci->cap_regs->hcs_params1, params1);
	//xhci->hcs_params1 = readl(&xhci2->cap_regs->hcs_params1);
	//xhci->hcs_params2 = readl(&xhci2->cap_regs->hcs_params2);
	//xhci->hcs_params3 = readl(&xhci2->cap_regs->hcs_params3);
	//xhci->hcc_params = readl(&xhci->cap_regs->hc_capbase);
	//xhci->hci_version = HC_VERSION(xhci->hcc_params);
	//xhci->hcc_params = readl(&xhci->cap_regs->hcc_params);


	printk(KERN_ERR "xhci hcs %x %x %x\n", xhci->hcs_params1, 
			xhci->hcs_params2,xhci->hcs_params3);


	pci_dev_put(dev);
}


/*
 * ** This function will be called when we read the sysfs file
 * */
static ssize_t sysfs_show(struct kobject *kobj, 
		                struct kobj_attribute *attr, char *buf)
{
	        pr_info("Sysfs - Read!!!\n");
		pci_dev_access_analysis();
		return sprintf(buf, "%d", etx_value);
}
/*
 * ** This function will be called when we write the sysfsfs file
 * */
static ssize_t sysfs_store(struct kobject *kobj, 
		                struct kobj_attribute *attr,const char *buf, size_t count)
{
	        pr_info("Sysfs - Write!!!\n");
		sscanf(buf,"%d",&etx_value);
	        pr_info("value: %d!!!\n", etx_value);
		if(etx_value == 1)
		{
			xhci_test01();
		}
		else if(etx_value == 2)
		{
			xhci_test02();
		}
		return count;
}




static int pci_stub_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	u8 revision;
	u32 val;
	unsigned long start, end, flags;

	pci_info(dev, "claimed by stub\n");
	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &val);
	pci_info(dev, "claimed by stub revision %d bar %x\n", revision, (unsigned int)val);

	//res = dev->bus->ops->read(dev->bus, dev->devfn, PCI_BASE_ADDRESS_0, 4, &addr);
	//pci_info(dev, "bus read ret: %d, bar %x\n", res, (unsigned int)addr);
	_myDev = pci_dev_get(dev);
	for(val = 0; val < 6; val ++)
	{
		start = pci_resource_start(dev, val);
		end = pci_resource_end(dev, val);
		flags = pci_resource_flags(dev, val);
		pci_info(dev, "%lx:%lx %lx\n", start,end, flags);
	}
	



	return 0;
}

static struct pci_driver stub_driver = {
	.name		= "pci-stub",
	.id_table	= pci_ids,
	.probe		= pci_stub_probe,
};

static int __init pci_stub_init(void)
{
	int rc;

	pr_err("jiangjqian sysinfo xx\n");
	rc = pci_register_driver(&stub_driver);
	if (rc)
		return rc;

        kobj_ref = kobject_create_and_add("etx_sysfs",kernel_kobj);
	 
        /*Creating sysfs file for etx_value*/
        if(sysfs_create_file(kobj_ref, &etx_attr.attr)){
                pr_err("Cannot create sysfs file......\n");
		goto r_sysfs;
    	}
	return 0;
r_sysfs:
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &etx_attr.attr);

	pci_unregister_driver(&stub_driver);
	return -1;
}

static void __exit pci_stub_exit(void)
{
	if(_myDev) pci_dev_put(_myDev);
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &etx_attr.attr);
	pci_unregister_driver(&stub_driver);
}

module_init(pci_stub_init);
module_exit(pci_stub_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris Wright <chrisw@sous-sol.org>");
