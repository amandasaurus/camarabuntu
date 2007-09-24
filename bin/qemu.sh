#! /bin/bash

ssh -X nayru sudo qemu -cdrom /home/rory/camara/camarabuntu/$1 -hda /home/rory/camara/camarabuntu/hda.img -boot d
