#! /usr/bin/python

from optparse import OptionParser
import os, re, commands

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository",
                   dest="repos", action="append", help="Repository path", default=[] )


(options, debs) = parser.parse_args()

assert len(debs) == 1
deb = debs[0]

assert len(options.repos) > 0

class Dependency():
    def __init__(self, string=None, package_name=None, version=None, relation=None):
        if string is not None:
            depends_re = re.compile( r"(?P<package_name>\S*) \((?P<relation><<|<=|=|>=|>>) (?P<version>\S*)\)" )
            result = depends_re.search( string )
            if result is None:
                self.package_name = string
                self.version = None
                self.relation = None
            else:
                self.package_name = result.group('package_name')
                self.version = result.group('version')
                self.relation = result.group('relation')
        else:
            self.package_name = package_name
            self.version = version
            self.relation = relation

    def __str__(self):
        if self.relation is None and self.version is None:
            return self.package_name
        else:
            return "%s (%s %s)" % (self.package_name, self.relation, self.version)

    def __repr__(self):
        return "Dependency( package_name=%r, version=%r, relation=%r )" % (self.package_name, self.version, self.relation )

class Package():
    def __init__(self, filename=None):
        self.name = None
        self.version = None
        self.depends_line = None

        if filename is not None:
            self.__init_from_deb_file(os.path.abspath(filename))

    def __init_from_deb_file(self, filename):
        assert os.path.isfile(filename)

        status, output = commands.getstatusoutput("dpkg-deb --info \"%s\"" % filename)
        if status != 0:
            print output
            return

        for line in output.split("\n"):
            # remove the leading space
            line = line[1:]
            line = line.rstrip()
            if line[0] == " ":
                continue
            if line.startswith("Depends: "):
                key, value = line.split( ": ", 1 )
                self.parse_dependencies( value )


    def parse_dependencies(self, depends_line):
        self.depends = self.parse_dependencies_rec( depends_line )


    def parse_dependencies_rec( self,  depends_line ):
        depends = []

        # Assuming that "," is more important than "|"
        packages = depends_line.split( "," )
        for package in packages:
            package = package.strip()
            depends.append( Dependency( string = package ) )
        
        return depends


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

package = Package( filename=deb )
print repr(package.depends)
print ", ".join([str(dep) for dep in package.depends])


        
