#!/usr/bin/perl
#
# $Id: run_tests.pl 159 2014-07-09 09:44:11Z roca $
#
# Run openfec simulations iteratively, according to the configuration specified in the "params.txt"
# file (or whatever file name) given in parameter.
# This tool populates the SQL database (either MySQL or SQLite) with the simulation results.
# This is the first step, the second step being the generate_curve.pl script that extracts results
# from the SQL database, calculates statistics, and generates the curves.
#
use strict;
use threads;
use threads::shared;
use Thread::Semaphore;
use DBI;
use DBD::SQLite;
use POSIX qw(ceil floor);

#deleted
#$SIG{INT} = 'interruption_handler'; #define subroutine to use when SIGINT is received (ctrl+C)

#final trace file. It contains at the end of tests, all eperf_tool commands with results.
my $trace_file;

#binary semaphore to protect write access to @tab_cmd
my $semaphore:shared;
# binary semaphore to protect write access into the sqlite file database (if sqlite is used).
my $semaphore_sqlite:shared;
#contains list of eperf_tool commands to exectute
my @tab_cmd:shared;
#used by differents threads for collecting into database.
my @connection_string:shared;
#used by threads to stock their progress state
my @thread_progress_state:shared;
my $current_test_number:shared;
#WRITINGTIME : used to store threads writing time
# You can activate writing time display by uncomment lines after "WRITINGTIME" occurences.
# WARNING : you can use this display with mono-thread execution only !
###my @thread_writing_time:shared;
###my $total_writing_time:shared;

#not buffering
$| = 1;

#path to the parameters file given in argument
my $file_params=shift;
#check if argument is OK
usage() unless $file_params;

my $start = time;
main();
my $end = time;
my $cpt=$end-$start;

print "\nElapsed time: $cpt s.\n";

#WRITINGTIME : display of writing time
###print "Including writing time : $total_writing_time s.\n\a";

