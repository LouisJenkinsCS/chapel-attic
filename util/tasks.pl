#!/usr/bin/env perl

use File::Basename;

$utildirname = dirname($0);

$preset_tasks   = $ENV{'CHPL_TASKS'};
$preset_threads = $ENV{'CHPL_THREADS'};

if ($preset_tasks eq "") {
    if ($preset_threads eq "pthreads") {
	$tasks = "fifo";
    } elsif ($preset_threads eq "dev-threads-1") {
	$tasks = "dev-tasks-1";
    } elsif ($preset_threads eq "") {
	$platform = `$utildirname/platform.pl --target`;
	chomp($platform);
	if ($platform =~ /^(mta|xmt(-sim)?)$/) {
	    $tasks = "mta";
	} elsif ($platform =~ /^x[12]-sim$/) {
	    $tasks = "none";
	} else {
	    $tasks = "fifo";
	}
    } else {
	die "Unexpected CHPL_THREADS value \"$preset_threads\".\n";
    }
} else {
    $tasks = $preset_tasks;
}

if ($preset_threads ne "") {
  system("$utildirname/check-tasks-threads-compatibility.pl",
	 "$tasks",
	 "$preset_threads") == 0 or exit(1);
}

print "$tasks\n";
exit(0);
