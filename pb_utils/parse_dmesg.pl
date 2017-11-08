#!/usr/bin/perl

my $dmesg_log = $ARGV[0];
my $last_time = 0;
my $last_nr = 0;

open (FILE, $dmesg_log);

while (<FILE>)
{
    unless (/^.*(EXEC|IDLE),(START|STOP),\d*,\d*.*/)
    {
        next;
    }
    chomp;

    ($dirty_mode, $state, $entry, $time) = split(/,/);

    $mode = "";
    if ($dirty_mode =~ /.*(EXEC|IDLE).*/)
    {
        $mode = $1;
    }

    if ( $state eq "START" )
    {
        $last_time = $time;
        $last_nr = $entry;
    }

    if ( $state eq "STOP" )
    {
        $result = $time - $last_time;
        print("${mode}[$last_nr]: ${result}us\n");
    }
}
close (FILE);
exit;
