#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "asgn1_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("proGion - dangbin1982@gmail.com");
MODULE_DESCRIPTION("Assignment 1 - Simplified Char Driver with ioctl");
MODULE_VERSION("1.0");

#define DEVICE_NAME  "asgn1"
#define CLASS_NAME   "asgn1_class"
#define BUFFER_SIZE  1024
#define DRIVER_MAJOR 242

static int major_number = DRIVER_MAJOR;
static char device_buffer[BUFFER_SIZE];
static int buffer_len = 0;

static int open_count = 0;
static int write_count = 0;
static int read_count = 0;
static int last_write_size = 0;
static int driver_mode = 1; /* Mode 1: Echo mode (mac dinh), Mode 0: Kernel mode */

static struct class  *asgn1_class  = NULL;
static struct device *asgn1_device = NULL;
static DEFINE_MUTEX(asgn1_mutex);

static int asgn1_open(struct inode *inode, struct file *file) {
    mutex_lock(&asgn1_mutex);
    open_count++;
    mutex_unlock(&asgn1_mutex);
    return 0;
}

static int asgn1_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t asgn1_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    int bytes;
    mutex_lock(&asgn1_mutex);
    if (*offset >= buffer_len) {
        mutex_unlock(&asgn1_mutex);
        return 0;
    }
    bytes = min(len, (size_t)(buffer_len - *offset));
    if (copy_to_user(buf, device_buffer + *offset, bytes)) {
        mutex_unlock(&asgn1_mutex);
        return -EFAULT;
    }
    *offset += bytes;
    read_count++;
    mutex_unlock(&asgn1_mutex);
    return bytes;
}

static ssize_t asgn1_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    char *temp_buf;
    mutex_lock(&asgn1_mutex);
    len = min(len, (size_t)(BUFFER_SIZE - 12));

    temp_buf = kmalloc(len + 1, GFP_KERNEL);
    if (!temp_buf) {
        mutex_unlock(&asgn1_mutex);
        return -ENOMEM;
    }

    if (copy_from_user(temp_buf, buf, len)) {
        kfree(temp_buf);
        mutex_unlock(&asgn1_mutex);
        return -EFAULT;
    }
    temp_buf[len] = '\0';

    if (driver_mode == 0)
        snprintf(device_buffer, BUFFER_SIZE, "[KERNEL] %s", temp_buf);
    else
        snprintf(device_buffer, BUFFER_SIZE, "%s", temp_buf);

    kfree(temp_buf);
    buffer_len = strlen(device_buffer);
    last_write_size = len;
    write_count++;
    mutex_unlock(&asgn1_mutex);
    return len;
}

static long asgn1_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    mutex_lock(&asgn1_mutex);
    switch (cmd) {
    case ASGN1_RESET_BUFFER:
        memset(device_buffer, 0, BUFFER_SIZE);
        buffer_len = write_count = read_count = last_write_size = 0;
        pr_info("asgn1_driver: ioctl RESET_BUFFER\n");
        break;

    case ASGN1_GET_STATS: {
        struct asgn1_stats stats = { open_count, write_count, read_count, buffer_len, last_write_size };
        if (copy_to_user((void __user *)arg, &stats, sizeof(stats))) {
            mutex_unlock(&asgn1_mutex);
            return -EFAULT;
        }
        pr_info("asgn1_driver: ioctl GET_STATS\n");
        break;
    }

    case ASGN1_SET_MODE: {
        int new_mode;
        if (copy_from_user(&new_mode, (void __user *)arg, sizeof(int))) {
            mutex_unlock(&asgn1_mutex);
            return -EFAULT;
        }
        driver_mode = new_mode;
        pr_info("asgn1_driver: ioctl SET_MODE = %d\n", driver_mode);
        break;
    }

    case ASGN1_GET_VERSION: {
        char ver[64] = "asgn1_driver v1.0 - proGion";
        if (copy_to_user((void __user *)arg, ver, sizeof(ver))) {
            mutex_unlock(&asgn1_mutex);
            return -EFAULT;
        }
        pr_info("asgn1_driver: ioctl GET_VERSION\n");
        break;
    }

    default:
        mutex_unlock(&asgn1_mutex);
        return -ENOTTY;
    }
    mutex_unlock(&asgn1_mutex);
    return 0;
}

static struct file_operations asgn1_fops = {
    .owner          = THIS_MODULE,
    .open           = asgn1_open,
    .release        = asgn1_release,
    .read           = asgn1_read,
    .write          = asgn1_write,
    .unlocked_ioctl = asgn1_ioctl,
};

static int __init asgn1_init(void) {
    if (register_chrdev(major_number, DEVICE_NAME, &asgn1_fops) < 0)
        return -EIO;
    asgn1_class = class_create(THIS_MODULE, CLASS_NAME);
    asgn1_device = device_create(asgn1_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    pr_info("asgn1_driver: Loaded\n");
    return 0;
}

static void __exit asgn1_exit(void) {
    device_destroy(asgn1_class, MKDEV(major_number, 0));
    class_destroy(asgn1_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("asgn1_driver: Unloaded\n");
}

module_init(asgn1_init);
module_exit(asgn1_exit);
