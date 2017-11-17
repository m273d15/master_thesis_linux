#!/usr/bin/perl

use Plan;

my $argc = @ARGV;

my $dmesg_log = $ARGV[0];

my $last_time = 0;
my $last_nr = 0;
my @results;

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
        push(@results, { mode => $mode, nr => $last_nr, time => $result });
    }
}
close (FILE);

if ( $argc > 1 )
{
    my $plan_csv = $ARGV[1];
    print_compared_result ($plan_csv, @results);
}
else
{
    print_simple_result (@results);
}

sub print_simple_result
{
    my (@results) = @_;

    foreach (@results)
    {
        print ($_->{mode}.'['.$_->{nr}.']'.': '.$_->{time}."\n");
    }
}

sub print_compared_result
{
    my ($plan_csv_file, @results) = @_;
    my @expected_results = parse_plan_csv($plan_csv_file);

    push(@results, {mode => 'IDLE', nr => $results[-1]{nr}, time => 0});

    for (my $i=0; $i <= $#results; $i++)
    {
        my $expected = $expected_results[$i]->{time};
        my $actual = $results[$i]->{time};
        my $diff = abs($expected - $actual);

        print ($results[$i]->{mode}.'['.$results[$i]->{nr}.']'.' actual: '.$actual.', expected: '.$expected.', diff: '.$diff."\n");
    }
}
