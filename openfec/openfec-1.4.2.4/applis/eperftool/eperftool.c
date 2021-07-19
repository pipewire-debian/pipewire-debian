/* $Id: eperftool.c 199 2014-10-21 14:25:02Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2011 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */


/* AL-FEC extended performance evaluation tool */

#include "eperftool.h"


static int print_params();


int
main   (int	argc,
	char	**argv )
{
	of_status_t	ret;
	INT32		dec_status;	/** decoding status: -1 in case of fatal error,
					 * 0 if decoding successful, 1 if decoding failed. */

//sleep(5); // for leaks analysis only...

#ifdef WIN32
	QueryPerformanceFrequency(&freq);	/* finish time variable init */
#endif
	print_preamble(argv[0]);
	ret = parse_command_line(argc, argv);
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, parse_command_line() failed\n"))
		goto error;
	}
	ret = finish_init_command_line_params();
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, finish_init_command_line_params() failed\n"))
		goto error;
	}
	if (of_verbosity > 0) {
		print_params();
	}
	if (find_min_overhead_mode == false) {
		dec_status = start_enc_dec_test ();
		if (dec_status < 0) {
			goto error;
		}
	} else {
#define DEFAULT_INCREMENT_TO_FIND_MIN_OVERHEAD		50
		INT32		lower_bound_with_failure;	/* highest known value of the nb of rcvd pkts that made decoding to fail */
		INT32		upper_bound_with_success;	/* lowest known value of the nb of rcvd pkts that enabled decoding */
		int		bak, new;			/* resp. the backup and new stdin file descriptors */
		
		/* use eperftool iteratively to find the mininum decoding overhead.
		 * We start with k received symbols and assume there's a single block.
		 * The algorithm used to find this minimum overhead consists in testing
		 * overhead values spaced by DEFAULT_INCREMENT_TO_FIND_MIN_OVERHEAD,
		 * and when the interval is found, to go further by sequentially increasing
		 * the overhead (start at lower_bound_with_failure + 1, up to a maximum of
		 * upper_bound_with_success, stop earlier if the minimum overhead is below.
		 * It's a better strategy than a dichotomic search as there's a high
		 * probability that the overhead is low.
		 *
		 * Note that the tx model and the loss model are both applied BEFORE we
		 * search for the minimum overhead. Said differently, given the ordered
		 * set of symbols that arrive at a receiver, we determine when we can stop
		 * considering received symbols while having a successful decoding.
		 *
		 * If of_verbosity is 0, no matter whether in Debug or Release mode,
		 * we suppress all the traces to stdout while searching the min overhead.
		 * In that case we only print the final traces, once the minimum overhead
		 * has been found.
		 *
		 * NB: make sure there is a single block, otherwise results are hazardous.
		 *
		 * NB: the algorithm can be simulated "by hand", using the -loss=5:nb
		 *     parameter (used to initialize the trim_after_this_nb_rx_pkts variable)
		 *     and increasing progressively the provided number.
		 */
		ASSERT(trim_after_this_nb_rx_pkts > 0);
		/* step 1: fast search, using big jumps */
		lower_bound_with_failure = tot_nb_source_symbols - 1;
		upper_bound_with_success = -1;
		for (trim_after_this_nb_rx_pkts = tot_nb_source_symbols; ; ) {
			OF_PRINT(("===> (1) test with %d recvd symbols (overhead %d) and %d lost symbols... ",
					trim_after_this_nb_rx_pkts, trim_after_this_nb_rx_pkts - tot_nb_source_symbols,
					tot_nb_encoding_symbols - trim_after_this_nb_rx_pkts))
			if (of_verbosity == 0) {
				/* switch off stdout, sending everything to /dev/null, temporarilly */
				fflush(stdout);
				bak = dup(1);
				new = open("/dev/null", O_WRONLY);
				dup2(new, 1);
				close(new);
			}
			/* launch the test now... */
			dec_status = start_enc_dec_test ();
			if (of_verbosity == 0) {
				/* switch on stdout, using the saved value */
				fflush(stdout);
				dup2(bak, 1);
				close(bak);
				OF_PRINT(("\t%s\n", dec_status == 0 ? "OK" : "failed"))
			} else {
				OF_PRINT(("\n"))
			}
			if (dec_status == 0) {
				/* decoding is successful, stop the test */
				upper_bound_with_success = trim_after_this_nb_rx_pkts;
				break;
			}
			if (dec_status < 0) {
				goto error;
			}
			/* else this number of packets is just not sufficient. Continue... */
			lower_bound_with_failure = trim_after_this_nb_rx_pkts;
			if (trim_after_this_nb_rx_pkts == tot_nb_encoding_symbols) {
				/* no test succeded, even after receiving all symbols! That's an error... */
				/* nb: max_decoding_steps is not yet known... Use tot_nb_encoding_symbols instead */
				OF_PRINT_ERROR(("ERROR, all tests failed to find min overhead, even after receiving %d symbols\n",
					trim_after_this_nb_rx_pkts))
				goto error;
			}
			/* increment the number of packets to test, but no more than what is available */
			trim_after_this_nb_rx_pkts += DEFAULT_INCREMENT_TO_FIND_MIN_OVERHEAD;
			trim_after_this_nb_rx_pkts = min(trim_after_this_nb_rx_pkts, tot_nb_encoding_symbols);
			/* nb: max_decoding_steps is not yet known... Use tot_nb_encoding_symbols instead */
		}
		/* step 2: refine, from the first number where it's known to fail, incrementing one by one */
		ASSERT(upper_bound_with_success != -1);
		if (upper_bound_with_success == lower_bound_with_failure + 1) {
			/* we found the minimum overhead, nothing else to do */
			ASSERT(trim_after_this_nb_rx_pkts == upper_bound_with_success);
		} else {
			for (trim_after_this_nb_rx_pkts = lower_bound_with_failure + 1;
			     trim_after_this_nb_rx_pkts <= upper_bound_with_success;
			     trim_after_this_nb_rx_pkts++) {
				OF_PRINT(("===> (2) test with %d recvd symbols (overhead %d) and %d lost symbols... ",
						trim_after_this_nb_rx_pkts, trim_after_this_nb_rx_pkts - tot_nb_source_symbols,
						tot_nb_encoding_symbols - trim_after_this_nb_rx_pkts))
				if (of_verbosity == 0) {
					/* switch off stdout, sending everything to /dev/null, temporarilly */
					fflush(stdout);
					bak = dup(1);
					new = open("/dev/null", O_WRONLY);
					dup2(new, 1);
					close(new);
				}
				/* launch the test now... */
				dec_status = start_enc_dec_test ();
				if (of_verbosity == 0) {
					/* switch on stdout, using the saved value */
					fflush(stdout);
					dup2(bak, 1);
					close(bak);
					OF_PRINT(("\t%s\n", dec_status == 0 ? "OK" : "failed"))
				} else {
					OF_PRINT(("\n"))
				}
				if (dec_status == 0) {
					/* decoding is successful, stop the test */
					break;
				}
				if (dec_status < 0) {
					goto error;
				}
			}
		}
		if (of_verbosity == 0) {
			/* do one more time to record the output to stdout */
			dec_status = start_enc_dec_test ();
			ASSERT(dec_status == 0);
		}
#if 0
		/* use eperftool iteratively to find the mininum decoding overhead. */
		ASSERT(trim_after_this_nb_rx_pkts > 0);
		while (trim_after_this_nb_rx_pkts <= tot_nb_encoding_symbols) {
			/* nb: in above test, max_decoding_steps is not yet known...
			 * Use the tot_nb_encoding_symbols variable instead */
			OF_PRINT(("===> test with %d recvd symbols and %d lost symbols...\n",
					trim_after_this_nb_rx_pkts, tot_nb_encoding_symbols - trim_after_this_nb_rx_pkts))
			dec_status = start_enc_dec_test ();
			if (dec_status == 0) {
				/* decoding is successful, stop the test */
				break;
			}
			if (dec_status < 0) {
				goto error;
			}
			trim_after_this_nb_rx_pkts++;
		}
#endif
	}
	return 0;

