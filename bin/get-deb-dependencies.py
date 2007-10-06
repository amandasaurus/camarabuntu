#! /usr/bin/python

from optparse import OptionParser
import os, re, commands, copy, urllib, gzip, tempfile

parser = OptionParser()

parser.add_option( "-r", "--repo", "--repository",
                   dest="repos", action="append", help="Repository path", default=[] )

parser.add_option( "-w", "--web-repo", "--web-repository", "--remote-repo", "--remote-repository",
                   dest="webrepo", action="append", help="URL of web repository", default=[] )


(options, debs) = parser.parse_args()

assert len(debs) == 1, "You must provide exactly one deb on the command line"
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

    def __eq__(self, other):
        return self.name == other.name and self.version == other.version and self.relation == other.version

    def __hash__(self):
        return hash((self.name, self.version, self.relation))

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
            bits = line.split(": ", 1)
            if len(bits) != 2:
                continue
            key, value = bits
            if key == "Depends":
                self.parse_dependencies( value )
            if key == "Version":
                self.version_string = value


    def __str__(self):
        return self.name

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

    def fulfils(self, dep):
        """Returns true iff this package can satify the dependency dep"""
        if dep.name != self.name:
            # Obviously
            return False

        if dep.version is None:
            # we don't care about version
            return True

        # version code from here:
        # http://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-Version
        version_re = re.compile("""((?P<epoch>\d+):)?(?P<upstream>[-0-9.+:]+)(-(?P<debian>[a-zA-Z0-9+.]+)?(ubuntu(?P<ubuntu>[a-zA-Z0-9+.]+))?)?""" )

        match = version_re.match( self.version_string )
        assert match is not None, "The version string for %s (%s) does not match the version regular expression" % (self.name, self.version_string)

    def unfulfilled_depenencies(self, local_repos, remote_repos):
        assert isinstance(self.depends, AndDependencyList)

        # Start off with all our depends. We have to keep going until we this
        # is empty.
        unfulfiled = set([dep for dep in self.depends])
        fulfiled_deps = set()

        while len(unfulfiled) > 0:
            dependency = unfulfiled.pop()

            fulfiled_locally = False

            for repo in local_repos:
                if fulfiled_locally:
                    break
                for package in repo.packages:
                    if fulfiled_locally:
                        break
                    if isinstance( dependency, OrDependencyList ):
                        if any([package.fulfils(dep) for dep in dependency] ):
                            #print "package %s fulfils the dependency %s, which is part of %s" % (package, dep, dependency)
                            fulfiled_locally = True
                            break
                    else:
                        if package.fulfils(dependency):
                            #print "package %s fulfils the dependency %s" % (package, dependency)
                            fulfiled_locally = True
                            break

            if fulfiled_locally:
                # Don't bother looking at the remote repositories if we can already fulfill locally
                print "The dependency %s can be fulfilled locally" % dependency
                continue # to the next package

            # we know we need to look on the web for this dependency

            fulfiled_remotely = False
            remote_repo = None
            for repo in remote_repos:
                if fulfiled_remotely:
                    break
                for package in repo.packages:
                    if fulfiled_remotely:
                        break
                    if isinstance( dependency, OrDependencyList ):
                        if any([packages.fulfils(dep) for dep in dependency] ):
                            fulfiled_remotely = True
                            remote_repo = repo
                            break
                    else:
                        if package.fulfils(dependency):
                            fulfiled_remotely = True
                            remote_repo = repo
                            break
            
            if fulfiled_remotely:
                print "The dependency %s can be fulfilled from the %s repository" % (dependency, remote_repo)

            if not fulfiled_remotely and not fulfiled_locally:
                print "Warning the dependency %s cannot be fulfilled remotely or locally. Try adding extra repositories" % dependency


class DependencyList():
    def __init__(self, *dependencies):
        self.dependencies = dependencies

    def __iter__(self):
        return self.dependencies.__iter__()
                


class AndDependencyList(DependencyList):
    def __str__(self):
        return ", ".join( [ str(dep) for dep in self.dependencies ] )

    def __repr__(self):
        return "AndDependencyList( %s )" % ", ".join( [ repr(dep) for dep in self.dependencies ] )

class OrDependencyList(DependencyList):
    def __str__(self):
        return " | ".join( [ str(dep) for dep in self.dependencies ] )

    def __repr__(self):
        return "OrDependencyList( %s )" % ", ".join( [ repr(dep) for dep in self.dependencies ] )


class Repository():
    def __init__(self, uri):
        self.packages = []
        if os.path.isdir( uri ):
            self.path = os.path.abspath( uri )
            self.__scan_local_packages()
        elif uri[0:7] == "http://":
            self.url = uri
            self.__scan_remote_packages()


    def __scan_remote_packages(self):
        # check for the repo file
        tmpfile_fp, tmpfile = tempfile.mkstemp(suffix=".gz", prefix="web-repo-")
        urllib.urlretrieve( self.url+ "/binary-i386/Packages.gz", filename=tmpfile )
        #releases_file = gzip.GzipFile(filename=None, fileobj=urllib.urlopen( "%s/binary-i386/Packages.gz" % self.url ) )
        self.__scan_packages( gzip.open( tmpfile ) )
    
    def __scan_packages(self, releases_fp):
        package = Package()
        self.packages = []
        for line in releases_fp:
            line = line.rstrip("\n")
            if line == "":
                self.packages.append(package)
                package = Package()
                continue
            if line[0] == " ":
                continue
            bits = line.split(": ", 1)
            if len(bits) != 2:
                continue
            key, value = bits
            maps = [ [ 'Package', 'name' ],
                     [ 'Version', 'version_string' ],
                     ]
            for key_name, attr in maps:
                if key == key_name:
                    setattr( package, attr, value )
            if key == 'Depends':
                package.parse_dependencies( value )


    def __scan_local_packages(self):
        package = Package()
        self.packages = []
        for line in open( "%s/binary-i386/Packages" % self.path ):
            line = line.rstrip("\n")
            if line == "":
                self.packages.append(package)
                package = Package()
                continue
            if line[0] == " ":
                continue
            key, value = line.split( ": ", 1 )
            maps = [ [ 'Package', 'name' ],
                     [ 'Version', 'version_string' ],
                     ]
            for key_name, attr in maps:
                if key == key_name:
                    setattr( package, attr, value )
            if key == 'Depends':
                package.parse_dependencies( value )

    def __contains__(self, package):
        if isinstance(package, str):
            # looking for package name
            pass
        elif isinstance(package, Package):
            # looking for an actual package
            return package in self.dependencies
        elif isinstance( package, Dependency ):
            # looking for a version
            possibilities = [ dep for dep in self.dependencies if deb.name == package.name ]
            print repr( possibilities )
        

repos = [Repository(r) for r in options.repos]

remote_repos = [Repository(r) for r in options.webrepo]

package = Package( filename=deb )

print "Checking local repos..."
package.check_for_depenencies(repos)

print "Checking remote..."
package.check_for_depenencies(remote_repos)
        
