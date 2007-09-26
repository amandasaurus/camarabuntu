#!/usr/bin/perl -w 

my $controlfile = "debian/control";

open(CDATA,"< $controlfile") || &error(sprintf("cannot read control file %s: %s", $controlfile, $!));
binmode(CDATA);
my $indices = &parsecdata('C',1,sprintf("control file %s",$controlfile));
$indices >= 2 || &error("control file must have at least one binary package part");

sub error { die sprintf("%s: error: %s", $progname, $_[0])."\n"; }

sub parsecdata {
    local ($source,$many,$whatmsg) = @_;
    # many=0: ordinary control data like output from dpkg-parsechangelog
    # many=1: many paragraphs like in source control file
    # many=-1: single paragraph of control data optionally signed
    local ($index,$cf,$paraborder);
    $index=''; $cf=''; $paraborder=1;
    while (<CDATA>) {
        s/\s*\n$//;
        next if (m/^$/ and $paraborder);
        next if (m/^#/);
        $paraborder=0;
        if (m/^(\S+)\s*:\s*(.*)$/) {
            $cf=$1; $v=$2;
            $cf= &capit($cf);
            $fi{"$source$index $cf"}= $v;
            $fi{"o:$source$index $cf"}= $1;
            if (lc $cf eq 'package') { $p2i{"$source $v"}= $index; }
        } elsif (m/^\s+\S/) {
            length($cf) || &syntax("continued value line not in field");
            $fi{"$source$index $cf"}.= "\n$_";
        } elsif (m/^-----BEGIN PGP/ && $many<0) {
            $many == -2 && syntax("expected blank line before PGP signature");
            while (<CDATA>) { last if m/^$/; }
            $many= -2;
        } elsif (m/^$/) {
            $paraborder = 1;
            if ($many>0) {
                $index++; $cf='';
            } elsif ($many == -2) {
                $_= <CDATA> while defined($_) && $_ =~ /^\s*$/;
                length($_) ||
                    &syntax("expected PGP signature, found EOF after blank line");
                s/\n$//;
                m/^-----BEGIN PGP/ ||
                    &syntax(sprintf("expected PGP signature, found something else \`%s'"), $_);
                $many= -3; last;
            } else {
                while (<CDATA>) {
                    /^\s*$/ ||
                        &syntax("found several \`paragraphs' where only one expected");
                }
            }
        } else {
            &syntax("line with unknown format (not field-colon-value)");
        }
    }
    $many == -2 && &syntax("found start of PGP body but no signature");
    if (length($cf)) { $index++; }
    $index || &syntax("empty file");
    return $index;
}

sub capit {
    my @pieces = map { ucfirst(lc) } split /-/, $_[0];
    return join '-', @pieces;
}
