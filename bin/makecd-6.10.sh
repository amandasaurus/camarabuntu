#! /bin/bash
IMAGE=camarabuntu-6.10.iso
BUILD=cd-image-6.10

mkisofs -r -V "Camarabuntu 6.10" \
  -cache-inodes \
  -J -l -b isolinux/isolinux.bin \
  -c isolinux/boot.cat -no-emul-boot \
  -boot-load-size 4 -boot-info-table \
  -o $IMAGE $BUILD