error:
	OF_PRINT(("decoding_status=2\n"))
	return -1;
}


/** returns the decoding status: -1 in case of fatal error, 0 if decoding successful, 1 if decoding failed. */
int
start_enc_dec_test ()
{
	of_status_t	ret;
	INT32		dec_status = -1;	/** decoding status: -1 in case of fatal error,
						 * 0 if decoding successful, 1 if decoding failed. */

	ret = init_sender();
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, init_sender() failed\n"))
		goto error;
	}
	ret = init_receiver();
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, init_receiver() failed\n"))
		goto error;
	}
	ret = init_tx_simulator();	/* must be done after init_sender */
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, init_tx_simulator() failed\n"))
		goto error;
	}
#if 0
#ifdef OF_DEBUG
	/* Warning, this function is not compatible with a normal use of eperftool...
	 * Said differently, do not expect to be able to decode afterwards with the
	 * standard receive_and_decode() function. Reason is that get_next_symbol_received
	 * is not re-entrant. */
	print_params();
	print_rx_stats();
#endif /* OF_DEBUG */
#endif
	ret = encode();
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, encode() failed\n"))
		goto error;
	}
//return;
	ret = receive_and_decode();
	switch (ret)
	{
	case OF_STATUS_ERROR:
	case OF_STATUS_FATAL_ERROR:
		close_tx_simulator();
		OF_PRINT_ERROR(("ERROR, receive_and_decode() failed\n"))
		goto error;
	case OF_STATUS_FAILURE:
		OF_PRINT(("eperf_tool: decoding failure\n")) /* do not change message, automatic tests depend on it */
		OF_PRINT(("decoding_status=1\n"))
		dec_status = 1;
		break;
	case OF_STATUS_OK:
		OF_PRINT(("eperf_tool: decoding ok\n")) /* do not change message, automatic tests depend on it */
		OF_PRINT(("decoding_status=0\n"))
		dec_status = 0;
		break;
	}
	ret = close_tx_simulator();
	if (ret != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR, close_tx_simulator() failed\n"))
		goto error;
	}
