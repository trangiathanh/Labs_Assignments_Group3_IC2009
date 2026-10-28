#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/random.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "sms_sensor"
#define PROC_NAME "sms_stats"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Group 3");
MODULE_DESCRIPTION("SMS Sensor Kernel Driver");

static int major_number;
static u64 total_bytes_read = 0;
static int min_temp = 99999;
static int max_temp = -99999;
static int has_sample = 0;

static int proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "total_bytes_read: %llu\n", total_bytes_read);
    if (!has_sample) {
        seq_printf(m, "min_temperature: N/A\n");
        seq_printf(m, "max_temperature: N/A\n");
    } else {
        seq_printf(m, "min_temperature: %d.%02d\n", min_temp / 100, min_temp % 100);
        seq_printf(m, "max_temperature: %d.%02d\n", max_temp / 100, max_temp % 100);
    }
    return 0;
}

static int proc_open(struct inode *inode, struct file *file) {
    return single_open(file, proc_show, NULL);
}

static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char kbuf[64];
    int len;
    u32 rand_val;
    int temp_val, hum_val;
    unsigned long ms = jiffies_to_msecs(jiffies);

    get_random_bytes(&rand_val, sizeof(rand_val));
    temp_val = 1500 + (rand_val % 3001); // 15.00C -> 45.00C
    get_random_bytes(&rand_val, sizeof(rand_val));
    hum_val = 3000 + (rand_val % 5001);  // 30.00% -> 80.00%

    if (temp_val < min_temp) min_temp = temp_val;
    if (temp_val > max_temp) max_temp = temp_val;
    has_sample = 1;

    len = snprintf(kbuf, sizeof(kbuf), "%lu,%d.%02d,%d.%02d\n",
                   ms, temp_val / 100, temp_val % 100, hum_val / 100, hum_val % 100);

    if (count < len) return -EINVAL;

    if (copy_to_user(buf, kbuf, len)) {
        return -EFAULT;
    }

    total_bytes_read += len;
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
};

static int __init sms_driver_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;
    proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    return 0;
}

static void __exit sms_driver_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    unregister_chrdev(major_number, DEVICE_NAME);
}

module_init(sms_driver_init);
module_exit(sms_driver_exit);
