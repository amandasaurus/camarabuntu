#! /usr/bin/python

from optparse import OptionParser
import os

parser = OptionParser()

parser.add_option("-c", "--cd-dir",
                  dest="cddir",
                  help="The directory of the install cd details")

(options, debs) = parser.parse_args()

cddir = options.cddir

# see what the name of the distro is, (eg dapper, edgy.. )
dists_dir = [ dir for dir in os.listdir( os.path.join ( cddir, 'dists' ) ) if dir not in ['stable', 'unstable' ] ]

assert len(dists_dir) == 1
dist = dists_dir[0]

def makedir_if_not_exist( *path_parts ):
    path = os.path.join( *path_parts )
    if not os.path.exists( path ):
        os.makedirs( path )

makedir_if_not_exist( cddir, 'dists', dist, 'extras', 'binary-i386' )
makedir_if_not_exist( cddir, 'pool', 'extras' )
makedir_if_not_exist( cddir, 'isolinux' )
makedir_if_not_exist( cddir, 'preseed' )

print dist
