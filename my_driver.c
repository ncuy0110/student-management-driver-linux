#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>

#define DRIVER_AUTHOR "NGUYEN CHON UY <nguyenchonuy2001@gmail.com>"
#define DRIVER_DESC "A sample loadable kernel module"

struct _vchar_drv {
  dev_t dev_num;
} vchar_drv;

static int __init driver_init(void) {
  int ret = 0;

  vchar_drv.dev_num = 0;
  ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_driver");
  if (ret < 0) {
    printk("failed to register device number dynamically\n");
    goto failed_register_devnum;
  }
  printk("Init success\n");
  return 0;

failed_register_devnum:
  return ret;
}

static void __exit driver_exit(void) {
  unregister_chrdev_region(vchar_drv.dev_num, 1);
  printk("Exit success\n");
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