sub main
{
	print "\e[0m";
	system("clear");
	print "\e[42m\e[K\e[1m\t\t\t\t\t\t\e[4mRUN TESTS\e[0m\n\n";
	#contains for each tests the eperf_tool command its arguments
	my $tool_with_args;
	#we save all parameters for this script in a hash.
	my %params;
	#init differents params before reading file parameters.
	#  init_params(\%params); → obsolète
	# read file parameters and fill the hash. it erases params set in init_params.
	read_params($file_params,\%params);
	# read parameters given in the command line
	$params{"verbose"} = 0; #initialize verbose mode to off
	$params{"force"} = 0; #initialize force mode to off
	$params{"nb_thread"} = 2*get_nb_cpu(); #initialize thread number
	read_inline_params(\%params);
	#check parameters validity
	check_params(\%params);
	
	#the database handle
	my $dbh;
	
	# all params used for tests
	my $database_type=  @{$params{"database"}}[0];
	my $erase_database = @{$params{"erase_database"}}[0];
	my $tool=@{$params{"fec_tool"}}[0];
	my $qc2mod2sparse_tool=@{$params{"qc2mod2sparse"}}[0];
	$trace_file = @{$params{"trace_file"}}[0];
	my $left_degree;
	my $fec_ratio;
	my $codec;
	my $symbol_size;
	my $r;
	my $k;
	my $num_k;
	my $ss;
	my $tx_type;
	my $loss;
	my $iter = @{$params{"nb_iterations"}}[0];
	my $counter=0;
	my $exp_factor=0;
	my $nb_thread = $params{"nb_thread"};
	
	verbose("$nb_thread thread(s) will be used.\n",$params{"verbose"});
	
	#only one thread can access to @tab_cmd and shift it !
	$semaphore = Thread::Semaphore->new;
	# matrix mode for QC-LDPC codecs : by default, it's binary matrices
	my $matrix_mode="binary";
	if (  @{$params{"matrix_mode"}}[0] eq "qc")
	{
		$matrix_mode="qc";
	}
	
	#used by the differents threads
	my $nb_tmp_files=0;

	my $os = get_os();
	my $hostname = `hostname`;
	my $uname = `uname -a`;

	#erase the final trace file
	unlink $trace_file;
	

	#set the database type, create tables, and set the connection
	if ($database_type eq "file")
	{
		verbose("Database type : File\n",$params{"verbose"});
		# only one thread can access to sqlite database file.
		$semaphore_sqlite = Thread::Semaphore->new;
		if ($erase_database eq "true")
		{
			verbose("Erasing database File\n",$params{"verbose"});
			#erase the database file if exists
			unlink @{$params{"database"}}[1] if (-e @{$params{"database"}}[1]);
			#open a connection to the database file
			@connection_string = ("dbi:SQLite:dbname=".@{$params{"database"}}[1],"","");
			$dbh = DBI->connect("dbi:SQLite:dbname=".@{$params{"database"}}[1],"","");
			verbose("Creating tables\n",$params{"verbose"});
			create_database_template($dbh,"sqlite");
			init_database($dbh);
		}
		else
		{
			#open a connection to the database file
			@connection_string =("dbi:SQLite:dbname=".@{$params{"database"}}[1],"","");
			$dbh = DBI->connect("dbi:SQLite:dbname=".@{$params{"database"}}[1],"","");
			verbose("Creating tables\n",$params{"verbose"});
			create_database_template($dbh,"sqlite");
			if (get_codec_id($dbh,"LDPC-Staircase") eq "3")
			{
				verbose("Fill codec table\n",$params{"verbose"});
				init_database($dbh);
			}
		}
	}
	else
	{
		if ($database_type eq "server")
		{
			verbose("Database type : Server\n",$params{"verbose"});
			@connection_string =("dbi:mysql:database=".@{$params{"database"}}[1].";host=".@{$params{"database"}}[2].";port=".@{$params{"database"}}[3],@{$params{"database"}}[4],@{$params{"database"}}[5]);
			#open a connection to the database file
			$dbh = DBI->connect("dbi:mysql:database=".@{$params{"database"}}[1].";host=".@{$params{"database"}}[2].";port=".@{$params{"database"}}[3],@{$params{"database"}}[4],@{$params{"database"}}[5]);
			if ($erase_database eq "true")
			{
				verbose("Erase database on server\n",$params{"verbose"});
				erase_database($dbh);		
				verbose("Creating tables\n",$params{"verbose"});
				create_database_template($dbh,"mysql");
				if (not (get_codec_id($dbh,"LDPC-Staircase") eq "3"))
				{
					verbose("Fill codec table\n",$params{"verbose"});
					init_database($dbh);
				}
		
			}
			else
			{
				verbose("Creating tables\n",$params{"verbose"});
				create_database_template($dbh,"mysql");
				if (not (get_codec_id($dbh,"LDPC-Staircase") eq "3"))
				{
					verbose("Fill codec table\n",$params{"verbose"});
					init_database($dbh);
				}
			}
		}
		else
		{
			print "\e[31m\e[1mERROR\e[0m\e[31m: Wrong database type. Exiting !\e[0m\n";
			return;
		}
	}

	
	system("echo '--------------------------------' > $trace_file");
	system("echo 'Hostname=$hostname OS=$os' >> $trace_file");
	system("echo 'uname=$uname' >> $trace_file");
	system("echo '--------------------------------\n' >> $trace_file");
	
	#get list of number of source symbols values with min max step scheme.
	my @list_of_k = get_list_of_values(\@{$params{"nb_source_symbols"}});
	my @list_of_r;

	my @list_of_left_degrees;
	#if (defined @{$params{"ldpc_N1"}})
	if (defined {$params{"ldpc_N1"}})
	{
		@list_of_left_degrees = get_list_of_values(\@{$params{"ldpc_N1"}});
	}
	my @list_of_exp_factor;
	#if (defined @{$params{"exp_factor"}})
	if (defined {$params{"exp_factor"}})
	{
		@list_of_exp_factor = get_list_of_values(\@{$params{"exp_factor"}});
	}
	#code_rate param is defined ( scalar > 1 <=> size > 0 ), we organize both nb_repair_symbols and nb_source_symbols tabs.
	#if (defined @{$params{"code_rate"}})
	if (defined {$params{"code_rate"}})
	{
		verbose("Using code rate parameter : repair symbols number will be calculated \n",$params{"verbose"});
		my $k_idx=0;
		@{$params{"nb_source_symbols"}} = ();
		#foreach my $ratio (@{$params{"code_rate"}})
		# Iteration on code rate was deleted
		my $ratio = $params{"code_rate"}[0];
	
		#$ratio=Math::Fraction::frac($ratio);
		foreach my $copy_k (@list_of_k)
		{
			@{$params{"nb_source_symbols"}}[$k_idx] = int($copy_k);
			@{$params{"nb_repair_symbols"}}[$k_idx] = int($copy_k * 1/$ratio)-int($copy_k);
			$k_idx++;
		}
	}
	else
	{
		verbose("Using nb repair symbols parameter.\n",$params{"verbose"});
		# only if number of repair symbols values are defined
		#if ( defined @{$params{"nb_repair_symbols"}})
		if (defined {$params{"nb_repair_symbols"}})
		{
			@list_of_r = get_list_of_values(\@{$params{"nb_repair_symbols"}});
		}
		if (scalar @list_of_k != scalar @list_of_r)
		{
			print "Please, check your parameters file and correct nb_source_symbols and nb_repair_symbols parameters: they must have same length.\n";
			return -1;
		}
		@{$params{"nb_source_symbols"}} = @list_of_k;
		@{$params{"nb_repair_symbols"}} = @list_of_r;
	}
	verbose("Cleaning tmp directory... ",$params{"verbose"});
	`rm ./tmp/* >/dev/null 2>/dev/null`;
	verbose("Done.\n",$params{"verbose"});
	
	verbose("Running tests...\n",$params{"verbose"});
	#calculate total tests number
	my $nb_tests = scalar(@{$params{"codec"}}) * scalar(@{$params{"tx_type"}}) * scalar(@{$params{"symbol_size"}}) * scalar(@{$params{"nb_source_symbols"}});
	if ($codec >=3) #ldpc
	{
		$nb_tests *= scalar(@list_of_left_degrees);
	}
	elsif ($codec==2)
	{
		$nb_tests *= scalar(@{$params{"rs_m"}});
	}
	
	$current_test_number = 1;
	
	#get list of losses, to launch progress display thread
	my @list_of_losses = get_list_of_losses(\@{$params{"loss"}});
	my $nb_loss = (@{$params{"find_min_overhead"}}[0] eq "false")?scalar(@list_of_losses):1; #number of loss in list_of_losses
	
	#launch progress display
	my $display_thread =  threads->create("thread_progress_display",($nb_thread,$nb_tests*$nb_loss));
	
	# here, we start loops for tests.
	foreach $codec (@{$params{"codec"}})
	{	
		foreach $tx_type (@{$params{"tx_type"}})
		{
			foreach $ss (@{$params{"symbol_size"}})
			{
				$num_k = 0;
				#if nb_repair_symbols is given in the parameters file, we have only one nb_source_symbols (condition verified in check_params) 
				foreach $k (@{$params{"nb_source_symbols"}})
				{
					$r = @{$params{"nb_repair_symbols"}}[$num_k];
					$num_k++;
					if ($codec >=3) #ldpc
					{
				 		foreach $left_degree (@list_of_left_degrees)
					 	{
					 		verbose("\e[1mStarting test n°$current_test_number ...\e[0m\n",$params{"verbose"});
							$tool_with_args = $tool." -k=".$k." -r=".$r." -codec=".$codec." -ldpc_N1=".$left_degree." -symb_sz=".$ss." -tx_type=".$tx_type;
							prepare_tests($tool_with_args,$iter,$nb_thread,\%params,$nb_tests,$current_test_number,$nb_loss,@list_of_losses);
							$current_test_number++;
					 	}
					}
					else
					{
						if ($codec==2)
						{
							foreach my $m (@{$params{"rs_m"}})
							{
								verbose("\e[1mStarting test n°$current_test_number ...\e[0m\n",$params{"verbose"});
								$tool_with_args = $tool." -k=".$k." -r=".$r." -codec=".$codec." -rs_m=".$m." -symb_sz=".$ss." -tx_type=".$tx_type;  
								prepare_tests($tool_with_args,$iter,$nb_thread,\%params,$nb_tests,$current_test_number,$nb_loss,@list_of_losses);
								$current_test_number++;
							}				
						}
						else
						{
							verbose("\e[1mStarting test n°$current_test_number ...\e[0m\n",$params{"verbose"});
							$tool_with_args = $tool." -k=".$k." -r=".$r." -codec=".$codec." -symb_sz=".$ss." -tx_type=".$tx_type;	
							prepare_tests($tool_with_args,$iter,$nb_thread,\%params,$nb_tests,$current_test_number,$nb_loss,@list_of_losses);
							$current_test_number++;
						}
					}
				}
			}
		}
	}


	#fill_database($dbh);
	
	$dbh->disconnect();
	#clear temporary datafiles
	clear_tmp($nb_tmp_files,$trace_file, $params{"verbose"});
	
	#WRITINGTIME : calculate total writing time
###	$total_writing_time = 0;
###	for (1..$nb_thread) {
###		$total_writing_time = $total_writing_time + @thread_writing_time[$_];
###	}
	
	$display_thread->join;
	verbose("Diplay thread joined.\n",,$params{"verbose"});
	
	verbose("\n\e[1mMain end\e[0m\n",$params{"verbose"});
	return(0);
}


