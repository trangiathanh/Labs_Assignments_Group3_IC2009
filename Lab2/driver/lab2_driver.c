/*
 * lab2_driver.c - LAB-02: Complete Character Device Driver
 * 
 * Driver nay cung cap:
 *   - Character device /dev/lab2
 *   - Procfs interface: /proc/lab2_info
 *   - Sysfs attributes: /sys/class/lab2_class/lab2/
 *   - Mutex bao ve device_buffer
 *
 * Tac gia: Nhom 3 - IC2009
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#define DEVICE_NAME "lab2"
#define CLASS_NAME  "lab2_class"
#define BUF_SIZE    1024
#define PROC_NAME   "lab2_info"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nhom 3 - IC2009");
MODULE_DESCRIPTION("LAB-02: Complete Character Device Driver");
MODULE_VERSION("1.0");

/* Bien toan cuc */
static int major_number;
static struct class *lab2_class = NULL;
static struct device *lab2_device = NULL;
static struct proc_dir_entry *proc_entry;
static char device_buffer[BUF_SIZE];
static int buffer_len = 0;
static int open_count = 0;
static char last_data[BUF_SIZE];

/* ============================================================
 * MUTEX - Bao ve device_buffer
 * DEFINE_MUTEX tao mutex bao ve device_buffer khoi race condition
 * khi nhieu process cung truy cap driver.
 * ============================================================ */
static DEFINE_MUTEX(lab2_mutex);

/* ============================================================
 * HAM lab2_open - Duoc goi khi user mo /dev/lab2
 * Nhiem vu: tang open_count, in log
 * LUU Y: KHONG lock mutex o day de tranh deadlock voi lab2_write
 * ============================================================ */
static int lab2_open(struct inode *inode, struct file *file)
{
    open_count++;
    pr_info("lab2_driver: device opened (count=%d)\n", open_count);
    return 0;
}

/* ============================================================
 * HAM lab2_release - Duoc goi khi user dong /dev/lab2
 * Nhiem vu: in log
 * LUU Y: KHONG unlock mutex o day
 * ============================================================ */
static int lab2_release(struct inode *inode, struct file *file)
{
    pr_info("lab2_driver: device closed\n");
    return 0;
}

/* ============================================================
 * HAM lab2_read - Doc du lieu tu device
 * 
 * Tai sao dung copy_to_user thay vi memcpy:
 *   - Kernel space va User space la hai khong gian dia chi ao
 *     hoan toan tach biet.
 *   - memcpy() chi copy trong cung khong gian dia chi.
 *   - copy_to_user() kiem tra tinh hop le cua dia chi user space
 *     truoc khi copy, tranh page fault va bao ve kernel.
 * ============================================================ */
static ssize_t lab2_read(struct file *file, char __user *buf,
                         size_t len, loff_t *offset)
{
    int bytes_to_read;
    
    if (*offset >= buffer_len)
        return 0;
    
    bytes_to_read = (len < buffer_len) ? len : buffer_len;
    
    if (copy_to_user(buf, device_buffer, bytes_to_read)) {
        return -EFAULT;
    }
    
    *offset += bytes_to_read;
    pr_info("lab2_driver: sent %d bytes to user\n", bytes_to_read);
    return bytes_to_read;
}

/* ============================================================
 * HAM lab2_write - Ghi du lieu vao device
 * 
 * Tai sao dung copy_from_user thay vi memcpy:
 *   - copy_from_user kiem tra dia chi user space truoc khi copy
 *     vao kernel space.
 *   - Tranh kernel panic neu user truyen dia chi khong hop le.
 * 
 * Su dung mutex_lock/mutex_unlock de bao ve device_buffer.
 * ============================================================ */
static ssize_t lab2_write(struct file *file, const char __user *buf,
                          size_t len, loff_t *offset)
{
    if (len > BUF_SIZE)
        len = BUF_SIZE;
    
    mutex_lock(&lab2_mutex);
    
    if (copy_from_user(device_buffer, buf, len)) {
        mutex_unlock(&lab2_mutex);
        return -EFAULT;
    }
    
    buffer_len = len;
    device_buffer[buffer_len] = '\0';
    
    if (buffer_len > 0 && device_buffer[buffer_len-1] == '\n') {
        device_buffer[buffer_len-1] = '\0';
        buffer_len--;
    }
    
    memset(last_data, 0, BUF_SIZE);
    memcpy(last_data, device_buffer, buffer_len);
    
    mutex_unlock(&lab2_mutex);
    pr_info("lab2_driver: received %zu bytes: [%s]\n", len, device_buffer);
    return len;
}

