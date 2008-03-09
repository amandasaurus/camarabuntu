#!/bin/bash

# Darik's Boot & Nuke build script.

# Note that this script does not compile DBAN components.  If you wish to
# recompile something, then you must do it manually and install it to the
# in.boot folder or to the in.root folder.
#
# DBAN will not have a proper build system until the work is sponsored.


# Load the version information.
. ./etc/version

# Load the build options.
. ./etc/options


function dbanfail()
{
	echo -e "\nBuild failed."

	if [ "${DBAN_MOUNTFLAG}" == "yes" ]; then
		echo -e "\nUnmounting filesystem... "
		umount -v ${DBAN_MOUNT}
	fi

	if [ "${DBAN_LOOPFLAG}" == "yes" ]; then
		echo -en "\nReleasing loopback device... "
		losetup -d ${DBAN_LOOP} && echo -e "done."
	fi

	echo -e "\nRemoving build directory... "
	rm -rfv ${DBAN_TMP} 
	echo -e "\nFinished.\n"
	exit 1
}


function signalhandler()
{
	echo -e "\n\nCaught signal."
	dbanfail
}

function errhandler()
{
	echo -e "\n\nCaught intermediate failure or user break."
	dbanfail
}

trap signalhandler ERR

trap errhandler SIGHUP SIGINT SIGQUIT SIGTERM

#if [ `id -u` -ne 0 ]; then
#	echo -e "\nWarning: You may need root privileges to run this script."
#	sleep 3s
#fi

echo -e "\nBuilding Darik's Boot & Nuke... "


echo -e "\nChecking utilities... "

# The number of utilities that are missing.
DBAN_MISSING=0

echo -n " fakeroot:    "
which fakeroot
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " gzip:        "
which gzip
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " syslinux:    "
which syslinux
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " mcopy:       "
which mcopy
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " mkcramfs:    "
which mkcramfs
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " mkdosfs:     "
which mkdosfs
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " mkdiskimage: "
which mkdiskimage
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " mkisofs:     "
which mkisofs
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " perl:        "
which perl
DBAN_MISSING=$(($DBAN_MISSING + $?))

echo -n " zip:         "
which zip
DBAN_MISSING=$(($DBAN_MISSING + $?))

if [ $DBAN_MISSING -gt 0 ]; then
	echo -e "\nUtilities that are required to build DBAN are missing."
	dbanfail
fi

echo -e "\nClearing temporary directory... "
rm -rf ${DBAN_TMP} && mkdir -v ${DBAN_TMP} || dbanfail

echo -e "\nClearing out directories..."
rm -rf ./out.zip/ && mkdir -v ./out.zip || dbanfail

echo -e "\nCreating the root image..."

# Copy the version file.
cp -v etc/version in.root/ || dbanfail

# The kernel will not execute the rc if it is not owned by root.
fakeroot mkcramfs in.root ${DBAN_TMP}/initrd || dbanfail

echo -e "\nCompressing the root image..."
gzip -v9 ${DBAN_TMP}/initrd || dbanfail

echo -e "\nCreating the boot image... "
mkdosfs -C -f 1 -n "${DBAN_BOOTLABEL}" "${DBAN_BOOTFILE}" ${DBAN_BOOTSIZE} || dbanfail

echo -e "\nPopulating the boot filesystem..."
mcopy -s -v -i ${DBAN_BOOTFILE} in.boot/* ${DBAN_TMP}/initrd.gz :: || dbanfail

echo -en "\nInstalling the boot loader... "
syslinux ${DBAN_BOOTFILE} && echo -e "done." || dbanfail

echo -e "\nMaking ZIP package..."
cp -v -r ./in.zip/* ./out.zip/
cp -v -r ./etc/config ./out.zip/
cp -v ./etc/changelog.txt ./out.zip/
cp -v ./etc/notes.txt ./out.zip/
cp -v "${DBAN_BOOTFILE}" "./out.zip/${DBAN_FLPNAME}"

cd out.zip
# Archive order affects the behavior of the WinImage SFX module.
dos2unix <readme.txt | zip -9 -z "../${DBAN_ZIPNAME}" ${DBAN_FLPNAME} || dbanfail
zip -9 -r -u "../${DBAN_ZIPNAME}" * || dbanfail
cd -

echo -e "\nMaking ISO package..."
mkisofs -l -V"${DBAN_BOOTLABEL}" -b "${DBAN_FLPNAME}" -o "${DBAN_ISONAME}" ./out.zip/ || dbanfail

echo -en "\nRemoving build directory... "
rm -rf ${DBAN_TMP} && echo -e "done."

echo -e "\nBuild succeeded.\n"

# eof
