#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("proGion - dangbin1982@gmail.com");
MODULE_DESCRIPTION("Task 2 - Misc Counter Driver");
MODULE_VERSION("1.0");

static atomic_t counter = ATOMIC_INIT(0);
static atomic_t total_reads = ATOMIC_INIT(0);
static atomic_t total_writes = ATOMIC_INIT(0);

static ssize_t counter_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    char kbuf[32];
    int val, bytes_to_copy;

    if (*offset > 0)
        return 0; /* EOF - Ngung doc khi da tra ve xong du lieu */

    val = atomic_read(&counter);
    atomic_inc(&counter); /* Tu dong tang 1 khi doc */
    atomic_inc(&total_reads);

    snprintf(kbuf, sizeof(kbuf), "%d\n", val);
    bytes_to_copy = strlen(kbuf);

    if (copy_to_user(buf, kbuf, bytes_to_copy))
        return -EFAULT;

    *offset = bytes_to_copy;
    return bytes_to_copy;
}

static ssize_t counter_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char kbuf[32];
    int val, ret;

    if (len >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    ret = kstrtoint(strim(kbuf), 10, &val);
    if (ret < 0 || val < 0)
        return -EINVAL; /* Tra ve loi neu nhap chuoi khong phai so hoac la so am */

    atomic_set(&counter, val);
    atomic_inc(&total_writes);

    return len;
}

static const struct file_operations counter_fops = {
    .owner = THIS_MODULE,
    .read  = counter_read,
    .write = counter_write,
};

static struct miscdevice counter_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "counter",
    .fops  = &counter_fops,
};

static int counter_proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "=== Counter Stats (proGion) ===\n");
    seq_printf(m, "Current Counter : %d\n", atomic_read(&counter));
    seq_printf(m, "Total Reads     : %d\n", atomic_read(&total_reads));
    seq_printf(m, "Total Writes    : %d\n", atomic_read(&total_writes));
    return 0;
}

static int counter_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, counter_proc_show, NULL);
}

static const struct proc_ops counter_proc_ops = {
    .proc_open    = counter_proc_open,
    .proc_read    = seq_read,
    .proc_release = single_release,
};

static int __init counter_init(void) {
    int ret;
    ret = misc_register(&counter_miscdev);
    if (ret) {
        pr_err("counter_driver: Could not register misc device\n");
        return ret;
    }

    if (!proc_create("counter_info", 0444, NULL, &counter_proc_ops)) {
        pr_warn("counter_driver: Could not create /proc/counter_info\n");
    }

    pr_info("counter_driver: Loaded successfully /dev/counter\n");
    return 0;
}

static void __exit counter_exit(void) {
    remove_proc_entry("counter_info", NULL);
    misc_deregister(&counter_miscdev);
    pr_info("counter_driver: Unloaded successfully\n");
}

module_init(counter_init);
module_exit(counter_exit);
