#! /usr/bin/python

from optparse import OptionParser
import os

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository",
                   dest="repos", action="append", help="Repository path", default=[] )


(options, debs) = parser.parse_args()

assert len(debs) == 1
deb = debs[0]

assert len(options.repos) > 0

class Package():
    def __init__(self):
        self.name = None
        self.version = None
        pass

    def __repr__(self):
        return "Package: name: %s version: %s" % (self.name, self.version)

class Repository():
    def __init__(self, path):
        self.path = os.path.abspath(path)
        self.packages = []
        self.__scan_packages()

    def __scan_packages(self):
        package = Package()
        for line in open( "%s/binary-i386/Packages" % self.path ):
            line = line.rstrip("\n")
            if line == "":
                print repr(package)
                self.packages.append(package)
                packages = Package()
                continue
            if line[0] == " ":
                continue
            key, value = line.split( ": ", 1 )
            maps = [ [ 'Package', 'name' ],
                     [ 'Version', 'version' ],
                     [ 'Depends', 'depends' ] ]
            for key_name, attr in maps:
                if key == key_name:
                    setattr( package, attr, value )







for repo in options.repos:
    r = Repository(repo)
        
