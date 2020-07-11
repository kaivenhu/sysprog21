#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define LOG_LINE_BUF_SIZE 32
#define LOG_BUF_OUTPUT_BUF_SIZE (LOG_LINE_BUF_SIZE << PAGE_SHIFT)

struct mylog {
    void *buff;
    ssize_t buf_write_size;
};

static struct mylog *GLOG = NULL;

static int mylog_open(struct inode *inode, struct file *file)
{
    file->private_data = GLOG;
    printk("client: %s (%d) open mylog\n", current->comm, current->pid);

    return 0;
}

static int mylog_release(struct inode *inode, struct file *file)
{
    struct mylog *mylog = file->private_data;
    printk("client: %s (%d) release mylog\n", current->comm, current->pid);

    return 0;
}


static int mylog_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct mylog *mylog = file->private_data;
    unsigned long pfn_start =
        (virt_to_phys(mylog->buff) >> PAGE_SHIFT) + vma->vm_pgoff;
    unsigned long size = vma->vm_end - vma->vm_start;
    printk("client: %s (%d) mmap mylog\n", current->comm, current->pid);

    if (LOG_BUF_OUTPUT_BUF_SIZE < ((vma->vm_pgoff << PAGE_SHIFT) + size)) {
        return -EINVAL;
    }

    return remap_pfn_range(vma, vma->vm_start, pfn_start, size,
                           vma->vm_page_prot);
}

static const struct file_operations mylog_fops = {
    .owner = THIS_MODULE,
    .open = mylog_open,
    .release = mylog_release,
    .mmap = mylog_mmap,
};

static ssize_t mylog_show(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          char *buf)
{
    ssize_t len = 0;
    for (int i = 0; i < LOG_LINE_BUF_SIZE; ++i) {
        char tmp[32] = {'\0'};
        len += snprintf(tmp, sizeof(tmp), "[%d]: %s\n", i,
                        (char *) (GLOG->buff + (i << PAGE_SHIFT)));
        strncat(buf, tmp, sizeof(tmp));
    }
    return len;
}
static struct kobj_attribute mylog_attr = __ATTR(mylog, 0444, mylog_show, NULL);

static struct attribute *attrs[] = {
    &mylog_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *mylog_kobj;

static struct miscdevice mylog_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mylog",
    .fops = &mylog_fops,
};

static int __init mylog_init(void)
{
    int ret = 0;

    GLOG = kzalloc(sizeof(struct mylog), GFP_KERNEL);
    if (!GLOG) {
        ret = -ENOMEM;
        goto err_mylog;
    }

    GLOG->buff = kzalloc(LOG_BUF_OUTPUT_BUF_SIZE, GFP_KERNEL);
    if (!GLOG->buff) {
        ret = -ENOMEM;
        goto err_buff;
    }

    SetPageReserved(virt_to_page(GLOG->buff));

    ret = misc_register(&mylog_misc);
    if (unlikely(ret)) {
        pr_err("failed to register misc device!\n");
        goto err_register;
    }

    mylog_kobj = kobject_create_and_add("mylog", kernel_kobj);
    if (!mylog_kobj) {
        pr_err("failed to create kobj!\n");
        goto err_create_kobj;
    }
    if (sysfs_create_group(mylog_kobj, &attr_group)) {
        pr_err("failed to create kobj!\n");
        goto err_create_kobj;
    }


    return 0;

err_create_kobj:
    kobject_put(mylog_kobj);
err_register:
    ClearPageReserved(virt_to_page(GLOG->buff));
    kfree(GLOG->buff);
    GLOG->buff = NULL;
err_buff:
    kfree(GLOG);
    GLOG = NULL;
err_mylog:
    return ret;
}

static void __exit mylog_exit(void)
{
    kobject_put(mylog_kobj);
    ClearPageReserved(virt_to_page(GLOG->buff));
    kfree(GLOG->buff);
    GLOG->buff = NULL;
    kfree(GLOG);
    GLOG = NULL;
    misc_deregister(&mylog_misc);
}

module_init(mylog_init);
module_exit(mylog_exit);
MODULE_LICENSE("GPL");
