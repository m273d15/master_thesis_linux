#!/usr/bin/perl

use Plan;

my @plan = parse_plan_csv($ARGV[0]);

foreach (@plan)
{
    my $field = lc($_->{mode}).'_t';
    my $nr = $_->{nr};
    my $time = $_->{time};

    print("    pb_rq->plan[$nr].$field = $time;\n");
}
