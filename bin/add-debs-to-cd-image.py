#! /usr/bin/python

from optparse import OptionParser
import os, shutil

parser = OptionParser()

parser.add_option("-d", "--cd-dir",
                  dest="cddir",
                  help="The directory of the install cd details")

(options, debs) = parser.parse_args()

cddir = options.cddir

# see what the name of the distro is, (eg dapper, edgy.. )
dists_dir = [ dir for dir in os.listdir( os.path.join ( cddir, 'dists' ) ) if dir not in ['stable', 'unstable' ] ]

assert len(dists_dir) == 1
dist = dists_dir[0]

dist_name_to_version = { 'warty':'4.10', 'hoary':'5.04', 'breezy':'5.10',
                         'dapper':'6.06', 'edgy':'6.10', 'feisty':'7.04',
                         'gusty':'7.10', 'hardy':'8.04' }

assert dist in dist_name_to_version.keys()

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
