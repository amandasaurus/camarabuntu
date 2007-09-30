#! /usr/bin/python

from optparse import OptionParser
import os, re

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository",
                   dest="repos", action="append", help="Repository path", default=[] )


(options, debs) = parser.parse_args()

assert len(debs) == 1
deb = debs[0]

assert len(options.repos) > 0

class Dependency():
    def __init__(self, package_name, version, relation):
        self.package_name = package_name
        self.version = version
        self.relation = relation

    def __str__(self):
        return "%s (%s %s)" % (self.package_name, self.relation, self.version)

    def __repr__(self):
        return "Dependency( %r, %r, %r )" % (self.package_name, self.version, self.relation )

class Package():
    def __init__(self):
        self.name = None
        self.version = None
        self.depends_line = None
        pass

    def parse_dependencies(self, depends_line):
        self.depends = []

        packages = depends_line.split( "," )
        depends_re = re.compile( r"(?P<package_name>\S*) \((?P<relation><<|<=|=|>=|>>) (?P<version>\S*)\)" )
        for package in packages:
            package = package.strip()
            result = depends_re.search(package)
            if result is None:
                # plain old depend
                self.depends.append( package )
            else:
                self.depends.append( Dependency( result.group('package_name'), result.group('version'), result.group('relation') ) )

        print repr( self.depends )


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
                self.packages.append(package)
                packages = Package()
                continue
            if line[0] == " ":
                continue
            key, value = line.split( ": ", 1 )
            maps = [ [ 'Package', 'name' ],
                     [ 'Version', 'version' ],
                     ]
            for key_name, attr in maps:
                if key == key_name:
                    setattr( package, attr, value )
            if key == 'Depends':
                package.parse_dependencies( value )







for repo in options.repos:
    r = Repository(repo)
        
