#! /usr/bin/python

from optparse import OptionParser
import os, shutil, tempfile, commands, glob, sys, re, subprocess, shutil, time

from apt import Package

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

assert options.isofilename is not None, "You must specify the filename for the resultant ISO file with --iso-file/-i"
assert options.name is not None, "You must provide a name for the CD with --name/-n"
assert options.debdir is not None, "You must provide a directory that contains all the debs using --deb-dir/-d"
assert options.gpgkey is not None, "You must provide the GPG key to sign the packages using --gpg-key/-g"

debdir = os.path.abspath( options.debdir )

# this is the CD we work in and will have be the cd image layout
apt_cdrom_dir = tempfile.mkdtemp( prefix="make-apt-cdrom-cdrom-", dir=os.getcwd() )

# TODO remove 'dapper' hardcoding'
for dir in ["pool", "pool/main", "dists", "dists/dapper", "dists/dapper/main", "dists/dapper/main/binary-i386"]:
    os.mkdir( os.path.join( apt_cdrom_dir, dir ) )

for package, filename in [(Package(filename=filename), filename) for filename in glob.glob( os.path.join( debdir, "*.deb" ) )]:
    print "Found deb %s" % package
    if package.name[0:3] == 'lib':
        dir_name = package.name[0:4]
    else:
        dir_name = package.name[0]
    dir_name = os.path.join( apt_cdrom_dir, "pool", "main", dir_name, package.name )
    if not os.path.isdir( dir_name ):
        os.makedirs( dir_name )
    shutil.copy( filename, dir_name )
    

# remake the packages file
# TODO remove 'dapper' hardcoding
status, output = commands.getstatusoutput( "apt-ftparchive packages \"%(aptdir)s/pool/main/\" | gzip -9c > \"%(aptdir)s/dists/dapper/main/binary-i386/Packages.gz\"" % {'aptdir':apt_cdrom_dir} )
if status != 0:
    print output
#assert status == 0

# release file:
config_file, config_filename = tempfile.mkstemp( prefix="make-apt-cdrom-release-conf-", dir=os.getcwd() )
# TODO remove tha 'apt-move' stuff. What else should it be called
open( config_filename, "w" ).write("""APT::FTPArchive::Release {
Origin "APT-Move";
Label "APT-Move";
Suite "dapper";
Codename "dapper";
Architectures "i386";
Components "main";
Description "%s";
};""" % options.name )

status, output = commands.getstatusoutput( "apt-ftparchive -c \"%(config_filename)s\" release %(aptdir)s/dists/dapper/" % {'config_filename':config_filename, 'aptdir':apt_cdrom_dir } )

if status != 0:
    print output
assert status == 0

open( os.path.join( apt_cdrom_dir, 'dists', 'dapper', 'Release'), 'w').write( output )

status,output = commands.getstatusoutput( "gpg -ba  --default-key=%(gpg_key)s -o \"%(aptdir)s/dists/dapper/Release.gpg\" \"%(aptdir)s/dists/dapper/Release\"" % {'gpg_key':options.gpgkey, 'aptdir':apt_cdrom_dir } )
if status != 0:
    print output

# Add the .disk directory to tell ubuntu about this
os.mkdir( os.path.join( apt_cdrom_dir, ".disk" ) )
open( os.path.join( apt_cdrom_dir, '.disk', 'info' ), 'w' ).write( options.name )

# Add the signature to the disk
status, output = commands.getstatusoutput( "gpg --export -a \"%s\" > \"%s\"" % (options.gpgkey, os.path.join( apt_cdrom_dir, "signature.key" ) ) )
if status != 0:
    print output
assert status == 0

# make the cd iso
status, output = commands.getstatusoutput( "mkisofs -r -A \"%(cd_name)s\" -o \"%(iso_filename)s\" \"%(aptdir)s\" " % {'cd_name':options.name, 'iso_filename':options.isofilename, 'aptdir':apt_cdrom_dir } )
if status != 0:
    print output
assert status == 0

print "Created the ISO"
