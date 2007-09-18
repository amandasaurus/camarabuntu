#! /bin/bash

ISO=$1
DIR_NAME=$2

if [ ! -d "${DIR_NAME}" ] ; then
    mkdir -p "${DIR_NAME}"
fi

TMP_DIR=$(mktemp -d cd-image-XXXXXX)
echo -n "Mounting the CD image now, you may need to enter your root password "
sudo mount -o loop "${ISO}" "${TMP_DIR}"
echo " ... Done"


echo -n "CD mounted, starting file copy "
cp -rT "${TMP_DIR}" "${DIR_NAME}"
echo " ... Done"

sudo umount "${TMP_DIR}"
rmdir "${TMP_DIR}"

echo -n "Updating permissions on cd image "
find "${DIR_NAME}" -exec chmod +w '{}' ';'
echo " ... Done"

echo "All Done"


