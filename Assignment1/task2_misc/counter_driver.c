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
    int bytes;

    if (*offset > 0)
        return 0; /* Tra ve EOF de tranh doc lap vo han */

    bytes = snprintf(kbuf, sizeof(kbuf), "%d\n", atomic_read(&counter));
    atomic_inc(&counter);     /* Tu dong tang counter khi doc */
    atomic_inc(&total_reads); /* Tang tong so lan doc */

    if (copy_to_user(buf, kbuf, bytes))
        return -EFAULT;

    *offset = bytes;
    return bytes;
}

static ssize_t counter_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char kbuf[32];
    int val;

    if (len >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    if (kstrtoint(strim(kbuf), 10, &val) < 0 || val < 0)
        return -EINVAL; /* Loi neu khong phai so hoac la so am */

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
    misc_register(&counter_miscdev);
    proc_create("counter_info", 0444, NULL, &counter_proc_ops);
    pr_info("counter_driver: Loaded /dev/counter\n");
    return 0;
}

static void __exit counter_exit(void) {
    remove_proc_entry("counter_info", NULL);
    misc_deregister(&counter_miscdev);
    pr_info("counter_driver: Unloaded\n");
}

module_init(counter_init);
module_exit(counter_exit);
