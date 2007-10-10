#! /usr/bin/python

import sys
from optparse import OptionParser

parser = OptionParser()

parser.add_option( "-f", "--iso-file",
                   dest="isofile", metavar="FILE", type="string", default=None,
                   help="The filename of the resultant ISO file" )

parser.add_option( "-d", "--dir", "--directory",
                   dest="dir", metavar="DIR", type="string", default=None,
                   help="The directory of the CD files" )

parser.add_option( "-n", "--name",
                   dest="name", metavar="NAME", type="string", default=None,
                   help="The CD name" )

(options, args) = parser.parse_args()


if options.isofile is None or options.dir is None or options.name is None:
    parser.print_help()
    sys.exit(1)

import commands

status, output = commands.getstatusoutput("""
mkisofs -r -V "%s" \
    -cache-inodes \
    -J -l -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot \
    -boot-load-size 4 -boot-info-table \
    -o "%s" "%s" """ % (options.name, options.isofile, options.dir ) )

if status != 0:
    print output
