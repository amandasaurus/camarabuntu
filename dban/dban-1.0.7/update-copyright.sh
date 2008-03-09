#!/bin/sh

# Update the copyright files for utilities that are bundled in the DBAN source
# tarball.  This script assumes a Debian system, which is where I usually work.

if [ ! -e /etc/debian_version ]
then
	echo "Error: This script is intended for Debian systems."
	exit 1
fi

for i in `ls ./bin/*`
do

	DBAN_BASENAME=$(basename "$i")
	DBAN_WHICH=$(which "$DBAN_BASENAME")

	# Check whether a package specifically provides the file.
	DBAN_PACKAGE=$(dpkg -S $DBAN_WHICH | cut -f1 -d:)

	if [ "X$DBAN_PACKAGE" == "X" ]
	then
		# Just use the first package that provides the name.
		DBAN_PACKAGE="`dpkg -S $DBAN_BASENAME | cut -f1 -d: | head -n1`"
	fi

	if [ "X$DBAN_PACKAGE" != "X" ]
	then
		mkdir -v -p "./doc/$DBAN_PACKAGE"	
		cp -v "/usr/share/doc/$DBAN_PACKAGE/copyright" "./doc/$DBAN_PACKAGE/"
	fi

done

# eof
