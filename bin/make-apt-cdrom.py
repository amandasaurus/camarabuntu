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


(options, debs) = parser.parse_args()

