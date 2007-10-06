#! /usr/bin/python

from optparse import OptionParser
#import os, re, commands, copy, urllib, gzip, tempfile

from apt import Dependency, Repository, Package, AndDependencyList, OrDependencyList

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository",
                   dest="repos", action="append", help="Repository path", default=[] )

parser.add_option( "-w", "--web-repo", "--web-repository", "--remote-repo", "--remote-repository",
                   dest="webrepo", action="append", help="URL of web repository", default=[] )


(options, debs) = parser.parse_args()

#assert len(debs) == 1, "You must provide exactly one deb on the command line"
#deb = debs[0]

assert len(options.repos) > 0, "No local repositorys provided"

repos = [Repository(r) for r in options.repos]

print "Downloading remote repositories... "
remote_repos = [Repository(r) for r in options.webrepo]
print "done"

#package = Package( filename=deb )

#package.unfulfilled_depenencies(repos, remote_repos)

#print repr([r['gstreamer0.10-ffmpeg'] for r in remote_repos])
remote_repos[1]['gstreamer0.10-ffmpeg'].save()

