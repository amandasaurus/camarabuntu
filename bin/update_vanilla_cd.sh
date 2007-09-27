#!/bin/bash

# automate the process of updating a vanilla edubuntu cd with our customisations
# usage: $0 <path to git clone of camarabuntu project> <path to cd tree>

camarabuntu_dir=$1
cd_image_dir=$2

set -e

ISO_DIRS="isolinux preseed"

for dir in $ISO_DIRS ; do
  cp -r $camarabuntu_dir/$dir/* $cd_image_dir/$dir/
done
