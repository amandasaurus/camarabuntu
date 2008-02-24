#! /bin/bash
CAMARABUNTU_DIR=${CAMARABUNTU_DIR:-${HOME}/camara/camarabuntu}
CAMARABUNTU_ISO=${1:-camarabuntu.iso}

qemu -cdrom ${CAMARABUNTU_DIR}/${CAMARABUNTU_ISO} -hda ${CAMARABUNTU_DIR}/hda.qcow -boot d -monitor stdio
