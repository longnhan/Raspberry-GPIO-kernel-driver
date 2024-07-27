#!/bin/bash

#./kernel-log-runtime.sh monitor -> run the script

# Function to display existing dmesg logs for aio_gpio_driver
display_logs() {
    echo "Displaying dmesg logs for aio_gpio_driver..."
    dmesg | grep 'aio_gpio_driver:'
}

# Function to monitor dmesg logs in real-time for aio_gpio_driver
monitor_logs() {
    echo "Monitoring dmesg logs for aio_gpio_driver..."
    dmesg -w | grep --line-buffered 'aio_gpio_driver:'
}

# Main script logic
if [ "$1" == "monitor" ]; then
    monitor_logs
else
    display_logs
fi
