#!/usr/bin/perl -w

use warnings;
use strict;
use IO::Socket::INET;
use IO::File;

$SIG{'CHLD'} = 'IGNORE';


my $port = shift;
die "port must be 1024..65535" unless defined ($port) && $port >= 1024 && $port <= 65535;
my $blocksize = 65536;
if ($ARGV[0] =~ /^\d+$/) {
  $blocksize = $ARGV[0];
  shift;
}

my $listen = new IO::Socket::INET (LocalPort => $port, Proto => "tcp", Listen => 5,
       ReuseAddr => 1) or die "cannot listen $!";
warn "listening on $port";

sub sendfiles {
  my $s = shift;
  while (my $f = shift) {
     my $fh = new IO::File ("<$f") or next;
     my $buffer;
     while (my $bytes = $fh->sysread ($buffer, $blocksize)) {
        $s->syswrite ($buffer, $bytes) or die "cannot write $!";
	sleep (1) if $blocksize < 128;
     }
  }
}

while (my $s = $listen->accept ()) {
  my $pid = fork ();
  die "fork $!" unless defined ($pid);
  if ($pid == 0) {
     warn "connection accepted";
     $listen->close ();
     sendfiles ($s, @ARGV);
     exit (0);
  } else {
     $s->close ();
  }
}
