#! /usr/bin/python

from optparse import OptionParser
import os, shutil, tempfile, commands, glob, sys, re, subprocess, shutil, time

usage = "%prog [options] <list of .debs>"

parser = OptionParser(usage=usage)

parser.add_option("-d", "--cd-dir",
                  dest="cddir", default=None,
                  help="The directory of the install cd details")

parser.add_option( "-k", "--gpg-key",
                    dest="gpgkey", default=None, type="string",
                    help="The GPG key used to sign the packages" )

parser.add_option( "--ubuntu-keyring",
                   dest="keyring", default=None, type="string",
                   help="The directory of the ubuntu keyring source. if not provided it will be downloaded." )

parser.add_option( "--indices",
                   dest="indices", default=None, type="string",
                   help="The directory of the ubuntu indices" )
                    
parser.add_option( "--passphrase",
                   dest="passphrase", default=None, type="string",
                   help="The passphrase for the gpg key" )

(options, orig_debs) = parser.parse_args()

if not orig_debs or not options.cddir or not options.gpgkey or not options.indices:
    parser.print_help()
    sys.exit(1)

assert os.path.isdir( options.indices )

debs = [os.path.abspath( deb ) for deb in orig_debs ]

cddir = os.path.abspath( options.cddir )
indices = os.path.abspath( options.indices )

# see what the name of the distro is, (eg dapper, edgy.. )
dists_dir = [ dir for dir in os.listdir( os.path.join ( cddir, 'dists' ) ) if dir not in ['stable', 'unstable' ] ]

assert len(dists_dir) == 1
dist = dists_dir[0]

dist_name_to_version = { 'warty':'4.10', 'hoary':'5.04', 'breezy':'5.10',
                         'dapper':'6.06', 'edgy':'6.10', 'feisty':'7.04',
                         'gutsy':'7.10', 'hardy':'8.04' }

assert dist in dist_name_to_version.keys()

def RunCommand(cmd, msg=None):
    print "running : %s" % cmd
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    returncode = p.wait()
    print "output : ", p.stdout.read()
    print "stderr: ", p.stderr.read()
    if returncode != 0:
      print "FAILED!"
      if msg is not None: print msg
      sys.exit(returncode)
      
def makedir_if_not_exist( *path_parts ):
    path = os.path.join( *path_parts )
    if not os.path.exists( path ):
        os.makedirs( path )

makedir_if_not_exist( cddir, 'dists', dist, 'extras', 'binary-i386' )
makedir_if_not_exist( cddir, 'pool', 'extras' )
makedir_if_not_exist( cddir, 'isolinux' )
makedir_if_not_exist( cddir, 'preseed' )

# Copy all the debs to the extras file
for deb in debs:
    print "Copying %s" % deb
    shutil.copy( deb, os.path.join( cddir, "pool", "extras" ) )

releases_file = open( os.path.join( cddir, 'dists', dist, 'extras', 'binary-i386', 'Release' ), 'w' )
releases_file.write( "Archive: " + dist + "\n" )
releases_file.write( "Version: " + dist_name_to_version[dist] + "\n" )
releases_file.write( "Component: extras\nOrigin: Ubuntu\nLabel: Ubuntu\nArchitecture: i386\n")
releases_file.close()
print "Wrote the Releases file"


old_cwd = os.getcwd()
temp_dir = tempfile.mkdtemp( prefix="/tmp/camarabuntu-tmp-", dir=old_cwd)
os.chdir( temp_dir )

#shutil.copytree( os.path.join( old_cwd, cddir, 'pool', 'main', 'u', 'ubuntu-keyring' ), temp_dir )
#assert false;


# TODO different versions of ubuntu-keyring
if options.keyring is None:
    # download a new one
    print "Downloading the ubuntu-keyring..."
    cmd =  "apt-get source ubuntu-keyring" 
    msg = """An error occured when downloading the source of the ubuntu-keyring package
    This is needed to sign our new packages"""
    RunCommand(cmd, msg=msg)
else:
    if options.keyring[-1] == '/':
        # This should be a directory, if there's a / at the end, it'll break os.path.split
        options.keyring = options.keyring[0:-1]
    shutil.copytree( os.path.join( old_cwd, options.keyring ), os.path.join( temp_dir, os.path.split( options.keyring )[1] ) )

# find the file
gpg_keys_filename = glob.glob ( os.path.join( temp_dir, 'ubuntu-keyring*/keyrings/ubuntu-archive-keyring.gpg' ) )[0]
ubuntu_keyring_dir = [name for name in glob.glob( os.path.join( temp_dir, 'ubuntu-keyring*' ) ) if os.path.isdir(name)][0]

