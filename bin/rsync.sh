#! /bin/bash
rsync -aPh --exclude=hda.img "$*" * nayru:camara/camarabuntu/