sub prepare_tests
{
	my ($tool_with_args,$iter,$nb_thread,$params,$nb_tests,$current_test_number,$nb_loss,@list_of_losses) = @_;
	my @threads;
	my $from=0;
	my $cmd;
	my $to=0;
	#my @tab_for_iter = get_nb_iters_for_each_threads($iter,$nb_thread);
	verbose("$$params{\"nb_iterations\"}[0] iterations will be run.\n",$$params{"verbose"});
	my ($nb_iter_in_small_part,$nb_iter_in_large_part,$nb_large_parts) = get_iter_allocation($nb_thread,$$params{"nb_iterations"}[0]);
	my $threshold;
	#now give in parameter
	#my @list_of_losses = get_list_of_losses(\@{$$params{"loss"}});
	#my $nb_loss = (@{$$params{"find_min_overhead"}}[0] eq "false")?scalar(@list_of_losses):1; #number of loss in list_of_losses
	# check if tmp folder exists ; if not, create it
	check_tmp_folder($$params{"verbose"});
	
	
	#WRITINGTIME : initialize threads progress and writing time arrays ...
###	for (1..$nb_thread) {
###		@thread_progress_state[$_] = 0;
###		@thread_writing_time[$_] = 0;
###	}

##	#launch progress display
##	my $display_thread =  threads->create("thread_progress_display",($nb_thread,$nb_tests*$nb_loss));
	
	if (@{$$params{"find_min_overhead"}}[0] eq "false")
	{		
		verbose("Iterating on $nb_loss loss values ...\n",$$params{"verbose"});
		foreach my $loss (@list_of_losses)
		{
			$threshold = 1-$loss/100;
			verbose("Calculated loss threshold for loss percentage $loss% : th = $threshold\n",$$params{"verbose"});
			$from = 0;
			$to = 0;
			$cmd = $tool_with_args." -loss=".$loss;
			for (1..$nb_thread)
			{
				$from = $to + 1;
				if ($_ <= $nb_large_parts) {
					$to = $from + $nb_iter_in_large_part - 1;
				} else {
					$to = $from + $nb_iter_in_small_part - 1;
				}
				
				verbose("\nLaunching : \n\tThread n°$_\n\tfrom $from to $to\n\tloss (type:percentage) = ($loss)\n\n",$$params{"verbose"});
				#launch thread
				$threads[$_] = threads->create("run_test",($cmd,$from+(@{$$params{"offset_iteration"}}[0]),$to+(@{$$params{"offset_iteration"}}[0]),"./tmp/tmp_".$_.".txt",@{$$params{"nb_iterations_for_partial_results"}}[0],@{$$params{"find_min_overhead"}}[0],$threshold, $$params{"verbose"},$_,$nb_thread));
			}
			#wait for all threads, store temporary values and delete temporary files
			for (1..$nb_thread)
			{
				verbose("Waiting for thread n°$_ ...\n",$$params{"verbose"});
				$threads[$_]->join();
				verbose("Thread n°$_ joined.\n",$$params{"verbose"});
				system("cat ./tmp/tmp_".$_.".txt_backup >> $trace_file");
				unlink "./tmp/tmp_".$_.".txt_backup";
			}
		}
	}
	else
	{
		for (1..$nb_thread)
		{
			$from = $to + 1;
			if ($_ <= $nb_large_parts) {
				$to = $from + $nb_iter_in_large_part - 1;
			} else {
				$to = $from + $nb_iter_in_small_part - 1;
			}

			verbose("\nLaunching : \n\tThread n°$_\n\tfrom $from to $to\n\n",$$params{"verbose"});
				#launch thread
			$threads[$_] = threads->create("run_test",($tool_with_args,$from,$to,"./tmp/tmp_".$_.".txt",@{$$params{"nb_iterations_for_partial_results"}}[0],@{$$params{"find_min_overhead"}}[0],$threshold, $$params{"verbose"},$_,$nb_thread));
		}
		#wait for all threads, store temporary values and delete temporary files 
		for (1..$nb_thread)
		{
			verbose("Waiting for thread n°$_ ...\n",$$params{"verbose"});
			$threads[$_]->join();
			verbose("Thread n°$_ joined.\n",$$params{"verbose"});
			system("cat ./tmp/tmp_".$_.".txt_backup >> $trace_file");
			unlink "./tmp/tmp_".$_.".txt_backup";
		}
		
	}
}

sub thread_progress_display
{
	#progress percentage given is just indicative, it's not quite correct.
	my ($nb_thread,$nb_tot_tests) = (shift,shift);
	my $thread_progress_sum;
	my $progress = 0;
	until ($progress>=100) {
		$thread_progress_sum =0;
		#calculate sum of global array thread_progress_state
		foreach (@thread_progress_state) {
			$thread_progress_sum += $_;
		}
		
		$progress = int($thread_progress_sum/($nb_thread*$nb_tot_tests));
		print "\e[1m\e[42m\e[s\e[0;0H\e[KProgress:\t$progress%\t\t\t\t\t\t\e[4mRUN TESTS\n\e[0m\e[K\e[u";
		sleep 1;
	}
}

#Changed, to got a optimized function
sub get_nb_iters_for_each_threads
{
	my ($iter,$nb_threads) = @_;
	my $rest = $iter;
	my $grain = floor($iter/$nb_threads);
	my @ret;
	for (my $i=1;$i<$nb_threads;$i++)
	{
		push @ret,$grain;
		$rest -=$grain;
	}
	push @ret,$rest;
	return @ret;
}

#optimized method to allocate iterations. return (small_size, large_size, nb_large_size)
sub get_iter_allocation
{
	my ($nb_parts,$total_iter) = (shift,shift);
	my $nb_iter_in_small_part = floor($total_iter/$nb_parts);
	my $nb_iter_in_large_part = ceil($total_iter/$nb_parts);
	my $nb_large_parts = $total_iter - $nb_iter_in_small_part * $nb_parts;
	return ($nb_iter_in_small_part,$nb_iter_in_large_part,$nb_large_parts);
}


sub get_list_of_values
{
	my @t;
	my $tab = shift;
	if ($$tab[2] < 1 or not defined $$tab[2]) { $$tab[2] =1 };
	for (my $i=$$tab[0];$i<=$$tab[1];$i+=$$tab[2])
	{
		push @t,$i;
	}
	return @t;
}


sub get_list_of_losses
{
	my @t;
	my $tab = shift;
	if ($$tab[3] < 1 or not defined $$tab[3]) { $$tab[3] =1 };
	for (my $i=$$tab[1];$i<=$$tab[2];$i+=$$tab[3])
	{
		push @t,"".$$tab[0].":".$i;
	}
	return @t;
}


