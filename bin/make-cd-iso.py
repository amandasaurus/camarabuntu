#! /usr/bin/python

from optparse import OptionParser

parser = OptionParser()

parser.add_option( "-f", "--iso-file",
                   dest="isofile", metavar="FILE", type="string",
                   help="The filename of the resultant ISO file" )

parser.add_option( "-d", "--dir", "--directory",
                   dest="dir", metavar="DIR", type="string",
                   help="The directory of the CD files" )

parser.add_option( "-n", "--name",
                   dest="name", metavar="NAME", type="string",
                   help="The CD name" )

(options, args) = parser.parse_args()


