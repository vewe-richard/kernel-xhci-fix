#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <asm/stacktrace.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vewe Richard");



static int __init mymodule_init (void)
{
	printk("hello kernel\n");
	return 0;
}

static void __exit mymodule_exit (void)
{
}

module_init(mymodule_init);
module_exit(mymodule_exit);


