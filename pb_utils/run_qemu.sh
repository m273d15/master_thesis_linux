#!/bin/bash
MODE=$1
TEST_IMAGE=qemu-image.img
KERNEL=/home/kelvin/git/master_thesis_linux/arch/x86/boot/bzImage
IMAGE_OPTIONS=" -drive file=$TEST_IMAGE,index=0,media=disk,format=raw"
DEBUG_OPTIONS=" -s -S"

COMMAND="qemu-system-x86_64"
[ "$MODE" == "gdb" ] &&  COMMAND+=$DEBUG_OPTIONS
COMMAND+=" -kernel $KERNEL"
COMMAND+=$IMAGE_OPTIONS
COMMAND+=" -append \"root=/dev/sda rootwait rw single console=ttyS0\""
COMMAND+=" --enable-kvm"
COMMAND+=" --nographic"
COMMAND+=" --smp 1"
COMMAND+=" -net nic -net user"

eval $COMMAND
