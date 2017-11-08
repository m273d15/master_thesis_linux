#!/bin/bash
image_path=$GHOME/qemu-image.img
mount_path=/mnt/qemu
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $dir/inject_mod

make all
[ "$?" != "0" ] && echo "Failed to build inject module" && exit 1

cd $dir

sudo mkdir -p $mount_path
sudo mount -o loop $image_path $mount_path

sudo cp $dir/inject_mod/pb_task_inject.ko $mount_path/root
sudo cp $dir/parse_dmesg.pl $mount_path/root

sudo umount /mnt/qemu
