#! /bin/bash
for i in * ; do [ -d $i ] || continue ; ( cd $i ; dpkg-buildpackage -b -rfakeroot & ) ; done
