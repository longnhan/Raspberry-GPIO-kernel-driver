#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/gpio.h>

#define BASE_GPIOCHIP   (u16)512
#define DRIVER_NAME "aio_gpio_driver"
#define MAX_BUF_SIZE (u8)80

#define GPIO_IOC_MAGIC 'G'
// #define GPIO_IOC_SET_DIRECTION _IOW(GPIO_IOC_MAGIC, 0, int)
#define GPIO_IOC_GET_VALUE _IOR(GPIO_IOC_MAGIC, 1, int)
#define GPIO_IOC_SET_VALUE _IOW(GPIO_IOC_MAGIC, 2, int)

static bool first;

struct gpio_data 
{
    int gpio;
    int value;
};

int misc_open(struct inode *node, struct file *filep)
{
    first = true;
    pr_info("aio_gpio_driver: %s, %d\n", __func__, __LINE__);

    pr_info("aio_gpio_driver: misc_open first is %d\n", first);
    pr_info("aio_gpio_driver: --------------------------------------\n");

    return 0;
}

int misc_release(struct inode *node, struct file *filep)
{
    pr_info("aio_gpio_driver: %s, %d\n", __func__, __LINE__);
    pr_info("aio_gpio_driver: --------------------------------------\n");
    first = false;

    return 0;
}

static ssize_t misc_read(struct file *filp, char __user *buf, size_t length,
                         loff_t *f_pos)
{
    size_t len=0;
    return len;
}

/*this function called when write to device file*/
static ssize_t misc_write(struct file *filp, const char __user *buf,
                          size_t length, loff_t *f_pos)
{
    char cmd[MAX_BUF_SIZE] = {0};
    char operation;
    u16 gpio_num = 0;
    u16 gpio_val = 0;
    char msg[MAX_BUF_SIZE] = {0};
    size_t len;

    pr_info("aio_gpio_driver: start misc_write to gpio\n");

    /*get cmd from user space*/
    pr_info("aio_gpio_driver: copy from user\n");
    if(copy_from_user(cmd, buf, length) != 0)
    {
        return -EFAULT;
    }

    cmd[length] = '\0';

    /*parse command*/
    pr_info("aio_gpio_driver: parse command\n");
    if(sscanf(cmd, "%c %hd %hd", &operation, &gpio_num, &gpio_val) < 2)
    {
        return -EINVAL;
    }

    gpio_num += BASE_GPIOCHIP;

    /*check if gpio is valid*/
    pr_info("aio_gpio_driver: check valid gpio\n");
    if(!gpio_is_valid(gpio_num))
    {
        pr_info("aio_gpio_driver: GPIO %d is not valid", gpio_num);
        return -EINVAL;
    }

    if(operation == 'w')
    {
        /*export GPIO unless already exported*/
        if(gpio_request(gpio_num, DRIVER_NAME))
        {
            pr_info("aio_gpio_driver: Fail to request gpio %d", gpio_num);
        }

        /*set gpio direction*/
        if(gpio_direction_output(gpio_num, gpio_val))
        {
            pr_info("aio_gpio_driver: Fail to set gpio %d", gpio_num);
            gpio_free(gpio_num);
        }

        pr_info("aio_gpio_driver: The GPIO %d has been set to %d", gpio_num - BASE_GPIOCHIP, gpio_val);

        gpio_set_value(gpio_num, gpio_val);

        pr_info("aio_gpio_driver: --------------------------------------\n");
    }
    else if(operation == 'r')
    {
        /*export GPIO unless already exported*/
        if(gpio_request(gpio_num, DRIVER_NAME))
        {
            pr_info("aio_gpio_driver: Fail to request gpio %d", gpio_num);
            return -EBUSY;
        }

        /*set gpio direction to input*/
        if(gpio_direction_input(gpio_num))
        {
            pr_info("aio_gpio_driver: Fail to set gpio %d as input", gpio_num);
            gpio_free(gpio_num);
        }

        /* Read the value of the GPIO pin */
        gpio_val = gpio_get_value(gpio_num);

        /* Prepare the message to be sent to the user space */
        len = snprintf(msg, MAX_BUF_SIZE, "GPIO%d value is %d\n", gpio_num - BASE_GPIOCHIP, gpio_val);

        /* Copy the message to the user space */
        if(copy_to_user((void __user *)buf, msg, min(len, length)))
        {
            gpio_free(gpio_num);
            return -EFAULT;
        }

        pr_info("aio_gpio_driver: Read GPIO %d value is %d", gpio_num - BASE_GPIOCHIP, gpio_val);
    
        pr_info("aio_gpio_driver: --------------------------------------\n");
    }
    else
    {
        return -EINVAL;
    }

    gpio_free(gpio_num);

    return length;
}

