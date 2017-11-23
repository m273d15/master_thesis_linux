#!/usr/bin/perl
my $mod_dir = './mod_gen/mods';
my $plan_dir = './plans';
my $main_log = 'test_results';
my $exclude_file = 'exclude_tests.txt';
my $plan_f_ext = '.csv';
my $tmp_log = 'log.tmp';

my @modules = glob( $mod_dir . '/*.ko' );
my @excludes;

if (-e $main_log)
{
    unlink($main_log) or die "Failed to remove main log: $main_log\n";
}

if (-e $exclude_file)
{
    open(my $efh, '<', $exclude_file);
    chomp(@lines = <$efh>);
    close($efh);
}

open(my $fh, '>>', $main_log) or die "Failed to open main log: $main_log\n";
foreach (@modules)
{
    my $regex_exc = join('|', @excludes);

    unless ($regex_exc != '' && /^.*\/(\Q$regex_exc)\.ko$/)
    {
        my $results = execute_test($_);
        say $fh "\nModule: $_\n$results";
    }
    else
    {
        print("Excluded: $_");
    }
}
close($fh);

print("Results are stored in $main_log\n");

sub execute_test
{
    my ($module_path) = @_;
    my $result;
    print("Starting $module_path\n");
    clear_dmesg();
    start_test($module_path) or die "Failed to start module $module_path\n";

    until (execution_done())
    {
        sleep(2);
    }
    sleep(1);
    $result = get_results(get_plan_path($module_path));
    sleep(1);
    finish_test($module_path) or die "Failed to stop module $module_path\n";
    print("Finished $module_path\n");
    sleep(3);
    return $result;
}

sub get_results
{
    my ($plan_path) = @_;

    if (-e $tmp_log)
    {
        unlink($tmp_log);
    }
    system('dmesg > ' . $tmp_log);

    chdir('./mod_gen');
    my $outp = `./parse_dmesg.pl ../$tmp_log $plan_path`;
    chdir('../');

    return $outp;
}

sub get_plan_path
{
    my ($module_path) = @_;
    my $plan_path;
    if ($module_path =~ /^.*\/(.+)\.ko$/)
    {
        $plan_path = $plan_dir . '/' . $1 . $plan_f_ext;
    }
    return $plan_path;
}

sub start_test
{
    my ($module_path) = @_;
    my $rc = system("insmod $module_path");
    return $rc == 0;
}

sub finish_test
{
    my ($module_path) = @_;
    my $rc = system("rmmod $module_path");
    return $rc == 0;
}

sub execution_done
{
    my $rc = system('dmesg | grep "PLAN DONE"');
    return $rc == 0;
}

sub clear_dmesg
{
    system('dmesg -C');
}
