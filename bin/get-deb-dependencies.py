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
            depends_re = re.compile( r"(?P<name>\S*) \((?P<relation><<|<=|=|>=|>>) (?P<version>\S*)\)" )
            result = depends_re.search( string )
            if result is None:
                self.name = string
                self.version = None
                self.relation = None
            else:
                self.name = result.group('name')
                self.version = result.group('version')
                self.relation = result.group('relation')
        else:
            self.name = name
            self.version = version
            self.relation = relation

    def __str__(self):
        if self.relation is None and self.version is None:
            return self.name
        else:
            return "%s (%s %s)" % (self.name, self.relation, self.version)

    def __repr__(self):
        return "Dependency( name=%r, version=%r, relation=%r )" % (self.name, self.version, self.relation )

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


    def __str__(self):
        #print repr(self.dependencies)
        return " | ".join( [ str(dep) for dep in self.dependencies ] )

    def parse_dependencies(self, depends_line):
        depends = []

        # Assuming that "," is more important than "|"
        # and that we ca only nest them 2 deep at most
        packages = depends_line.split( "," )
        for package in packages:
            package = package.strip()
            alts = [ s.strip() for s in package.split("|") ]
            if len(alts) == 1:
                # just a normal package line
                depends.append( Dependency( string = package ) )
            else:
                depends.append( OrDependencyList( *[ Dependency(string=alt) for alt in alts ] ) )

        self.depends = AndDependencyList( *depends )

    def fulfills(self, dep):
        """Returns true iff this package can satify the dependency dep"""
        if dep.name != self.name:
            return False

        pass
    

class DependencyList():
    def __init__(self):
        pass

class AndDependencyList(DependencyList):
    def __init__(self, *dependencies):
        self.dependencies = dependencies

    def __str__(self):
        return ", ".join( [ str(dep) for dep in self.dependencies ] )

    def __repr__(self):
        return "AndDependencyList( %s )" % ", ".join( [ repr(dep) for dep in self.dependencies ] )

class OrDependencyList(DependencyList):
    def __init__(self, *dependencies):
        self.dependencies = dependencies

    def __str__(self):
        #print repr(self.dependencies)
        return " | ".join( [ str(dep) for dep in self.dependencies ] )

    def __repr__(self):
        return "OrDependencyList( %s )" % ", ".join( [ repr(dep) for dep in self.dependencies ] )


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

    def __contains__(self, package):
        if isinstance(package, str):
            # looking for package name
        elif isinstance(package, Package):
            # looking for an actual package
            return package in self.dependencies
        elif isinstance( package, Dependency ):
            # looking for a version
            possibilities = [ dep for dep in self.dependencies if deb.name = package.name ]
            print repr( possibilities )
        


for repo in options.repos:
    r = Repository(repo)

package = Package( filename=deb )
print repr(package.depends)
print str(package.depends)


        