assert os.path.isfile( gpg_keys_filename )

print "Adding GPG key %s to the ubuntu-keyring" % options.gpgkey

cmd = "gpg --import < %s" % gpg_keys_filename
# if this fails someone was messing with the code before
RunCommand(cmd)

cmd = "gpg --export FBB75451 437D05B5 %s > %s" % (options.gpgkey, gpg_keys_filename)
# Invalid keys are not detected here, unfortunatly
RunCommand(cmd)

print "\n\nRebuilding the ubuntu-keyring."
os.chdir( ubuntu_keyring_dir )
# Invalid keys are not detected here, unfortunatly
cmd = "dpkg-buildpackage -rfakeroot -k%s" % options.gpgkey
msg = """Unable to rebuild the ubuntu-keyring
    Possible causes:
    (*) The GPG key you gave (%s) is invalid, check available keys with "gpg --list-keys" """ % options.gpgkey
RunCommand(cmd, msg=msg)
print "Finished Rebuilding the ubuntu-keyring.\n\n"

# Copy these files into the main component
shutil.copy( glob.glob( os.path.join( temp_dir, "ubuntu-keyring*_all.deb" ) )[0], os.path.join( old_cwd, cddir, 'pool/main/u/ubuntu-keyring/' ) )


# Clean up, remove all this crap
os.chdir( old_cwd )
shutil.rmtree( temp_dir )


## Now create the indices.
temp_dir = tempfile.mkdtemp( prefix="/tmp/camarabuntu-tmp-", dir=old_cwd )
os.chdir( temp_dir )

ftparchive_deb = open( os.path.join( temp_dir, 'apt-ftparchive-deb.conf' ), 'w' )
ftparchive_deb.write( """Dir {
  ArchiveDir "%(cddir)s";
};

TreeDefault {
  Directory "pool/";
};

BinDirectory "pool/main" {
  Packages "dists/%(dist)s/main/binary-i386/Packages";
  BinOverride "%(indices)s/override.%(dist)s.main";
  ExtraOverride "%(indices)s/override.%(dist)s.extra.main";
};

BinDirectory "pool/restricted" {
 Packages "dists/%(dist)s/restricted/binary-i386/Packages";
 BinOverride "%(indices)s/override.%(dist)s.restricted";
};

Default {
  Packages {
    Extensions ".deb";
    Compress ". gzip";
  };
};

Contents {
  Compress "gzip";
};
""" % {'cddir':cddir, 'dist':dist, 'indices':indices} )

ftparchive_deb.close()

# generate the extras override. This is converted from the perl version on
# https://help.ubuntu.com/community/InstallCDCustomization

main_packages = open( os.path.join( old_cwd, cddir, 'dists', dist, 'main/binary-i386/Packages' ) )
extra_overrides = open( os.path.join( temp_dir, "override.%s.extra.main" % dist ), 'w' )
task = None
for line in main_packages:
    line = line.rstrip("\n")
    line = line.rstrip("\r\n")
    line = line.rstrip("\r")
    if re.search("^\s", line):
        continue # to the next line
    if line == "" and task is not None:
        extra_overrides.write("%s Task %s\n" % (package, task))
        package = None
        task = None
    if line == "":
        continue
    key, value = line.split(": ", 1)
    if key == "Package":
        package = value
    if key == "Task":
        task = value


extra_overrides.close()
main_packages.close()

ftparchive_udeb = open( os.path.join( temp_dir, "apt-ftparchive-udeb.conf" ), 'w')
ftparchive_udeb.write("""Dir {
  ArchiveDir "%(cddir)s";
};

TreeDefault {
  Directory "pool/";
};

BinDirectory "pool/main" {
  Packages "dists/%(dist)s/main/debian-installer/binary-i386/Packages";
  BinOverride "%(indices)s/override.%(dist)s.main.debian-installer";
};

BinDirectory "pool/restricted" {
  Packages "dists/%(dist)s/restricted/debian-installer/binary-i386/Packages";
  BinOverride "%(indices)s/override.%(dist)s.restricted.debian-installer";
};

Default {
  Packages {
    Extensions ".udeb";
    Compress ". gzip";
  };
};

Contents {
  Compress "gzip";
};
""" % {'cddir':cddir, 'dist':dist, 'indices':indices} )
ftparchive_udeb.close()

ftparchive_extras = open( os.path.join( temp_dir, 'apt-ftparchive-extras.conf' ), 'w')
ftparchive_extras.write("""Dir {
  ArchiveDir "%(cddir)s";
};

TreeDefault {
  Directory "pool/";
};

BinDirectory "pool/extras" {
  Packages "dists/%(dist)s/extras/binary-i386/Packages";
};

Default {
  Packages {
    Extensions ".deb";
    Compress ". gzip";
  };
};

Contents {
  Compress "gzip";
};
""" % {'cddir':cddir, 'dist':dist, 'indices':indices} )
ftparchive_extras.close()