/* File operations */
static struct file_operations lab2_fops = {
    .owner   = THIS_MODULE,
    .open    = lab2_open,
    .release = lab2_release,
    .read    = lab2_read,
    .write   = lab2_write,
};

/* ============================================================
 * PROCFS - Su dung seq_file API
 * 
 * seq_file API va single_open():
 *   - seq_file cung cap co che doc du lieu kernel an toan.
 *   - single_open() don gian hoa viec tao seq_file khi chi can
 *     hien thi du lieu mot lan.
 * ============================================================ */
static int lab2_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "LAB-02 Driver Statistics\n");
    seq_printf(m, "======================\n");
    seq_printf(m, "Driver name  : %s\n", DEVICE_NAME);
    seq_printf(m, "Major number : %d\n", major_number);
    seq_printf(m, "Buffer size  : %d bytes\n", BUF_SIZE);
    seq_printf(m, "Data length  : %d bytes\n", buffer_len);
    seq_printf(m, "Open count   : %d\n", open_count);
    seq_printf(m, "Last data    : %s\n", buffer_len ? last_data : "None");
    return 0;
}

static int lab2_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, lab2_proc_show, NULL);
}

static const struct proc_ops lab2_proc_fops = {
    .proc_open    = lab2_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ============================================================
 * SYSFS - Su dung DEVICE_ATTR_RO
 * 
 * DEVICE_ATTR_RO va kobject:
 *   - DEVICE_ATTR_RO tao attribute file chi doc trong sysfs.
 *   - kobject la co che quan ly thiet bi trong kernel.
 *   - sysfs_create_group() tao nhom attribute files.
 * ============================================================ */

static ssize_t buffer_len_show(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", buffer_len);
}
static DEVICE_ATTR_RO(buffer_len);

static ssize_t open_count_show(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", open_count);
}
static DEVICE_ATTR_RO(open_count);

static ssize_t last_data_show(struct device *dev,
                              struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", buffer_len ? last_data : "None");
}
static DEVICE_ATTR_RO(last_data);

static struct attribute *lab2_attrs[] = {
    &dev_attr_buffer_len.attr,
    &dev_attr_open_count.attr,
    &dev_attr_last_data.attr,
    NULL,
};

static struct attribute_group lab2_attr_group = {
    .attrs = lab2_attrs,
};

/* ============================================================
 * HAM lab2_init - Duoc goi khi insmod module
 * Nhiem vu: dang ky device, tao procfs/sysfs
 * ============================================================ */
static int __init lab2_init(void)
{
    int ret;
    pr_info("lab2_driver: initializing module\n");
    
    ret = register_chrdev(0, DEVICE_NAME, &lab2_fops);
    if (ret < 0) {
        pr_alert("lab2_driver: failed to register char device\n");
        return ret;
    }
    major_number = ret;
    
    lab2_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(lab2_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(lab2_class);
    }
    
    lab2_device = device_create(lab2_class, NULL,
                                MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(lab2_device)) {
        class_destroy(lab2_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(lab2_device);
    }
    
    sysfs_create_group(&lab2_device->kobj, &lab2_attr_group);
    proc_entry = proc_create(PROC_NAME, 0444, NULL, &lab2_proc_fops);
    
    pr_info("lab2_driver: loaded, major=%d\n", major_number);
    return 0;
}

/* ============================================================
 * HAM lab2_exit - Duoc goi khi rmmod module
 * Nhiem vu: giai phong tai nguyen
 * ============================================================ */
static void __exit lab2_exit(void)
{
    if (proc_entry)
        proc_remove(proc_entry);
    
    sysfs_remove_group(&lab2_device->kobj, &lab2_attr_group);
    device_destroy(lab2_class, MKDEV(major_number, 0));
    class_destroy(lab2_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    
    pr_info("lab2_driver: module unloaded\n");
}

module_init(lab2_init);
module_exit(lab2_exit);
