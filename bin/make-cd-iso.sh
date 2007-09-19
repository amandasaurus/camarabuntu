#! /bin/bash

while getopts ":f:d:n:" options; do
  case $options in
    f ) ISO_NAME=$OPTARG;;
    d ) DIR=$OPTARG;;
    n ) NAME=$OPTARG;;
  esac
done

mkisofs -r -V "${NAME}" \
    -cache-inodes \
    -J -l -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot \
    -boot-load-size 4 -boot-info-table \
    -o $ISO_NAME $DIR