sub fill_database_with_file(file, thread_nb, verbose)
{
	#WRITINGTIME : remember when writing start
###	my $start = time;
	
	my ($f,$thread_nb,$verbose) = (shift,shift,shift);
	$_=$connection_string[0];
	# are we using SQLite ?
	if (/SQLite/)
	{
		verbose("Using SQLite.\nWaiting for semaphore to write $f in database ...\n",$verbose);
		$semaphore_sqlite->down();
		verbose("Semaphore token to write datafile $f.\n",$verbose);
	}
	my $dbh=DBI->connect(@connection_string);
	my %res;
	my $id_init_table=0;
	my $id_encoding_table;
	my $id_decoding_table;
	my $id_matrix_table;
	my $id_run_table;
	my $unik_id_run_table;
	open (I_FILE,$f) || do {print "Could not open $f\n"; print $!."\n"; exit -1; }; #open temporary file ...
	while (<I_FILE>) {
		SEARCH: {
			/-loss/ && do {
				$res{"loss_type"} = getval($_,'-loss');
			};
			/tot_nb_source_symbols/ && do {
				$res{"nb_xor_it"} = 0;
				$res{"nb_xor_ml"} = 0;
				$res{"k"} = getval($_, 'tot_nb_source_symbols');
				$res{"r"} = getval($_, 'tot_nb_repair_symbols');
				$res{"ss"}= getval($_, 'symbol_size');
				$res{"l"} = getval($_, 'ldpc_N1');
				$res{"rs_m"} = getval($_, 'rs_m');
			};
			/codec_id/ && do {
				$res{"c"} = getval($_,'codec_id');
			};
			/transmission_type/ && do {
				$res{"tx"} = getval($_,'transmission_type');
			};
			/iter/ && do {
				$res{"i"} = getval($_,'iter');
			};
			/prng seed/ && do {
				$res{"seed"} = getval($_,'seed');
			};
			# init_start has been removed from eperftool traces since it does not
			# correspond to a meaningful initialization time, unlike its name.
			#/init_start/ && do {
			#  $res{"init_start"} = getval($_,'init_start');
			#};
			/init_end/ && do {
				$res{"init_end"} = getval($_,'init_end');
				$res{"init_time"} = getval($_,'init_time');
			};
			/encoding_start/ && do {
				$res{"encoding_start"} = getval($_,'encoding_start');
			};
			/encoding_end/ && do {
				$res{"encoding_end"} = getval($_,'encoding_end');
				$res{"encoding_time"} = getval($_,'encoding_time');
			};
			/decoding_start/ && do {
				$res{"decoding_start"} = getval($_,'decoding_start');
			};
			/nb_xor_for_IT/ && do {
				$res{"nb_xor_it"} = getval($_,'nb_xor_for_IT');
			};
			/nb_xor_for_ML/ && do {
				$res{"nb_xor_ml"} = getval($_,'nb_xor_for_ML');
			};
			/decoding_end/ && do {
				$res{"decoding_end"} = getval($_,'decoding_end');
				$res{"decoding_time"} = getval($_,'decoding_time');
				$res{"nb_received_symbols"} = getval($_,'nb_received_symbols');
				$res{"inef_ratio"} = getval($_,'inefficiency_ratio');
			};

			/decoding_status/ && do {
				$res{"decoding_status"} = getval($_,'decoding_status');
			
				#remove space caracters
				$res{"c"} =~ s/^\s+//;
				if (not $res{"decoding_status"} eq "0")
				{
					print "\n\e[31m\e[1mERROR\e[0m\e[31m: ignoring this record..(-k=$res{\"k\"},-r=$res{\"r\"},-codec=$res{\"c\"},-loss =".get_loss_percentage($res{"loss_type"},$res{"k"},$res{"r"})."%)\e[0m";
					next;
				}
				else
				{
					# init_start has been removed from eperftool traces since it does not
					# correspond to a meaningful initialization time, unlike its name.
					#$dbh->do("INSERT INTO init_table (init_start,init_end,init_time) VALUES (".$res{"init_start"}.",".$res{"init_end"}.",".$res{"init_time"}.");") or die "database insert failed";
					$id_init_table = get_last_insert_id($dbh);
					$dbh->do("INSERT INTO encoding_table (encoding_start,encoding_end,encoding_time) VALUES (".$res{"encoding_start"}.",".$res{"encoding_end"}.",".$res{"encoding_time"}.");") or die "database insert failed";
					$id_encoding_table = get_last_insert_id($dbh);
					if ($res{"inef_ratio"} == "")
					{
						$res{"nb_received_symbols"} = "0";
						$dbh->do("INSERT INTO decoding_table (decoding_start,decoding_end,decoding_time,decoding_steps,decoding_inef,decoding_status) VALUES (".$res{"decoding_start"}.",0,0,0,0,".$res{"decoding_status"}.");") or die "database insert failed";
					}
					else
					{
						$dbh->do("INSERT INTO decoding_table (decoding_start,decoding_end,decoding_time,decoding_steps,decoding_inef,decoding_status) VALUES (".$res{"decoding_start"}.",".$res{"decoding_end"}.",".$res{"decoding_time"}.",".$res{"nb_received_symbols"}.",".$res{"inef_ratio"}.",".$res{"decoding_status"}.");") or die "database insert failed";
					}
					$id_decoding_table = get_last_insert_id($dbh);
					if ($res{"c"} == '2') 
					{	
						$unik_id_run_table = get_run_id($dbh,$res{"k"},$res{"r"},$res{"c"},$res{"rs_m"},$res{"ss"},get_tx_id($dbh,$res{"tx"}));
					}
					else
					{
						$unik_id_run_table = get_run_id($dbh,$res{"k"},$res{"r"},$res{"c"},$res{"l"},$res{"ss"},get_tx_id($dbh,$res{"tx"}));
					}
					if ($unik_id_run_table eq "")
					{
						if ($res{"c"} == '2') 
						{
							$dbh->do("INSERT INTO run_table (run_k,run_r,codec_id,run_left_degree,run_symbol_size,tx_id) VALUES (".
							$res{"k"}.",".$res{"r"}.",".$res{"c"}.",".$res{"rs_m"}.",".$res{"ss"}.",".get_tx_id($dbh,$res{"tx"}).");") or die "database insert failed";
						}
						else
						{
							$dbh->do("INSERT INTO run_table (run_k,run_r,codec_id,run_left_degree,run_symbol_size,tx_id) VALUES (".
							$res{"k"}.",".$res{"r"}.",".$res{"c"}.",".$res{"l"}.",".$res{"ss"}.",".get_tx_id($dbh,$res{"tx"}).");") or die "database insert failed";
						}
						
						$id_run_table = get_last_insert_id($dbh);
						verbose("Values to write in database :\n\tid_run_table = ".$id_run_table."\n\tid_init_table = "
													.$id_init_table."\n\tid_encoding_table = "
													.$id_encoding_table."\n\tid_decoding_table = "
													.$id_decoding_table."\n\titeration number = "
													.$res{"i"}."\n\tseed = "
													.$res{"seed"}."\n\tnb_received_symbols = "
													.$res{"nb_received_symbols"}."\n\tloss_percentage = "
													.get_loss_percentage($res{"loss_type"},$res{"k"},$res{"r"})."\n\tnb_xor_it = "
													.$res{"nb_xor_it"}."\n\tnb_xor_ml = "
													.$res{"nb_xor_ml"}."\n\n",$verbose);
						
						$dbh->do("INSERT INTO iter_table(	run_id,
															init_id,
															encoding_id,
															decoding_id,
															matrix_id,
															iter_num,
															iter_seed,
															iter_nb_received_symbols,
															iter_loss_percentage,
															iter_nb_xor_for_it,
															iter_nb_xor_for_ml)
										VALUES ("	.$id_run_table.","
													.$id_init_table.","
													.$id_encoding_table.","
													.$id_decoding_table.
													",0,"
													.$res{"i"}.","
													.$res{"seed"}.","
													.$res{"nb_received_symbols"}.","
													.get_loss_percentage($res{"loss_type"},$res{"k"},$res{"r"}).","
													.$res{"nb_xor_it"}.","
													.$res{"nb_xor_ml"}.");")
								or die "database insert failed";
					}
					else
					{
	#					verbose("Values to write in database :\n\tunik_id_run_table = ".$unik_id_run_table.",\n\tid_init_table = "
	#												.$id_init_table.",\n\tid_encoding_table = "
	#												.$id_encoding_table.",\n\tid_decoding_table = "
	#												.$id_decoding_table.",\n\titeration number = "
	#												.$res{"i"}.",\n\tseed = "
	#												.$res{"seed"}.",\n\tnb_received_symbols = "
	#												.$res{"nb_received_symbols"}.",\n\tloss_percentage = "
	#												.get_loss_percentage($res{"loss_type"},$res{"k"},$res{"r"}).",\n\tnb_xor_it = "
	#												.$res{"nb_xor_it"}.",\n\tnb_xor_ml = "
	#												.$res{"nb_xor_ml"}."\n\n",$verbose);
						$dbh->do("INSERT INTO iter_table(	run_id,
															init_id,
															encoding_id,
															decoding_id,
															matrix_id,
															iter_num,
															iter_seed,
															iter_nb_received_symbols,
															iter_loss_percentage,
															iter_nb_xor_for_it,
															iter_nb_xor_for_ml)
										VALUES ("	.$unik_id_run_table.","
													.$id_init_table.","
													.$id_encoding_table.","
													.$id_decoding_table.
													",0,"
													.$res{"i"}.","
													.$res{"seed"}.","
													.$res{"nb_received_symbols"}.","
													.get_loss_percentage($res{"loss_type"},$res{"k"},$res{"r"}).","
													.$res{"nb_xor_it"}.","
													.$res{"nb_xor_ml"}.");")
							or die "database insert failed";
					}
				}
			};
		}
	}
	close(I_FILE);
	$dbh->disconnect();
	$_=$connection_string[0];
	if (/SQLite/)
	{
		$semaphore_sqlite->up();
		verbose("1 semaphore given.\n",$verbose);
	}
	
	#WRITINGTIME : calculate and store writing time.
###	my $end = time;
###	@thread_writing_time[$thread_nb] = @thread_writing_time[$thread_nb] + ($end - $start);
}

 
# Perform a simulation iteration and write the result in the database.
# Return the number of symbols received if okay, or 0 in case of error.
sub run_and_write_db
{
	my $cmd = $_[0];
	my $trc_file = $_[1];
	my $fmo_bool = $_[2];
	my $result;
	my $status = -1;	# decoding status returned by eperftool
	my $nb_rx = -1;		# minimum number of received symbols for success

	no warnings;
	# launch eperftool command in find minimum overhead mode so that it automatically searches
	# the minimum overhead, and in non verbose mode so that it only prints the final traces,
	# when the minimum overhead is found (if possible).
	
	#choose option find min overhead for eperftool
	if ($fmo_bool == 1) {
		$cmd = $cmd . ' -find_min_overhead';
	}
	#choose verbose level for eperftool
	$cmd = $cmd . ' -v=0';
	#print "cmd = $cmd \n";
	$result = `$cmd 2>&1`;
	#print "result = $result \n";
	$status = getval($result, 'decoding_status') ;
	$nb_rx = getval($result, 'nb_received_symbols');
	if (($status == 2) || ($nb_rx == -1) ) {
		# something wrong happened, stop everything...
		print "\e[33m\e[1mWARNING\e[0m\e[33m: decoding failed (status=$status, nb_rx=$nb_rx) for : $cmd\e[0m\n";
		return 0;
	}
	if ($status == 0) {
#		if ($nb_rx==-1) {
#			$nb_rx=0;
#		}
		system("echo '$cmd' >> $trc_file");
		system("echo '$result' >> $trc_file");
		return $nb_rx;
	} else {
		# failure, there is no possible solution. Return 0 which remains a valid loss value for eperftool
		return 0;
	}
}


