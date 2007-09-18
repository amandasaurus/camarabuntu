#! /usr/bin/python

import optparse

parser = OptionParser()
parser.add_option("-r", "--releases", dest="releases",
                  help="the filename where the releases file is", metavar="FILENAME")
parser.add_option("-c", "--cd-dir",
                  dest="cddir"
                  help="The directory of the install cd")

(options, args) = parser.parse_args()


print options.releases
print options.cddir
