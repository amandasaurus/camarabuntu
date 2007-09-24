#! /bin/bash

for DIST in warty hoary breezy dapper edgy feisty ; do
    wget http://ie.archive.ubuntu.com/ubuntu/indices/override.$DIST.{extra.main,main,main.debian-installer,restricted,restricted.debian-installer};
done
