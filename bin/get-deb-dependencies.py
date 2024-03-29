#! /usr/bin/python

from optparse import OptionParser
#import os, re, commands, copy, urllib, gzip, tempfile
import os

from apt import Dependency, Repository, Package, AndDependencyList, OrDependencyList
import apt

parser = OptionParser()

parser.add_option( "-l", "--local-repo", "--local-repository",
                   dest="local_repos", action="append", help="Repository path", default=[] )

parser.add_option( "-r", "--remote-repo", "--remote-repository", "--remote-repo", "--remote-repository",
                   dest="remote_repos", action="append", help="URL of web repository", default=[] )

parser.add_option( "-d", "--directory",
                   dest="directory", help="The directory to store the downloaded deb files. If not specified, the debs are downloaded into the current directory", default=None)


(options, debs) = parser.parse_args()

assert len(options.local_repos) > 0, "No local repositories provided"

local_repos = [Repository(r) for r in options.local_repos]

print "Downloading remote repositories... "
remote_repos = [Repository(r) for r in options.remote_repos]
print "done"

if options.directory is None:
    directory = os.getcwd()
else:
    if not os.path.isdir( options.directory ):
        print "Error: '%s' is not a directory." % options.directory
        os.exit(1)
    else:
        directory = options.directory

apt.dl_depenencies( debs, local_repos, remote_repos, directory )

#package.unfulfilled_depenencies(repos, remote_repos)

#print repr([r['gstreamer0.10-ffmpeg'] for r in remote_repos])
#remote_repos[1]['gstreamer0.10-ffmpeg'].save()

