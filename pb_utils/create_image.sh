#!/bin/bash
IMG=qemu-image-big.img
DIR=mount-point.dir
DEBIAN_VERSION=jessie

qemu-img create $IMG 2g

mkfs.ext2 $IMG
mkdir $DIR

sudo mount -o loop $IMG $DIR

sudo debootstrap --arch amd64 "$DEBIAN_VERSION" $DIR

sudo umount $DIR
rmdir $DIR
