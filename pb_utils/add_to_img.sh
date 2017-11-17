#!/bin/bash
image_path=$GHOME/qemu-image.img
mount_path=/mnt/qemu
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $dir/mod_gen

./plan_to_module.pl

cd mods

make all
[ "$?" != "0" ] && echo "Failed to build inject module" && exit 1

cd $dir

sudo mkdir -p $mount_path
sudo mount -o loop $image_path $mount_path

sudo cp -r $dir/mod_gen $mount_path/root
sudo cp -r $dir/execute_tests.pl $mount_path/root
sudo cp -r $dir/exclude_tests.txt $mount_path/root

sudo umount /mnt/qemu
