rsync -aP --exclude=hda.img * nayru:camara/camarabuntu/
ssh nayru /home/rory/camara/camarabuntu/makecd-7.04.sh
ssh -X nayru qemu -cdrom /home/rory/camara/camarabuntu/camarabuntu-7.04.iso -hda /home/rory/camara/camarabuntu/hda.img -boot d
#qemu -cdrom /home/rory/camara/camarabuntu/camarabuntu-7.04.iso -hda /home/rory/camara/camarabuntu/hda.img -boot d
