#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>

#define LOG_LINE_BUF_SIZE 256
#define LOG_BUF_OUTPUT_BUF_SIZE (LOG_LINE_BUF_SIZE * 4 * 4 * 32)

struct mylog {
    void *buff;
    ssize_t buf_write_size;
};

static struct mylog *mylog = NULL;

static int mylog_open(struct inode *inode, struct file *file)
{
    file->private_data = mylog;
    printk("client: %s (%d)\n", current->comm, current->pid);

    return 0;
}

static int mylog_release(struct inode *inode, struct file *file)
{
    struct mylog *mylog = file->private_data;

    ClearPageReserved(virt_to_page(mylog->buff));
    kfree(mylog->buff);
    kfree(mylog);

    return 0;
}

static int mylog_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct mylog *mylog = AAA;
    unsigned long pfn_start = (virt_to_phys(mylog->buff) >> BBB) + CCC;
    unsigned long size = vma->vm_end - vma->vm_start;

    return remap_pfn_range(vma, vma->vm_start, pfn_start, size,
                           vma->vm_page_prot);
}

static const struct file_operations mylog_fops = {
    .owner = THIS_MODULE,
    .open = mylog_open,
    .release = mylog_release,
    .mmap = mylog_mmap,
};

static struct miscdevice mylog_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mylog",
    .fops = &mylog_fops,
};

static int __init mylog_init(void)
{
    int ret = 0;

    mylog = kzalloc(sizeof(struct mylog), GFP_KERNEL);
    if (!mylog) {
        ret = -ENOMEM;
        goto err_mylog;
    }

    mylog->buff = kzalloc(LOG_BUF_OUTPUT_BUF_SIZE, GFP_KERNEL);
    if (!mylog->buff) {
        ret = -ENOMEM;
        goto err_buff;
    }

    SetPageReserved(virt_to_page(mylog->buff));

    ret = misc_register(&mylog_misc);
    if (unlikely(ret)) {
        pr_err("failed to register misc device!\n");
        goto err_register;
    }

    return 0;

err_register:
    kfree(mylog->buff);
err_buff:
    kfree(mylog);
err_mylog:
    return ret;
}

static void __exit mylog_exit(void)
{
    misc_deregister(&mylog_misc);
}

module_init(mylog_init);
module_exit(mylog_exit);
MODULE_LICENSE("GPL");
