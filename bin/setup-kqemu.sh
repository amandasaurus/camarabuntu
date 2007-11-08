#! /bin/bash
sudo modprobe kqemu

if [ $? -eq 1 ] ; then
    echo "kQEMU was not installed. Press enter to install it now, or Control-C to quit. This might take a few minutes and might require internet access. It might ask if you want to install some source files, press enter if that happens"
    read
    sudo apt-get install qemu kernel-package linux-source kqemu-source build-essential
    sudo module-assistant prepare
    sudo module-assistant build kqemu
    sudo module-assistant install kqemu
fi

sudo mknod /dev/kqemu c 250 0
sudo chmod 666 /dev/kqemu

