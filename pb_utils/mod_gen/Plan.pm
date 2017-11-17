package Plan;

use Exporter 'import';

our @EXPORT = qw ( parse_plan_csv );

sub parse_plan_csv
{
    my ($plan_csv_file) = @_;
    my @expected_results;
    my $counter = 0;

    open (FILE, $plan_csv_file);

    while (<FILE>)
    {
        if (/\s*(\d+)\s*,\s*(\d+)\s*/)
        {
            push(@expected_results, {mode => 'EXEC', nr => $counter, time => $1});
            push(@expected_results, {mode => 'IDLE', nr => $counter, time => $2});
            $counter++;
        }
    }

    close (FILE); 

    return @expected_results;
}
