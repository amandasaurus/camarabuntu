#! /usr/bin/python

from optparse import OptionParser
#import os, re, commands, copy, urllib, gzip, tempfile
import os

from apt import Dependency, Repository, Package, AndDependencyList, OrDependencyList
import apt

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository", "--local-repos", "--local-repositories",
                   dest="local_repos", action="append", help="Repository path", default=[] )

parser.add_option( "-w", "--web-repo", "--web-repository", "--remote-repo", "--remote-repository",
                   dest="remote_repos", action="append", help="URL of web repository", default=[] )


(options, debs) = parser.parse_args()

assert len(debs) == 1, "You must provide exactly one deb or package name on the command line"
deb = debs[0]

assert len(options.local_repos) > 0, "No local repositorys provided"

local_repos = [Repository(r) for r in options.local_repos]

print "Downloading remote repositories... "
remote_repos = [Repository(r) for r in options.remote_repos]
print "done"

def get_deb_depenencies(self, package):
    global local_repos, remote_repos
    package.unfulfilled_depenencies(local_repos, remote_repos)   
    

def dl_deb(package_name):
    global local_repos, remote_repos
    if any([r[package_name] is not None for r in local_repos]):
        print "Package %s is already in the local repositories"
        return

    available_repos = [r for r in remote_repos if r[package_name] is not None]
    assert len(available_repos) == 1, "There should be only one repo that provides "+package_name

    remote_package = available_repos[0][package_name]
    remote_package.save()
    
    # Now we look for the dependencies
    
    return


apt.dl_depenencies( deb, local_repos, remote_repos )

#package.unfulfilled_depenencies(repos, remote_repos)

#print repr([r['gstreamer0.10-ffmpeg'] for r in remote_repos])
#remote_repos[1]['gstreamer0.10-ffmpeg'].save()

