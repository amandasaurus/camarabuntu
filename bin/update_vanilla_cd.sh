#!/bin/bash

# automate the process of updating a vanilla edubuntu cd with our customisations
# usage: $0 <path to git clone of camarabuntu project> <path to cd tree>

function help {
        echo "USAGE: $0 <path to git clone of camarabuntu project> <path to cd tree>"
        exit 1
}

case $1 in 
        "-h"|"--help")
                help
esac

camarabuntu_dir=$1
cd_image_dir=$2

if [ ! "$camarabuntu_dir" -o ! "$cd_image_dir" ] ; then
        help
fi

set -e

ISO_DIRS="isolinux preseed"

for dir in $ISO_DIRS ; do
  echo "updating $dir from git"
  cp -r $camarabuntu_dir/$dir/* $cd_image_dir/$dir/
done
