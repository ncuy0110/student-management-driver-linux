#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DRIVER_AUTHOR "NGUYEN CHON UY <nguyenchonuy2001@gmail.com>"
#define DRIVER_DESC "A sample loadable kernel module"

static int __init init_hello(void) {
  printk("Hello Vietnam\n");
  return 0;
}

static void __exit exit_hello(void) {
  printk("Goodbye Vietname\n");
}

module_init(init_hello);
module_exit(exit_hello);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
