#!/bin/zsh

echo "How do you want to access Pi zero"
echo "1:build  | 2: Transfer .ko"
echo "Insert number here: "

#TODO: modify depends on your board
BOARD_ADDR=pi@raspberrypi0.local

read MY_OPTION

case $MY_OPTION in
  1)
    # build kernel module
    mkdir -p result/
    make
    mv *.o ./result/
    mv *.mod ./result/
    mv *.mod.c ./result/
    mv *.ko ./result/
    mv Module.symvers ./result/
    mv modules.order ./result/
    find . -maxdepth 1 -name "*.cmd" -exec mv {} ./result/ \;

    ;;
  2)
    # transfer from latop
    echo "Transfer file"
    echo "Insert path kernel file"
    # read LOCAL_FILE
    echo "storage path on Pi"
    # read PI_LOCATION
    # scp $LOCAL_FILE $BOARD_ADDR:$PI_LOCATION
    scp ./result/aio_gpio_driver.ko $BOARD_ADDR:~/01_Projects
    echo "File is stored at ~/01_Projects"
    ;;
    
   *)
    # Invalid option5
    echo "Not a valid option. Please choose 1 or 2."
    ;;
esac