release_file = open( os.path.join( temp_dir, "release.conf" ), 'w' )
release_file.write("""
APT::FTPArchive::Release::Origin "Ubuntu";
APT::FTPArchive::Release::Label "Ubuntu";
APT::FTPArchive::Release::Suite "%(dist)s";
APT::FTPArchive::Release::Version "%(dist_number)s";
APT::FTPArchive::Release::Codename "%(dist)s";
APT::FTPArchive::Release::Architectures "i386";
APT::FTPArchive::Release::Components "main restricted extras";
APT::FTPArchive::Release::Description "Ubuntu %(dist_number)s";
""" % {'cddir':cddir, 'dist':dist, 'indices':indices, 'dist_number':dist_name_to_version[dist]} )
release_file.close()

# now build the repository

print "Generating the APT repository..."

status, output = commands.getstatusoutput("apt-ftparchive -c %(temp_dir)s/release.conf generate %(temp_dir)s/apt-ftparchive-deb.conf" % {'temp_dir':temp_dir})
if status != 0:
    print output
assert status == 0

status, output = commands.getstatusoutput("apt-ftparchive -c %(temp_dir)s/release.conf generate %(temp_dir)s/apt-ftparchive-udeb.conf" % {'temp_dir':temp_dir})
if status != 0:
    print output
assert status == 0

status, output = commands.getstatusoutput("apt-ftparchive -c %(temp_dir)s/release.conf generate %(temp_dir)s/apt-ftparchive-extras.conf"% {'temp_dir':temp_dir})
if status != 0:
    print output
assert status == 0

if os.path.isfile( os.path.join( cddir, 'dists', dist, 'Release.gpg') ):
    # Otherwise apt-ftparchive will ask to overwrite it, we want this whole process automated
    os.remove( os.path.join( cddir, 'dists', dist, 'Release.gpg' ) )
status, output = commands.getstatusoutput("apt-ftparchive -c %(temp_dir)s/release.conf release %(cddir)s/dists/%(dist)s > %(cddir)s/dists/%(dist)s/Release" % {'dist':dist, 'cddir':cddir, 'temp_dir':temp_dir})
if status != 0:
    print output
assert status == 0

cmd = "gpg --default-key \"%(gpgkey)s\" --output %(cddir)s/dists/%(dist)s/Release.gpg -ba %(cddir)s/dists/%(dist)s/Release" % {'cddir':cddir, 'dist':dist, 'gpgkey':options.gpgkey}
print cmd
RunCommand(cmd)

# Clean up
os.chdir( old_cwd )
shutil.rmtree( temp_dir )


# Find out the name of all the packages in the repository to add to the preseed file
print "Generating a list of all the packages to install"
package_names = []
for deb in debs:
    status, output = commands.getstatusoutput( "dpkg --info \"%s\"" % deb )
    assert status == 0
    for line in output.split("\n"):
        line = line.rstrip("\n")
        splits = line.split(": ", 1)
        if len(splits) != 2:
            continue
        key, value = splits
        if key == " Package":
            package_names.append(value)

# now append the d-i directive to the preseed file to install our custom packages
preseed_dir = os.path.join(options.cddir, 'preseed')
camarabuntu_preseeds = [x for x in os.listdir(preseed_dir) if x.startswith('camarabuntu')]
print repr(camarabuntu_preseeds)
for preseed_file in camarabuntu_preseeds:
    preseed_file = os.path.join(preseed_dir, preseed_file)
    print 'editing ', preseed_file 
    lines = open(preseed_file, 'r').readlines()
    newlines = []
    for line in lines:
        if line.startswith("d-i\tpkgsel/install-pattern\tstring ~t^edubuntu-standard$|~t^edubuntu-desktop$|~t^edubuntu-server"):
            print "Got to the line", line
            newlines.append( "# The following line has been automatically added by the camarabuntu scripts\n" )
            # we need to remove the trailing \n
            newlines.append( line[:-1]+"|"+"|".join( ["~n^%s$" % package_name for package_name in package_names] ) + "\n" )
        else:
            newlines.append(line)

    newlines.append( "# The following line has been automatically added by the camarabuntu scripts\n" )
    newlines.append( "d-i\tpkgsel/include\tstring " + " ".join(package_names) + "\n" )
    
    fd = open(preseed_file, "w")
    fd.write( "".join( newlines ) )
    fd.close()
    

