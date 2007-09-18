#! /bin/bash
cd /home/rory/camara/camarabuntu/

IMAGE=camarabuntu-7.04.iso
BUILD=cd-image-7.04

rm $IMAGE
mkisofs -r -V "Camarabuntu 7.04" \
    -cache-inodes \
    -J -l -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot \
    -boot-load-size 4 -boot-info-table \
    -o $IMAGE $BUILD