//sleep(10); // for leaks analysis only...
	return dec_status;

error:
	OF_PRINT(("decoding_status=2\n"))
	return -1;
}


void
print_preamble (char *command_line)
{
	char		*version;	/* pointer to version string */
	char		*copyrights;	/* pointer to copyrights string */

	OF_PRINT(("%s", command_line))
	OF_PRINT(("eperf_tool: an extended AL-FEC performance evaluation tool\n"))
	/* NB: since session pointer is null, we only get a generic string for the
	 * OpenFEC.org project, not specific to the codec itself */
	of_more_about((of_session_t*)NULL, &version, &copyrights);
	OF_PRINT(("%s\n", version))
}


static int
print_params(void)
{
	OF_TRACE_LVL(1, ("eperftool params:\n"))
	OF_TRACE_LVL(1, ("\tcodec_id:\t\t%i\n", codec_id))
	OF_TRACE_LVL(1, ("\ttot_nb_source_symbols:\t%i\n", tot_nb_source_symbols))
	OF_TRACE_LVL(1, ("\ttot_nb_repair_symbols:\t%i\n", tot_nb_repair_symbols))
	OF_TRACE_LVL(1, ("\tcode_rate:\t\t%f\n", code_rate))
	OF_TRACE_LVL(1, ("\tfec_ratio:\t\t%f\n", fec_ratio))
	OF_TRACE_LVL(1, ("\tsrc_pkt_ratio:\t\t%i\n", src_pkt_ratio))
	OF_TRACE_LVL(1, ("\tuse_src_pkt_ratio:\t%s\n", use_src_pkt_ratio ? "true" : "false"))
	OF_TRACE_LVL(1, ("\tsrc_pkt_nb:\t\t%i\n", src_pkt_nb))
	OF_TRACE_LVL(1, ("\tobject_size:\t\t%i bytes\n", object_size))
	OF_TRACE_LVL(1, ("\tsymbol_size:\t\t%i bytes\n", symbol_size))
	OF_TRACE_LVL(1, ("\tsuggested_seed:\t\t%i\n", suggested_seed))
	OF_TRACE_LVL(1, ("\tloss_model:\t\t%i\n", loss_model))
	OF_TRACE_LVL(1, ("\tldpc_N1:\t\t%i\n", ldpc_N1))
	OF_TRACE_LVL(1, ("\tp_success_when_losses:\t%f\n", p_success_when_losses))
	OF_TRACE_LVL(1, ("\tp_loss_when_ok:\t\t%f\n", p_loss_when_ok))
	OF_TRACE_LVL(1, ("\tp_loss:\t\t\t%f\n", p_loss))
	OF_TRACE_LVL(1, ("\tnb_loss:\t\t%i\n", nb_loss))
	OF_TRACE_LVL(1, ("\ttrim_after_this_nb_rx_pkts:\t\t%i\n", trim_after_this_nb_rx_pkts))
	return 0;
}

