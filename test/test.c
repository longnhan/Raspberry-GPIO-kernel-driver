#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define GPIO_IOC_MAGIC 'G'
// #define GPIO_IOC_SET_DIRECTION _IOW(GPIO_IOC_MAGIC, 0, int)
#define GPIO_IOC_GET_VALUE _IOR(GPIO_IOC_MAGIC, 1, int)
#define GPIO_IOC_SET_VALUE _IOW(GPIO_IOC_MAGIC, 2, int)

#define GPIO_INPUT 0
#define GPIO_OUTPUT 1
#define GPIO_HIGH 1
#define GPIO_LOW 0

struct gpio_data 
{
    int gpio;
    int value;
};

/**
 * TODO test the kernel driver
 * 
    sudo ./gpio_control set_dir 21 1  # Set GPIO 21 as output
    sudo ./gpio_control set_val 21 1  # Set GPIO 21 to high
    sudo ./gpio_control get_val 21    # Get the value of GPIO 21
 *
 */

int main(int argc, char *argv[]) {
    int fd;
    struct gpio_data gpio_info;

    if (argc < 3) 
    {
        printf("Usage: %s <command> <gpio> [value]\n", argv[0]);
        return -1;
    }

    char *command = argv[1];
    gpio_info.gpio = atoi(argv[2]);

    fd = open("/dev/aio_gpio_driver", O_RDWR);
    if (fd < 0) 
    {
        perror("Failed to open device file");
        return -1;
    }
#ifdef GPIO_IOC_SET_DIRECTION
    if (strcmp(command, "set_dir") == 0) 
    {
        if (argc != 4) 
        {
            printf("Usage: %s set_dir <gpio> <direction>\n", argv[0]);
            close(fd);
            return -1;
        }
        gpio_info.value = atoi(argv[3]);
        ioctl(fd, GPIO_IOC_SET_DIRECTION, &gpio_info);
    }
#endif
    else if (strcmp(command, "set_val") == 0) 
    {
        if (argc != 4) 
        {
            printf("Usage: %s set_val <gpio> <value>\n", argv[0]);
            close(fd);
            return -1;
        }
        gpio_info.value = atoi(argv[3]);
        ioctl(fd, GPIO_IOC_SET_VALUE, &gpio_info);
    } 
    else if (strcmp(command, "get_val") == 0) 
    {
        ioctl(fd, GPIO_IOC_GET_VALUE, &gpio_info);
        printf("GPIO%d value is %d\n", gpio_info.gpio, gpio_info.value);
    } 
    else 
    {
        printf("Invalid command. Use 'set_dir', 'set_val', or 'get_val'.\n");
    }

    close(fd);
    return 0;
}
