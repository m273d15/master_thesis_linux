#!/usr/bin/perl

use Plan;
use Template;

my $tmpl_dir = './tmpl';
my $output_dir = './mods';
my $plan_dir = './plans';
my $make_tt = "$tmpl_dir/Makefile.tt";
my $module_tt = "$tmpl_dir/module.tt";

my $linux_src = '/home/kelvin/git/master_thesis_linux/';
my $tt = Template->new({
    OUTPUT_PATH => $output_dir,
    RELATIVE => 1
});

print("Process plans:\n");
my @modules;

if (-d $output_dir)
{
    system("rm -r $output_dir");
    mkdir($output_dir) || die "Failed to create $output_dir";
}

foreach (glob( $plan_dir . '/*.csv' ))
{
    if ($_ =~ /^.*\/(.*)\.csv$/)
    {
        process_plan($_, $1);
        push(@modules, $1);
    }
}

process_makefile(@modules);

sub process_plan
{
    my ($plan_file, $module_name) = @_;

    print("Process $plan_file\n");

    my @plan = parse_plan_csv ($plan_file); 
    my @formatted_plan;
    
    for (my $i = 0; $i < scalar @plan ; $i += 2) {
        if ($plan[$i]->{time} >= 20000000000)
        {
            die "ERROR: Watchdog will cause kernel panic!\n";
        }
        push(@formatted_plan, {
            exec_t => $plan[$i]->{time},
            idle_t => $plan[$i+1]->{time},
            index => $i/2
        });
    }

    my $size = scalar @formatted_plan;
    my $vars = {
        plan_size => "$size",
        plan => \@formatted_plan,
    };
    $tt->process($module_tt, $vars, "$module_name.c");
}

sub process_makefile
{
    my (@modules) = @_;
    print("Process Makefile\n");

    my $vars = {
        linux_src_dir => $linux_src,
        module_names => \@modules,
    };
    $tt->process($make_tt, $vars, 'Makefile') || die $tt->error();
}
