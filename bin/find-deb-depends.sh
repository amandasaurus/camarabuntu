#!/bin/bash

# crap script to download all dependencies of a .deb
# it's probably a better idea just to try installing the deb from within edubuntu

# invocation: $0 <deb name>

bin_location=$(echo $0 | sed "s/$(basename $0)//")
# apt-cdrom -c bin/apt.conf.cdrom -d ~/edubuntu-cd/ -m add

EXTRAS_DIR="pool/extras/"
POOL_DIR="pool/"

deb=$1

depends=`dpkg-deb --show --showformat='${Depends}' $deb  |  tr "," "\n" | sed 's/(.*)//g' `

echo depends $depends
skip=
for deb_name in $depends ; do
    # alternatives are seperated by "|" - the next 2 stanzas handle them properly
    if [ $deb_name == "|"  ] ; then
        if [ "$found" ] ; then
            skip="true"
        fi
        continue
    fi
        
    echo -n $deb_name" "

    if [ "$skip" ] ; then
        echo "satisfied by alternative"
        skip=
        continue
    fi
       
    # now see if the dependency is on the cd
    apt-get --config-file /home/tom/camarabuntu.git/camarabuntu/bin/apt.conf.cdrom update 
    found=$(apt-cache -c /home/tom/camarabuntu.git/camarabuntu/bin/apt.conf.cdrom search "^$deb_name$")

    # if not then apt-get it. unfortunately this also gets all _its_ dependencies. should be a way to just get it
    if [ !  "$found" ] ; then
        echo "not found"
        apt-get --config-file /home/tom/camarabuntu.git/camarabuntu/bin/apt.conf.all update
        apt-get -y --config-file /home/tom/camarabuntu.git/camarabuntu/bin/apt.conf.all -d install $deb_name
    else
        echo "found"
    fi

    # TODO - this copies all debs with the same prefix. not good.
    cp bin/cache.all/archives/$deb_name* $EXTRAS_DIR/
    # TODO - we should make a list of everything in $EXTRAS_DIR and append the deb we download to that list so we can recursively get all dependencies.
    # to do things really properly we'd need to regenerate the repo after each download so that we don't download the same thing multiple times.

done
