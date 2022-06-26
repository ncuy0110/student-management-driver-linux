#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/string.h>

#include "vchar_device.h"

#define DRIVER_AUTHOR "NGUYEN CHON UY <nguyenchonuy2001@gmail.com>"
#define DRIVER_DESC "A sample loadable kernel module"


typedef struct vchar_dev {
  unsigned char * control_regs;
  unsigned char * status_regs;
  unsigned char * data_regs;
} vchar_dev_t;

struct _vchar_drv {
  dev_t dev_num;
  struct class *dev_class;
  struct device *dev;
  vchar_dev_t * vchar_hw;
  struct cdev *vcdev;
  unsigned int open_cnt;
} vchar_drv;


static int vchar_driver_open(struct inode *inode, struct file *filp) {
  vchar_drv.open_cnt++;
  printk("Handle opened event (%d)\n", vchar_drv.open_cnt);
  return 0;
}

static int vchar_driver_release(struct inode *inode, struct file *filp) {
  printk("Handle close event\n");
  return 0;
}

static ssize_t normalize_character(struct file *filp, char __user *user_buf, size_t len, loff_t *off) {
  char *kernel_buf = NULL;
  int num_bytes = len;
  int i;

  printk("Handle normalize event start from %lld, %zu bytes\n", *off, len);

  kernel_buf = kzalloc(len, GFP_KERNEL);

  if(copy_from_user(kernel_buf, user_buf, len))
    return -EFAULT;

  num_bytes -= 1;
  while (kernel_buf[0] == ' ') {
    for (i = 0; i<num_bytes-1; i++) {
      kernel_buf[i] = kernel_buf[i+1];
    }
    num_bytes -=1;
    kernel_buf[num_bytes] = '\0';
  }

  while (kernel_buf[num_bytes-1] == 32) {
    num_bytes -= 1;
    kernel_buf[num_bytes] = '\0';
  }

  if (kernel_buf[0] > 96 && kernel_buf[0] < 123) {
    kernel_buf[0] -= 32;
  }

  i = 1;
  while (i<num_bytes-2) {
    if (kernel_buf[i] == ' ' && kernel_buf[i+1] == ' ') {
      int j;
      for (j=i; j<num_bytes-1; j++){
        kernel_buf[j] = kernel_buf[j+1];
      }
      num_bytes -= 1;
      kernel_buf[num_bytes] = '\0';
    } else {
      i+=1;
    } 
  }

  for (i=1; i<num_bytes; i++) {
    if ((kernel_buf[i] >= 'a' && kernel_buf[i] <='z') && kernel_buf[i-1] == ' ') {
      kernel_buf[i] -= 32;
    }
  }

  if(kernel_buf == NULL)
    return 0;

  if (num_bytes < 0)
    return -EFAULT;

  if (copy_to_user(user_buf, kernel_buf, num_bytes))
    return -EFAULT;

  *off += num_bytes;
  return num_bytes;
}

static struct file_operations fops = {
  .owner    = THIS_MODULE,
  .open     = vchar_driver_open,
  .release  = vchar_driver_release,
  .read     = normalize_character,
};

int vchar_hw_init(vchar_dev_t *hw) {
  char * buf;
  buf = kzalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
  
  if (!buf) {
    return -ENOMEM;
  }

  hw->control_regs = buf;
  hw->status_regs = hw->control_regs + NUM_CTRL_REGS;
  hw->data_regs = hw->status_regs + NUM_STS_REGS;

  return 0;
}

void vchar_hw_exit(vchar_dev_t *hw) {
  kfree(hw->control_regs);
}

static int __init vchar_driver_init(void) {

  /* cap phat device number */
  int ret = 0;
  vchar_drv.dev_num = 0;

  ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "vchar_driver");
  if (ret < 0) {
    printk("failed to register device number dynamically\n");
    goto failed_register_devnum;
  }

  printk("allocated device number (%d, %d)\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));

  // tao file device
  vchar_drv.dev_class = class_create(THIS_MODULE, "class_vchar_dev");
  if (vchar_drv.dev_class == NULL) {
    printk("failed to create a device class\n");
    goto failed_create_class;
  }

  vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "vchar_dev");
  if (IS_ERR(vchar_drv.dev)) {
    printk("failed to create a device\n");
    goto failed_create_device;
  }

  vchar_drv.vchar_hw = kzalloc(sizeof(vchar_dev_t), GFP_KERNEL);
  if (!vchar_drv.vchar_hw) {
    printk("failed to allocate data structure of the driver\n");
    ret = -ENOMEM;
    goto failed_allocate_structure;
  }

  ret = vchar_hw_init(vchar_drv.vchar_hw);

  if (ret < 0) {
    printk("failed to initialize a virtual character device\n");
    goto failed_init_hw;
  }

  vchar_drv.vcdev = cdev_alloc();
  if (vchar_drv.vcdev == NULL) {
    printk("failed to allocate cdev structure\n");
    goto failed_allocate_cdev;
  }
  cdev_init(vchar_drv.vcdev, &fops);
  ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
  if (ret < 0) {
    printk("failed to add a char device to the system\n");
    goto failed_allocate_cdev;
  }

  printk("Init success\n");
  return 0;

failed_allocate_cdev:
  vchar_hw_exit(vchar_drv.vchar_hw);
failed_init_hw:
  kfree(vchar_drv.vchar_hw);
failed_allocate_structure:
  device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
failed_create_device:
  class_destroy(vchar_drv.dev_class);
failed_create_class:
  unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_devnum:
  return ret;
}

static void __exit vchar_driver_exit(void) {
  device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
  class_destroy(vchar_drv.dev_class);

  cdev_del(vchar_drv.vcdev);
  vchar_hw_exit(vchar_drv.vchar_hw);
  kfree(vchar_drv.vchar_hw);

  unregister_chrdev_region(vchar_drv.dev_num, 1);
  printk("Exit success\n");
}

module_init(vchar_driver_init);
module_exit(vchar_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
