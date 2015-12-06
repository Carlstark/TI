export PATH=/work/bin/arm/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH

make am335x_evm_defconfig
! [ $? = 0 ] && exit 1

make CROSS_COMPILE=arm-linux- -j2
! [ $? = 0 ] && exit 1
