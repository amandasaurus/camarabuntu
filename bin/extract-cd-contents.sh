#! /bin/bash

ISO=$1
DIR_NAME=$2

function help {
        echo "USAGE: $0 <path of .iso> <path to dir to extract to>"
        exit
}

case $1 in 
        "-h"|"--help")
                help
esac

if [ ! "$ISO" -o ! "$DIR_NAME" ] ; then
        help
fi

function cleanup {
  local status=$1
  sudo umount "${TMP_DIR}"
  rmdir "${TMP_DIR}"
  exit $status
}

trap cleanup SIGINT SIGTERM

if [ ! -d "${DIR_NAME}" ] ; then
    mkdir -p "${DIR_NAME}" || exit $?
else
    rm -rf "${DIR_NAME}"
fi

TMP_DIR=$(mktemp -t -d cd-image-XXXXXX)
[ "$TMP_DIR" ] || exit 1

echo "Mounting the CD image now, you may need to enter your root password "
sudo mount -o loop "${ISO}" "${TMP_DIR}" || exit $?
echo " ... Done"

echo -n "CD mounted, starting file copy "
cp -rT "${TMP_DIR}" "${DIR_NAME}" || cleanup $?
echo " ... Done"

echo -n "Updating permissions on cd image "
find "${DIR_NAME}" -exec chmod +w '{}' ';'  # why not just 'chmod -R +w'   ?
echo " ... Done"

echo "All Done"

cleanup
