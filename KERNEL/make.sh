export PATH=/work/bin/arm/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH

if ! [ -f ".config" ]; then
    make ARCH=arm beagleboneblack_defconfig
    ! [ $? = 0 ] && exit 1
fi

make ARCH=arm CROSS_COMPILE=arm-linux- dtbs
! [ $? = 0 ] && exit 1

make ARCH=arm CROSS_COMPILE=arm-linux- zImage -j5
! [ $? = 0 ] && exit 1

if test; then
    make ARCH=arm CROSS_COMPILE=arm-linux- modules -j5
    ! [ $? = 0 ] && exit 1

    make ARCH=arm CROSS_COMPILE=arm-linux- modules_install
    ! [ $? = 0 ] && exit 1
fi