sub get_nb_max_loss_for(cmd)
{
	my $cmd = shift;
	my $k = getval($cmd,'-k');
	my $r = getval($cmd,'-r');
	return $k * ($r-1) + 1;
}


sub run_test
{
	my ($cmd, $from, $to, $trc_file, $nb_iter_before_insert, $find_min_overhead, $threshold, $verbose, $thread_nb, $nb_tot_threads) = @_;
	my $nb_rx;
	#iteration number since last write
	#my $i = int($nb_iter_before_insert + ($nb_iter_before_insert/$nb_tot_threads) * $thread_nb);
	my $i = int (($nb_iter_before_insert/$nb_tot_threads) * $thread_nb);
	verbose("Spacing for thread $thread_nb : $i\n",$verbose);
	my $iter;	#global iteration number
	my $my_cmd;
	my $fmo_param;
	my $old_percentage = @thread_progress_state[$thread_nb];
	
	verbose("Starting thread n°$thread_nb run ...\n",$verbose);
	unlink $trc_file;
	#write thread number in file
	system("echo 'thread number=$thread_nb\nrange = $from,$to\n\n' > $trc_file");
LOOP: for ($iter = $from; $iter <= $to; $iter++) {
		#calculate progress percentage
		my $range=($to-$from);
		if ($range>0) {
			@thread_progress_state[$thread_nb] = int((($iter-$from)/$range) *100 + $old_percentage);
		} else {
			@thread_progress_state[$thread_nb] = 100 + $old_percentage;
		}
		
		$my_cmd = $cmd ." -seed=".($iter+1);
		system("echo '\n\niter=$iter\n' >> $trc_file");
		if ($find_min_overhead eq "true")
		{
			$fmo_param = 1;
		} else {
			$fmo_param = 0;
		}
		$nb_rx = run_and_write_db($my_cmd, $trc_file, $fmo_param);
		if ($nb_rx == 0)
		{
			my $err_str = "\nignoring test with seed=".($iter+1)."\n";
			verbose($err_str,$verbose);
			system("echo 'Test ignored\n\n' >> $trc_file");
			next LOOP;
		}
		if ($i >= $nb_iter_before_insert)
		{
			verbose("Writing data in database ... (thread n°$thread_nb)");
			&fill_database_with_file($trc_file, $thread_nb, $verbose);
			system("cat $trc_file >> ".$trace_file."_backup");
			unlink $trc_file;
			$i = 0;
		}
		$i++;
	}
	if ($i != 1) {
		&fill_database_with_file($trc_file, $thread_nb, $verbose);
		system("cat $trc_file >> ".$trc_file."_backup");
		unlink $trc_file;
	}
	verbose("Thread n°$thread_nb run end.\n",$verbose);
	return;
}


#only for mysql
sub erase_database(dbh)
{
	my $dbh = shift;
	my $database = shift;
	my @reqs= (
		"DROP TABLE IF EXISTS codec_table;",
		"DROP TABLE IF EXISTS encoding_table;",
		"DROP TABLE IF EXISTS decoding_table;",
		"DROP TABLE IF EXISTS init_table;",
		"DROP TABLE IF EXISTS tx_table;",
		"DROP TABLE IF EXISTS run_table;",
		"DROP TABLE IF EXISTS iter_table;",
		"DROP TABLE IF EXISTS matrix_table;");
	foreach  my $r (@reqs)
	{
		$dbh->do($r) or die "created failed";
	}
}


