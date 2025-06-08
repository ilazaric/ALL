#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kernel.h> /* for sprintf() */ 
#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h> /* for get_user and put_user */ 
#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "myreservedmem"

static const phys_addr_t ivl_phys_start = 0x200000000ULL;
static const size_t ivl_phys_size = 0x20000000ULL; // 512 MB
static const phys_addr_t ivl_phys_end = ivl_phys_start + ivl_phys_size;

static dev_t devt;
static struct cdev my_cdev;
static struct class *my_class;

static int myreservedmem_open(struct inode *inode, struct file *filp)
{
  try_module_get(THIS_MODULE);
  return 0;
}

static int myreservedmem_release(struct inode *inode, struct file *filp)
{
  module_put(THIS_MODULE);
  return 0;
}

static int myreservedmem_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long vma_size = vma->vm_end - vma->vm_start;

    if (vma_size > ivl_phys_size) {
      pr_err(DEVICE_NAME ": requested mmap size is too large\n");
      return -EINVAL;
    }

    // Calculate PFN (page frame number) of the physical base
    // Shift right by PAGE_SHIFT (typically 12 on x86_64).
    unsigned long pfn = ivl_phys_start >> PAGE_SHIFT;

    // We must ensure the region is page-aligned, etc.
    // In your case, 0x200000000 is nicely aligned.
    pr_info("myreservedmem: mmap pfn=0x%lx, size=%lu\n", pfn, vma_size);

    // Adjust protections if desired. For normal read/write caching:
    // vma->vm_page_prot = pgprot_cached(vma->vm_page_prot); 
    // or pgprot_cached(), pgprot_device(), etc. 
    // (depends on the usage you need: cacheable vs. uncached)

    // Map the physical pages into the user’s VMA
    if (remap_pfn_range(vma, vma->vm_start, pfn,
                        vma_size, vma->vm_page_prot)) {
        pr_err("myreservedmem: remap_pfn_range failed\n");
        return -EAGAIN;
    }

    return 0;
}

static const struct file_operations myreservedmem_fops = {
    .owner   = THIS_MODULE,
    .open    = myreservedmem_open,
    .release = myreservedmem_release,
    .mmap    = myreservedmem_mmap,
};

static int __init myreservedmem_init(void)
{
    int ret;

    // Allocate a device number
    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("myreservedmem: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&my_cdev, &myreservedmem_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, devt, 1);
    if (ret < 0) {
        pr_err("myreservedmem: cdev_add failed\n");
        unregister_chrdev_region(devt, 1);
        return ret;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) 
    my_class = class_create(DEVICE_NAME); 
#else 
    my_class = class_create(THIS_MODULE, DEVICE_NAME); 
#endif
    if (IS_ERR(my_class)) {
        pr_err("myreservedmem: class_create failed\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(my_class);
    }

    if (!device_create(my_class, NULL, devt, NULL, DEVICE_NAME)) {
        pr_err("myreservedmem: device_create failed\n");
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(devt, 1);
        return -EINVAL;
    }

    pr_info("myreservedmem: module loaded. Device /dev/%s major=%d minor=%d\n",
            DEVICE_NAME, MAJOR(devt), MINOR(devt));
    pr_info("myreservedmem: exposing region 0x%llx - 0x%llx\n",
            (unsigned long long)ivl_phys_start,
            (unsigned long long)(ivl_phys_end - 1));
    return 0;
}

static void __exit myreservedmem_exit(void)
{
    device_destroy(my_class, devt);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(devt, 1);

    pr_info("myreservedmem: module unloaded\n");
}

module_init(myreservedmem_init);
module_exit(myreservedmem_exit);

MODULE_AUTHOR("Ivan Lazaric");
MODULE_DESCRIPTION("Expose reserved physical memory via mmap");
MODULE_LICENSE("GPL");
