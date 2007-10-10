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


(options, debs) = parser.parse_args()

assert len(options.local_repos) > 0, "No local repositorys provided"

local_repos = [Repository(r) for r in options.local_repos]

print "Downloading remote repositories... "
remote_repos = [Repository(r) for r in options.remote_repos]
print "done"

apt.dl_depenencies( debs, local_repos, remote_repos )

#package.unfulfilled_depenencies(repos, remote_repos)

#print repr([r['gstreamer0.10-ffmpeg'] for r in remote_repos])
#remote_repos[1]['gstreamer0.10-ffmpeg'].save()