# Initialize the codec_table and tx_table tables with default values.
# Warning: make sure that these tables are in line with the eperftool
# values. Therefore the order is extremely important.
#
sub init_database(dbh)
{
	my $dbh=shift;
	my @reqs= (
		"INSERT INTO codec_table (codec_name) VALUES ('Reed-Solomon GF(2^8)');",
		"INSERT INTO codec_table (codec_name) VALUES ('Reed-Solomon GF(2^m)');",
		"INSERT INTO codec_table (codec_name) VALUES ('LDPC-Staircase');",
		"INSERT INTO codec_table (codec_name) VALUES ('not working');",
		"INSERT INTO codec_table (codec_name) VALUES ('2D parity matrix');",
		"INSERT INTO codec_table (codec_name) VALUES ('LDPC from file');",
		"INSERT INTO tx_table (tx_name) VALUES ('randomly_send_all_source_and_repair_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('randomly_send_a_few_source_symbols_and_repair_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('randomly_send_a_few_src_symbols_first_then_randomly_all_repair_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('randomly_send_only_repair_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('sequentially_send_all_src_symbols_first_then_repair_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('sequentially_send_all_repair_symbols_first_then_src_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('sequentially_send_all_src_symbols_first_then_randomly_src_symbols');",
		"INSERT INTO tx_table (tx_name) VALUES ('sequentially_send_all_repair_symbols_first_then_randomly_src_symbols');"
	);
	foreach  my $r (@reqs)
	{
		$dbh->do($r) or die "created failed";
	}
}


#create tables and default values ( particulary for codec table)
sub create_database_template(dbh,db_type)
{
	my $dbh = shift;
	my $db_type = shift;
	my @reqs;
	if ($db_type eq "mysql")
	{
		@reqs= (
			"CREATE TABLE IF NOT EXISTS codec_table ( codec_id INTEGER  AUTO_INCREMENT PRIMARY KEY, codec_name VARCHAR(50) );",
			"CREATE TABLE IF NOT EXISTS matrix_table (matrix_id INTEGER AUTO_INCREMENT PRIMARY KEY);",
			"CREATE TABLE IF NOT EXISTS tx_table (tx_id INTEGER AUTO_INCREMENT PRIMARY KEY, tx_name VARCHAR(100) );",
			"CREATE TABLE IF NOT EXISTS init_table ( init_id INTEGER  AUTO_INCREMENT PRIMARY KEY, init_start FLOAT, init_end FLOAT, init_time FLOAT );",
			"CREATE TABLE IF NOT EXISTS encoding_table ( encoding_id INTEGER  AUTO_INCREMENT PRIMARY KEY, encoding_start DOUBLE, encoding_end DOUBLE, encoding_time FLOAT );",
			"CREATE TABLE IF NOT EXISTS decoding_table ( decoding_id INTEGER  AUTO_INCREMENT PRIMARY KEY, decoding_start DOUBLE, decoding_end DOUBLE,decoding_time FLOAT, decoding_steps FLOAT, decoding_inef FLOAT,decoding_status BOOL );",
			"CREATE TABLE IF NOT EXISTS run_table ( run_id INTEGER  AUTO_INCREMENT PRIMARY KEY, run_k INTEGER, run_r INTEGER, codec_id INTEGER , run_left_degree INTEGER, run_symbol_size INTEGER,tx_id INTEGER,CONSTRAINT fk_codec_id FOREIGN KEY (codec_id) REFERENCES codec_table(codec_id),CONSTRAINT fk_tx_id FOREIGN KEY (tx_id) REFERENCES tx_table(tx_id));",
			"CREATE TABLE IF NOT EXISTS iter_table (iter_id INTEGER  AUTO_INCREMENT PRIMARY KEY, run_id INTEGER , 
			init_id INTEGER, 
			encoding_id INTEGER, 
			decoding_id INTEGER,
			matrix_id INTEGER,
			iter_num INTEGER, iter_seed INTEGER, iter_nb_received_symbols INTEGER, iter_loss_percentage FLOAT, iter_nb_xor_for_it INTEGER, iter_nb_xor_for_ml INTEGER,
			CONSTRAINT fk_run_id FOREIGN KEY (run_id) REFERENCES run_table(run_id),
			CONSTRAINT fk_init_id FOREIGN KEY (init_id) REFERENCES init_table(init_id),
			CONSTRAINT fk_encoding_id FOREIGN KEY (encoding_id) REFERENCES encoding_table(encoding_id),
			CONSTRAINT fk_decoding_id FOREIGN KEY (decoding_id) REFERENCES decoding_table(decoding_id),
			CONSTRAINT fk_matrix_id FOREIGN KEY (matrix_id) REFERENCES matrix_table(matrix_id));"
		);
	}
	else
	{
		@reqs= (
			"CREATE TABLE IF NOT EXISTS codec_table ( codec_id INTEGER PRIMARY KEY, codec_name VARCHAR(50) );",
			"CREATE TABLE IF NOT EXISTS matrix_table (matrix_id INTEGER PRIMARY KEY);",
			"CREATE TABLE IF NOT EXISTS tx_table (tx_id INTEGER PRIMARY KEY, tx_name VARCHAR(100) );",
			"CREATE TABLE IF NOT EXISTS init_table ( init_id INTEGER PRIMARY KEY, init_start FLOAT, init_end FLOAT, init_time FLOAT );",
			"CREATE TABLE IF NOT EXISTS encoding_table ( encoding_id INTEGER PRIMARY KEY, encoding_start DOUBLE, encoding_end DOUBLE, encoding_time FLOAT );",
			"CREATE TABLE IF NOT EXISTS decoding_table ( decoding_id INTEGER PRIMARY KEY, decoding_start DOUBLE, decoding_end DOUBLE,decoding_time FLOAT, decoding_steps FLOAT, decoding_inef FLOAT,decoding_status BOOL);",
			"CREATE TABLE IF NOT EXISTS run_table ( run_id INTEGER PRIMARY KEY, run_k INTEGER, run_r INTEGER, codec_id INTEGER CONSTRAINT fk_codec_id REFERENCES codec_table(codec_id), run_left_degree INTEGER, run_symbol_size INTEGER,tx_id INTEGER);",
			"CREATE TABLE IF NOT EXISTS iter_table (iter_id INTEGER PRIMARY KEY, run_id INTEGER CONSTRAINT fk_run_id REFERENCES run_table(run_id), init_id INTEGER CONSTRAINT fk_init_id REFERENCES init_table(init_id), encoding_id INTEGER CONSTRAINT fk_encoding_id REFERENCES encoding_table(encoding_id), decoding_id INTEGER CONSTRAINT fk_decoding_id REFERENCES decoding_table(decoding_id),matrix_id INTEGER CONSTRAINT fk_matrix_id REFERENCES matrix_table(matrix_id), iter_num INTEGER, iter_seed INTEGER,iter_nb_received_symbols INTEGER,iter_loss_percentage FLOAT,iter_nb_xor_for_it INTEGER,iter_nb_xor_for_ml INTEGER );"
		);
	}
	foreach  my $r (@reqs)
	{
		$dbh->do($r) or die "created failed";
	}
}

sub get_loss_percentage(str,k,r)
{
	my ($str,$k,$r) = @_;
	my @t = split(':',$str);
	if ($t[0] ==2) #percentage
	{
		return ($t[1]);
	}
	elsif($t[0] == 3) #number
	{
		return $t[1]/($k+$r)*100;
	}
	elsif($t[0] == 0)
	{
		return 0;
	}
}


