#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/random.h>

#define DEVICE_NAME "emulated_cython"
#define CLASS_NAME "OpenBCI"

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Andrey Parfenov");    
MODULE_DESCRIPTION ("OpenBCI Cython serial emulator");  
MODULE_VERSION ("1.0");  

static int major_number;
static unsigned int total_bytes_readed = 0;
static struct class *cython_class = NULL;
static struct device *cython_device = NULL;
static unsigned char *data = NULL;
static unsigned int package_size = 33;
module_param (package_size, uint, S_IRUGO);
static unsigned int buffer_size = 330;
module_param (buffer_size, uint, S_IRUGO);
static char current_command = 's';
static unsigned int current_buffer_position = 0;
static int device_open = 0;
static const char welcome_message[] = "OpenBCI Emulator $$$";

static int cython_open (struct inode *inode, struct file *f);
static int cython_release (struct inode *inode, struct file *f);
static ssize_t cython_read (struct file *f, char *buf, size_t count, loff_t *f_pos);
static ssize_t cython_write (struct file *f, const char *buf, size_t count, loff_t *f_pos);
static void cython_exit (void);
static int cython_init (void);

static struct file_operations cython_fops = {
    .open = cython_open,
    .read = cython_read,
    .write = cython_write,
    .release = cython_release,
};

module_init (cython_init);
module_exit (cython_exit);

/* the init function is called once, when the module is inserted */
static int cython_init (void) 
{
    major_number = register_chrdev (0, DEVICE_NAME, &cython_fops);
    if (major_number < 0)
    {
        printk (KERN_ALERT "Failed to reguster chardev %d\n", major_number);
        return major_number;
    }
    printk (KERN_INFO "Registered device with major number: %d\n", major_number);

    cython_class = class_create (THIS_MODULE, CLASS_NAME);
    if (IS_ERR (cython_class))
    {
        unregister_chrdev (major_number, DEVICE_NAME);
        printk (KERN_ALERT "Failed to register device class\n");
        return PTR_ERR (cython_class);
    }
    printk (KERN_INFO "Device class registered correctly\n");

    cython_device = device_create (cython_class, NULL, MKDEV (major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR (cython_device))
    {
        class_destroy (cython_class);
        unregister_chrdev (major_number, DEVICE_NAME);
        printk (KERN_ALERT "Failed to create the device\n");
        return PTR_ERR (cython_device);
    }
    printk (KERN_INFO "Device class created correctly\n");

    data = kmalloc (buffer_size, GFP_KERNEL);
    if (!data)
    { 
        printk (KERN_ALERT "kmalloc error\n");
        cython_exit ();
        return -ENOMEM; 
    } 

    memset (data, 0, buffer_size);
    printk (KERN_INFO "Cython module is loaded\n"); 

    return 0;
}

static void cython_exit (void)
{
    device_destroy (cython_class, MKDEV (major_number, 0));
    class_unregister (cython_class);
    class_destroy (cython_class);
    unregister_chrdev (major_number, DEVICE_NAME);

    printk (KERN_INFO "Cython module is unloaded\n"); 
    if (data)
        kfree (data);
}

static int cython_open (struct inode *inode, struct file *f)
{
    if (device_open)
    {
        printk (KERN_ALERT, "Device is opened\n");;
        return -EBUSY;
    }
    printk (KERN_INFO, "Open port was called\n");
    device_open++;
    return 0;
}

static int cython_release (struct inode *inode, struct file *f)
{
    printk (KERN_INFO, "Close port was called\n");
    device_open--;
    return 0;
}

static ssize_t cython_write (struct file *f, const char *buf, size_t count, loff_t *f_pos)
{
    if (!count)
        return -EIO;

    if (count > 2)
        printk (KERN_WARNING "You should write single character\n");

    switch (buf[0])
    {
        case 'b':
            current_command = 'b';
            current_buffer_position = 0;
            break;
        case 'v':
            current_command = 'v';
            current_buffer_position = 0;
            break;
        case 's':
            current_command = 's';
            current_buffer_position = 0;
            break;
        default:
            printk (KERN_ALERT "Unsupported command\n");
    }

    return count;
}

static ssize_t cython_read (struct file *f, char *buf, size_t count, loff_t *f_pos)
{
    mdelay (4);

    int i, j;

    if (current_command == 'v')
    {
        size_t welcome_len = sizeof (welcome_message);
        if (current_buffer_position >= welcome_len)
            return 0;

        size_t bytes_to_write = 0;
        if (count > (welcome_len + current_buffer_position))
            bytes_to_write = welcome_len - current_buffer_position;
        else
            bytes_to_write = count;

        int error_count = 0;
        error_count = copy_to_user (buf, welcome_message + current_buffer_position, bytes_to_write);
        if (!error_count)
        {
            printk (KERN_INFO "Send %d to userspace\n", bytes_to_write);
            current_buffer_position += bytes_to_write;
            return bytes_to_write;
        }
        else
        {
            printk (KERN_ALERT "Unable to send %d to userspace\n", bytes_to_write);
            return -EIO;
        }
    }

    if (current_command == 's')
        return 0;

    if (current_command == 'b')
    {
        if (current_buffer_position % package_size == 0)
            current_buffer_position = 0;

        char rand_number;
        int bytes_to_write = 0;
        int package_num = total_bytes_readed / package_size;
        for (i = 0; i < buffer_size / package_size; i++)
        {
            data[i * package_size] = 0xA0;
            data[i * package_size + package_size - 1] = 0xC0;
            data[i * package_size + 1] = package_num;
            for (j = 2; j < package_size - 1; j++)
            {
                get_random_bytes (&rand_number, sizeof (rand_number));
                data[i * package_size + j] = rand_number;
            }
        }

        if (count > current_buffer_position + buffer_size)
            bytes_to_write = buffer_size - current_buffer_position;
        else
            bytes_to_write = count;
        total_bytes_readed += bytes_to_write;

        int error_count = 0;
        error_count = copy_to_user (buf, data + current_buffer_position, bytes_to_write);
        if (!error_count)
        {
            printk (KERN_INFO "Send %d to userspace\n", bytes_to_write);
            current_buffer_position += bytes_to_write;
            return bytes_to_write;
        }
        else
        {
            printk (KERN_ALERT "Unable to send %d to userspace\n", bytes_to_write);
            return -EIO;
        }
    }

    printk (KERN_ALERT "Unsupported command\n");
    return 0;
}