static long misc_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct gpio_data gpio_info;
    int ret = 0;
    int gpio_num;
    int value;

    switch(cmd) 
    {
#ifdef GPIO_IOC_SET_DIRECTION
        // TODO: merge set direction into write and read
        case GPIO_IOC_SET_DIRECTION:
            if (copy_from_user(&gpio_info, (struct gpio_data __user *)arg, sizeof(gpio_info)))
                return -EFAULT;

            gpio_num = gpio_info.gpio + BASE_GPIOCHIP;

            if (!gpio_is_valid(gpio_num))
                return -EINVAL;

            if (gpio_request(gpio_num, DRIVER_NAME))
                return -EBUSY;

            if (gpio_info.value == 1) 
            {
                gpio_direction_output(gpio_num, 0);
            } 
            else 
            {
                gpio_direction_input(gpio_num);
            }
            gpio_free(gpio_num);
            break;
#endif /*GPIO_IOC_SET_DIRECTION*/

        case GPIO_IOC_GET_VALUE:
            if (copy_from_user(&gpio_info, (struct gpio_data __user *)arg, sizeof(gpio_info)))
                return -EFAULT;

            gpio_num = gpio_info.gpio + BASE_GPIOCHIP;

            /*check GPIO if valid*/
            if (!gpio_is_valid(gpio_num))
                return -EINVAL;

            if (gpio_request(gpio_num, DRIVER_NAME))
                return -EBUSY;

            /*set GPIO direction*/
            gpio_direction_input(gpio_num);

            value = gpio_get_value(gpio_num);
            gpio_info.value = value;
            gpio_free(gpio_num);

            if (copy_to_user((struct gpio_data __user *)arg, &gpio_info, sizeof(gpio_info)))
                return -EFAULT;
            break;
        case GPIO_IOC_SET_VALUE: /*TODO test this gpio write*/
            if (copy_from_user(&gpio_info, (struct gpio_data __user *)arg, sizeof(gpio_info)))
                return -EFAULT;

            gpio_num = gpio_info.gpio + BASE_GPIOCHIP;

            /*check GPIO if value*/
            if (!gpio_is_valid(gpio_num))
                return -EINVAL;

            /*request to use GPIO*/
            if (gpio_request(gpio_num, DRIVER_NAME))
                return -EBUSY;

            /*set GPIO direction*/
            gpio_direction_output(gpio_num, 0);

            /*set GPIO output value*/
            gpio_set_value(gpio_num, gpio_info.value);

            /*free GPIO*/
            gpio_free(gpio_num);
            break;

        default:
            ret = -EINVAL;
    }

    return ret;
}

struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .open = misc_open,
    .release = misc_release,
    .read = misc_read,
    .write = misc_write,
    .unlocked_ioctl = misc_ioctl,
};

static struct miscdevice aio_gpio_driver = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "aio_gpio_driver",
    .fops = &misc_fops,
};

static int misc_init(void)
{
    printk("Load kernel module successfully\n");
    pr_info("aio_gpio_driver: misc module init\n");

    if(misc_register(&aio_gpio_driver) == 0)
    {
        pr_info("aio_gpio_driver: registered device file success\n");
        pr_info("aio_gpio_driver: --------------------------------------\n");
    }

    return 0;
}

static void misc_exit(void)
{
    pr_info("aio_gpio_driver: misc module exit\n");
    misc_deregister(&aio_gpio_driver);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_AUTHOR("longnhan <longnhan252@gmail.com>");
MODULE_DESCRIPTION("aio_gpio_driver");
MODULE_LICENSE("GPL");