#!/bin/bash 

set -e

stage=0

function usage {
cat <<FOO
$0 [-s <stage num> ]

default is to run all stages.

stages:
1: extract-cd-contents.sh
2: update_vanilla_cd.sh
3: add-debs-to-cd-image.py
4: make-cd-iso.py
5: qemu

FOO

}

 
while [ "$1" != "" ]; do
    case $1 in
        -h | --help )           usage
                                exit
                                ;;
        -s | --)                shift
                                stage=$1
                                ;;
        * )                     usage
                                exit 1
    esac
    shift
done


base_dir=$(echo $0 | sed "s/$(basename $0)$//")

# read in configuration vars
. $base_dir/do_it_all.conf

if [ $stage -le 1 ] ; then
    $base_dir/extract-cd-contents.sh $VANILLA_ISO $EXTRACTED_CD_DIR
fi

if [ $stage -le 2 ] ; then
    $base_dir/update_vanilla_cd.sh $GIT_DIR $EXTRACTED_CD_DIR
fi

if [ "$UBUNTU_KEYRING" ] ; then
        keyring_param="--ubuntu-keyring=$UBUNTU_KEYRING"
else
        keyring_param=""
fi

if [ $stage -le 3 ] ; then
    $base_dir/add-debs-to-cd-image.py --cd-dir=$EXTRACTED_CD_DIR --gpg-key=$GPGKEY $keyring_param --indices=$INDICES_DIR $DEBS_DIR/*.deb
fi

if [ $stage -le 4 ] ; then
    $base_dir/make-cd-iso.py --iso-file=$OUTPUT_ISO --dir=$EXTRACTED_CD_DIR --name=Camarabuntu
fi

if [ $stage -le 5 ] ; then
    sudo modprobe kqemu
    [ -e /dev/kqemu ] || sudo mknod /dev/kqemu c 250 0
    sudo chmod 666 /dev/kqemu

    qemu -cdrom $OUTPUT_ISO -hda $QEMU_HDA -boot d
fi
