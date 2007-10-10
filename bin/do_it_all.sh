#!/bin/bash 

set -e
set -x

base_dir=$(echo $0 | sed "s/$(basename $0)$//")

. $base_dir/do_it_all.conf

$base_dir/extract-cd-contents.sh $VANILLA_ISO $EXTRACTED_CD_DIR

$base_dir/update_vanilla_cd.sh $GIT_DIR $EXTRACTED_CD_DIR

if [ "$UBUNTU_KEYRING" ] ; then
        keyring_param="--ubuntu-keyring=$UBUNTU_KEYRING"
else
        keyring_param=""
fi

$base_dir/add-debs-to-cd-image.py --cd-dir=$EXTRACTED_CD_DIR --gpg-key=$GPGKEY $keyring_param --indices=$INDICES_DIR $DEBS_DIR/*.deb

$base_dir/make-cd-iso.py --iso-file=$OUTPUT_ISO --dir=$EXTRACTED_CD_DIR --name=Camarabuntu

sudo modprobe kqemu
[ -e /dev/kqemu ] || sudo mknod /dev/kqemu c 250 0
sudo chmod 666 /dev/kqemu

qemu -cdrom $OUTPUT_ISO -hda $QEMU_HDA -boot d
