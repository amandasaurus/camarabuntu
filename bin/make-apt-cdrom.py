#! /usr/bin/python

from optparse import OptionParser
import os, shutil, tempfile, commands, glob, sys, re, subprocess, shutil, time

parser = OptionParser()


parser.add_option( "-i", "--iso-file",
                   dest="isofilename", default=None,
                   help="The filename of the .iso that'll be made" )

parser.add_option( "-n", "--cd-name",
                   dest="name", default=None,
                   help="The name of the CD" )

parser.add_option( "-d", "--deb-dir",
                   dest="debdir", default=None,
                   help="The directory that has all the debs" )

parser.add_option( "-k", "--gpg-key",
                    dest="gpgkey", default=None, type="string",
                    help="The GPG key used to sign the packages" )


(options, unneeded) = parser.parse_args()

debdir = os.path.abspath( options.debdir )

# this is the CD we work in and will have be the cd image layout
apt_cdrom_dir = tempfile.mkdtemp( prefix="apt-cdrom-", dir=os.getcwd() )

# this is our apt-move config file
apt_config, apt_config_filename = tempfile.mkstemp( prefix="apt-move.conf-", dir=os.getcwd() )
# TODO dist
open( apt_config_filename, "w" ).write("""APTSITES="/all/"
LOCALDIR=%(apt_cdrom_dir)s
DIST=dapper
PKGTYPE=binary
FILECACHE=%(debdir)s
LISTSTATE=/var/lib/apt/lists
DELETE=no
MAXDELETE=20
COPYONLY=yes
PKGCOMP=gzip
CONTENTS=no
GPGKEY=
""" % {'debdir':debdir, 'apt_cdrom_dir':apt_cdrom_dir} )

status, output = commands.getstatusoutput("apt-move -c \"%s\" update" % apt_config_filename)
if status != 0:
    print output
#assert status == 0

# remake the packages file
status, output = commands.getstatusoutput( "apt-ftparchive packages \"%(aptdir)s/pool/main/\" | gzip -9c > \"%(aptdir)s/dists/dapper/main/binary-i386/Packages.gz\"" % {'aptdir':apt_cdrom_dir} )
if status != 0:
    print output
#assert status == 0

# release file:
release_file, release_filename = tempfile.mkstemp( prefix="release-", dir=os.getcwd() )
open( release_filename, "w" ).write("""APT::FTPArchive::Release {
Origin "APT-Move";
Label "APT-Move";
Suite "dapper";
Codename "dapper";
Architectures "i386";
Components "main";
Description "%s";
};""" % options.name )

os.remove( os.path.join( apt_cdrom_dir, "dist/dapper/Release" ) )
status, output = commands.getstatusoutput( "apt-ftparchive -c ~/myapt.conf release dists/dapper/ > Release" )

status,output = commands.getstatusoutput( "gpg -ba  --default-key=%(gpg_key)s -o \"%(aptdir)s/dists/dapper/Release.gpg\" \"%(aptdir)s/dists/dapper/Release\"" % {'gpg_key':options.gpgkey, 'aptdir':apt_cdrom_dir } )
if status != 0:
    print output

# details:
# https://help.ubuntu.com/community/AptMoveHowto