#
# Search in record rec the string str and return the value
# that follows immediately "=".
#
sub getval(rec,str) {
	my	$rec = shift;
	my	$str = shift;
	my	@A;
	my	@B;

	$str = $str . '=';
	@A = split($str, $rec);
	if ($#A < 1)
	{
		return -1;
	}
	#die "ERROR, string \"$str\" not found in:\n$rec ($A[1])" unless ($#A >= 1);
	@B = split(/\s+/, $A[1]);	# split what follows $str using spaces
	return $B[0];			# and only consider the 1st field...
}

#read params in file
sub read_params(file,hash)
{
	my $file=shift;
	my $hash = shift;
	open(I_FILE,"<$file") || die "Could not open $file\n";
	while(<I_FILE>){
		next unless s/^(\w+)\s*//;
		$$hash{$1} = [split];
	}
	close(I_FILE);
}

#read parameters given in the command line 
sub read_inline_params(\%params)
{
	my $param=shift;
	my $nb_warning = 0;
	foreach my $arg (@ARGV)
	{
		SWITCH: for ($arg)
		{
			/-v$/ && do {$$param{"verbose"}=1; print "\n\e[1m[VERBOSE MODE ACTIVATED]\e[0m\n\n"; last;}; #verbose mode ; to leave after all parameters beggining with 'v' in this subroutine
			/-f$/ && do {$$param{"force"}=1;last;};
			#if parameter don't match anything, raise warning
			warn "\e[31m\e[1mERROR:\e[0m\e[31m unkown parameter in expression '$arg'\e[0m\n";
			$nb_warning++;
		}
	}
	if ($nb_warning>0) {
		#left program with error code -1 ...
		usage(-1);
	}
}


#init params when a param isn't defined is the file
# UNUSED
sub init_params(param)
{
	my $param = shift;
	$$param{"ldpc_N1"} = [5];
	$$param{"codec"} = [2];
	$$param{"nb_source_symbols"} = [1000];
	$$param{"nb_repair_symbols"} = [500];
	$$param{"symbol_size"} = [1024];
	$$param{"nb_iterations"} = [10];
	$$param{"nb_iterations_for_partial_results"} = [10000];
	$$param{"tx_type"}= [0];
	$$param{"loss"} = [0,0,0,1];
	$$param{"verbose"} = 0;
}


#check if all necessary parameters are defined, and compatibily between them
sub check_params(params)
{
	my $params = shift;	
	
	#error number
	my $err_found = 0;
	
	#essentials parameters are defined ?
	my $undef_nb = 0;
	my @essentials_params = ("ldpc_N1","codec","nb_source_symbols","symbol_size","nb_iterations","nb_iterations_for_partial_results","tx_type");
	verbose("Checking parameters read from file ...\n",$$params{"verbose"});
	foreach (@essentials_params) {
		unless (defined $$params{$_}) {
			$undef_nb++;
			print "Error:\t$_ not declared in parameters file.\n";
		}
	}
	#particular case : code_rate and nb_repair_symbols ; only one of them must be specified
	my $conc_cr_and_r = 0;
	if (defined $$params{"code_rate"}) {
		$conc_cr_and_r++;
		#verify if no multiple code_rates was given
		if (scalar @{$$params{"code_rate"}}>1) {
			print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tmultiple code rates given. You cannot iterate on code rates, please modify your parameters file.\e[0m\n";
			$err_found++;
		}
	}
	if (defined $$params{"nb_repair_symbols"}) {
		$conc_cr_and_r++;
		#in this case, only 1 nb_source_symbols can be given.
		my $nb_occ = $$params{"nb_source_symbols"}[1] - $$params{"nb_source_symbols"}[0];
		if ($nb_occ > 0) {
			print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tthe source symbols range given is invalid : You cannot iterate on source symbols number and get an nb_repair_symbols. You can :";
			print "\n\t⋅ set a code rate instead of the repair symbols number and keep the source symbols range unchanged.";
			print "\n\t⋅ set a source symbols range like : \"nb_source_symbols ".$$params{"nb_source_symbols"}[0]."\"\e[0m\n";
			$err_found++;
		}
		#only one repair symbols number must be given if nb_repair_symbols id defined
		if (scalar @{$$params{"nb_repair_symbols"}}>1) {
			$err_found++;
			print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tmultiple repair symbols numbers given. You cannot iterate on this value, please modify your parameters file.\e[0m\n";
		}
	}
	
	if ($conc_cr_and_r==0) {
		$undef_nb++;
		print "Error:\tyou must specify code_rate OR nb_repair_symbols in parameters file.\n";
	}
	
	if ($undef_nb > 0) {
		$err_found++;
		print "\n\e[31m\e[1mERROR\e[0m\e[31m: Missing $undef_nb value(s) in parameters file. Please specify them.\e[0m\n";
	}
	
	#other constraints
	#loss percentage must be defined if NOT find_min_overhead
	if ((not ($$params{"find_min_overhead"}[0] eq "true")) and (not defined $$params{"loss"})) {
		$err_found++;
		print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tyou are not in find min overhead mode, please specify a loss percentage.\e[0m\n";
	}
	
	#  manage concurrence between code_rate and nb_repair_symbols
	if ($conc_cr_and_r>1) {
		$err_found++;
		print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tplease check the code_rate and nb_repair_symbols values given in the parameters file.\e[0m\n";
		print "\t\e[31mYou MUST choose either the code_rate or nb_repair_symbols parameter, but NOT both.\e[0m\n";
	}
	
	#  check if nb_source_symbols range is valid (min<=max)
	if ($$params{"nb_source_symbols"}[0] > $$params{"nb_source_symbols"}[1]) {
		$err_found++;
		print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tplease check the source symbols range given : the maximum source symbols number is bigger than minimum given.\e[0m\n";
	}

	#check if eperftool program is reachable
	unless (-e $$params{"fec_tool"}[0]) {
		$err_found++;
		print "\n\e[31m\e[1mERROR\e[0m\e[31m:\tcannot reach eperftool executable at the given path :\n\t $$params{\"fec_tool\"}[0]\e[0m\n";
	}

	#leave program if error found above
	if ($err_found>0) {
		die "\n\n\e[1mExiting with $err_found errors ...\e[0m\n";
	} else {
		verbose("No errors found in parameters.\n",$$params{"verbose"});
	}

	
	#WARNING TESTS : ask if user want to continue, if a warning is found
	
	# if find min overhead is UNabled, loss_percentage is used
	# those values are constrained, so we must check code rate.
	# Conditions :
	#	1) at least 1 iteration : cr < 1-(minimum_loss_percentage/100)
	#		/!\ be careful : this is a necessary condition, but not sufficient
	#		conversely, if cr ≥ 1-(minimum_loss_percentage/100), no possible iteration exists
	#	2) all iterations possibles : cr < 1-(maximum_loss_percentage/100)
	
	my $nb_warning = 0;
	
	if ($$params{"find_min_overhead"}[0] eq "false" ) {
		#foreach my $cr (@{$$params{"code_rate"}}) {
		# Iteration on code rate was deleted
		my $cr = $$params{"code_rate"}[0];
		# get maximum loss percentage
		my $loss_max = $$params{"loss"}[2];
		# checking if all iterations are possibles
		my $threshold = 1-($loss_max/100);
		if ($cr >= $threshold) {
			$nb_warning++;
			print "\n\e[33m\e[1mWARNING\e[0m\e[33m: some data in the parameters file might be incompatibles:\e[0m\n";
			print "\t\e[33mCode rate $cr might be too big for some values in the given loss percentage range.\n";
			print "\t\e[33mFor these values eperftool will probably be unable to perform FEC decoding.\e[0m\n";
			print "\e[1mAdvices\e[0m:\n";
			print "\tMaximum code rate for the loss percentage range given: $threshold\n";
			my $loss_threshold = -100*$cr + 100;
			print "\tMaximum loss percentage for the code rate given: $loss_threshold\n\n";
		}
	}
	
	#check if thread number is correct
	unless($$params{"nb_thread"}>0) {
		$nb_warning++;
		$$params{"nb_thread"} = get_nb_cpu();
		warn "\n\e[33m\e[1mWARNING\e[0m\e[33m: given thread number is incorrect, value set to $$params{\"nb_thread\"}.\e[0m\n";
	}
	
	#warn if database will be erase
	if ($$params{"erase_database"}[0] eq "true") {
		$nb_warning++;
		warn "\n\e[33m\e[1mWARNING\e[0m\e[33m: you will erase existant database (erase_database parameter is set to 'true')\n\tDatabase name : $$params{\"database\"}[1]\n\tServer : $$params{\"database\"}[2]\e[0m\n";
	}
	
	if ($nb_warning > 0) {
		print "\n\n\e[1m$nb_warning warning(s) found(s).\e[0m\n";
		#ask user if he wishes to continue
		if ($$params{"force"}!=1) {
			print "\e[1mContinue anyway ? (y/n)\e[0m   ";
			my $continue = <STDIN>;
			my $cont = lc($continue);
			if (not (lc($continue) eq "y\n" || lc($continue) eq "yes\n")) {
				die "\n\e[31m\e[1mExecution aborted by user.\e[0m\n";
			} 
		} else {
			verbose("Force mode activated: the script will continue.\n",$$params{"verbose"});
		}
		
	} else {
		verbose("Nothing to report in parameters.\n",$$params{"verbose"});
	}
}


#usage
sub usage
{
	my $exit_code = shift;
	#default exit code : no error (code 1)
	unless(defined $exit_code) {
		$exit_code = 1;
	}
	print<<EOM;
Script for managing performance tests with the OpenFEC.org codes/codecs.

\e[1mUsage\e[0m: 
\t$0 <param_file> [-v -f]

\e[1mParams\e[0m:
\t<param_file>        the parameter file describing the simulations to perform
\t-v                  interactive verbose mode
\t-f                  force mode (user answers are forced to yes)

NB: it is essential to know what kind of curve you want to generate before initializing
    the param_file file. See examples and the generate_curve.pl help.
EOM

	exit($exit_code);
}


sub get_last_insert_id(dbh)
{
	my $dbh=shift;
	my $id;
	my $prep;
	my $infos =	$dbh->get_info(2);
	if ($infos =~ /mysql/)
	{ 
		$prep = $dbh->prepare("select last_insert_id();");
	}
	else
	{
		$prep = $dbh->prepare("select last_insert_rowid();");
	}
	$prep->execute() or die "echec";
	$id =  $prep->fetchrow_array;
	$prep->finish();
	return $id;
}


sub get_codec_id(dbh,codec_name)
{
	my $dbh = shift;
	my $codec_name=shift;
	my $id;
	my $prep = $dbh->prepare("select codec_id from codec_table where codec_name=?");
	$prep->execute($codec_name) or warn "unable to find codec_id";
	$id = $prep->fetchrow_array();
	$prep->finish();
	return $id;
}

sub get_tx_id(dbh,tx_name)
{
	my $dbh = shift;
	my $tx_name=shift;
	my $id;
	my $prep = $dbh->prepare("select tx_id from tx_table where tx_name=?");
	$prep->execute($tx_name) or warn "unable to find tx_id";
	$id = $prep->fetchrow_array();
	$prep->finish();
	return $id;
}

sub get_run_id(dbh,k,r,c,l,ss,tx)
{
	my ($dbh,$k,$r,$c,$l,$ss,$tx) = @_;

	my $id;

	my $prep = $dbh->prepare("select run_id from run_table where run_k=? and run_r=? and codec_id=? and run_left_degree=? and run_symbol_size=? and tx_id=?");
	$prep->execute($k,$r,$c,$l,$ss,$tx) or die "unable to find run_id";
	$id = $prep->fetchrow_array();
	$prep->finish();
	return $id;
}

# NOT USED
sub  get_k_and_r_from_matrix_file()
{
	my $file = shift;
	my $verbose = shift;
	my $k=-1;
	my $r=-1;
	my $n=-1;
	open(I_FILE,"<$file") || die "Could not open $file\n";
	$r=<I_FILE>;
	$n=<I_FILE>;
	$r=$r*1; # remove the space before the numerical value. Copyright Mathieu Cunche ;)
	$k=$n-$r;
	close(I_FILE);
	verbose("k=$k r=$r\n",$verbose);
	return ($k,$r);
}

# get number of logical CPU depending of get_os function.
sub get_nb_cpu()
{
	my $os=get_os();
	SWITCH: for($os)
	{
		/linux/ && do { return `cat /proc/cpuinfo |grep "MHz"|wc -l`; last;};
		/darwin/ && do {my @t=split('=',`sysctl hw.availcpu`); return $t[1];last;};
		die "Unsupported platform.\n";
	}
}

#get the name of the current OS
sub get_os()
{
	return $^O;
}

#check if tmp folder exists. Else, create it.
sub check_tmp_folder()
{
	my $verbose = shift;
	verbose("Cheking temporary folder ...\n",$verbose);
	if (opendir(DIR,"./tmp")) {
		# the tmp folder exists
		verbose("Temporary folder exists.\n",$verbose);
		closedir(DIR);
	} else {
		# making it ...
		verbose("Temporary folder doesn't exists. Making it ... ",$verbose);
		mkdir("./tmp");
		verbose("Done.\n",$verbose);
	}
}

sub clear_tmp(nb_tmp_files,trace_file,verbose)
{
	my ($nb_tmp_files,$trace_file,$verbose) = (shift,shift,shift);
	
	# deleting all temporary files
	verbose("Removing tmp files...\r",$verbose);
	for (my $i=0;$i < $nb_tmp_files;$i++)
	{
		print "Removing tmp files...%.2f %%\r", 100*$i/$nb_tmp_files;
		system("cat  ./tmp/tmp_$i.txt  >> $trace_file ");
		system("rm ./tmp/tmp_$i.txt");
	}
	verbose("Removing tmp files... Done\n",$verbose);
	
	#deleting temporary directory
	verbose("Removing tmp directory... ",$verbose);
	if (rmdir "./tmp") {
		verbose("Done.\n",$verbose);
	} else {
		verbose("\n\e[31m\e[1mERROR\e[0m\e[31m: Deletion failure.\e[0m\n\n",$verbose);
	}
}


sub verbose(message,verbose_mode) {
	my ($message,$vm) = (shift,shift);
	if ($vm) {
		print "$message";
	}
}


#DELETED
#subroutine called when SIGINT is received
###sub interruption_handler()
###{
###	print "\e[0m";
###	system("clear");
###	print "\e[1mExecution aborted by user.\e[0m\n";
###	$SIG{INT} = 'DEFAULT';
###	kill KILL => $$;
###	threads->exit(0);
###}



