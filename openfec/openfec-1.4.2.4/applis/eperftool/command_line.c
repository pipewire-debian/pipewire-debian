/* $Id: command_line.c 207 2014-12-10 19:47:50Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2012 INRIA - All rights reserved
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


#include "eperftool.h"


/*
 * Certain variables are linked together, and changing one value through eperftool arguments
 * must be reflected on other variables. Since the eperftool user has no constraint on the
 * order in which these variables are set through the various arguments, they can only be
 * globally updated later on, once argument parsing is over.
 * The goal of the static variables below is to keep in memory which variables have been updated.
 */

static bool	updated_tot_nb_source_symbols = false;
static bool	updated_tot_nb_repair_symbols = false;
static bool	updated_code_rate = false;
static bool	updated_object_size = false;
static bool	updated_symbol_size = false;
static bool     updated_matrix_file = false;



void
printUsage (char *cmdName)
{
	printf("Usage: %s [options]\n", cmdName);
	printf("SOURCE/REPAIR SPECIFICATION METHOD 1\n");
	printf("    -tot_src=<n> or -k=<n>\n");
	printf("                set the total number of source symbols to n (default %d).\n", DFLT_TOT_NB_SOURCE);
	printf("                This is equivalent to k (code dimension) when there is a single block\n");
	printf("    -tot_rep=<n> or -r=<n>\n");
	printf("                set the total number of repair symbols to n (default %d). \n", DFLT_TOT_NB_REPAIR);
	printf("                This is equal to n-k (code length - code dimension) when there is\n");
	printf("                a single block. If -tot_rep/-k is used, -rate must not be used.\n");
	printf("    -rate=<n>   set code rate (i.e. k/n) to n (floating point value) (default %.3f).\n",
			(double)DFLT_TOT_NB_SOURCE / (double)(DFLT_TOT_NB_SOURCE + DFLT_TOT_NB_REPAIR));
	printf("                If -rate is used, -tot_rep/-k must not be used.\n");
	printf("    -symb_sz=<n>\n");
	printf("                set the symbol size to n (default %d)\n", DFLT_SYMBOL_SZ);
	printf("    -obj_sz=<n> set object size to n bytes\n");
	printf("\n");
	printf("GENERAL PURPOSE OPTIONS:\n");
	printf("    -h[elp]     this help\n");
	printf("    -v=<n>      set verbosity to n (only {0, 1, 2} are accepted)\n");
	printf("    -find_min_overhead\n");
	printf("                find the minimum code overhead. This test implies launching several\n");
	printf("                decoding instances, with a number of received packets that is increased,\n");
	printf("                starting from k, until decoding succeeds. Limited to a single block, and\n");
	printf("                -seed must be specified to keep the same code and pkt losses (default: false).\n");
	printf("\n");
	printf("CODE/CODEC RELATED OPTIONS:\n");
	printf("    -codec=<n>  set the code/codec:\n");
#ifdef OF_USE_REED_SOLOMON_CODEC
	printf("                  %d: RS over GF(2^8) stable codec\n", OF_CODEC_REED_SOLOMON_GF_2_8_STABLE);
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
	printf("                  %d: RS over GF(2^m) (with m=4 or 8), stable codec\n", OF_CODEC_REED_SOLOMON_GF_2_M_STABLE);
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
	printf("                  %d: LDPC-Staircase stable codec (default)\n", OF_CODEC_LDPC_STAIRCASE_STABLE);
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
	printf("                  %d: 2D parity check matrix stable codec\n", OF_CODEC_2D_PARITY_MATRIX_STABLE);
#endif
#ifdef  OF_USE_LDPC_FROM_FILE_CODEC
	printf("                  %d: LDPC, with a binary H matrix specified in a file, advanced codec\n", OF_CODEC_LDPC_FROM_FILE_ADVANCED);
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
	printf("    -rs_m=<n>   set the m parameter of RS over GF(2^m) codes to n (default %d)\n", DFLT_RS_M_PARAM);
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
	printf("    -ldpc_N1=<n> set the LDPC N1 parameter to n (default %d)\n", DFLT_LDPC_N1);
	printf("                (only with LDPC-Staircase codecs)\n");
#endif
	printf("    -use_callbacks\n");
	printf("                eperftool allocates memory for decoded source symbols (otherwise the\n");
	printf("                codec allocates it)\n");
	printf("\n");
	printf("TRANSMISSION RELATED OPTIONS:\n");
	printf("    -loss=<n>[:<n1>:<n2>|:<n1>]\n");
	printf("                set the loss model and associated probabilities/numbers when applicable:\n");
	printf("                  -loss=0         no loss (default)\n");
	printf("                  -loss=1 or -loss=1:<n1>:<n2>\n");
	printf("                                  simulate random losses using two state markov model\n");
	printf("                                  n1: loss probability in OK state (integer, default %i)\n", P_LOSS_WHEN_OK);
	printf("                                  n2: success probability in NOK state (integer, default %i)\n", P_SUCCESS_WHEN_LOSSES);
	printf("                  -loss=2:<n1>    simulate random losses by specifying the target loss\n");
	printf("                                  probability expressed in percent (floating point value)\n");
	printf("                  -loss=3:<n1>    simulate random losses by specifying the target number\n");
	printf("                                  of losses (rather than probability)\n");
	printf("                  -loss=4         randomly choose one packet to send out of the n possibles\n");
	printf("                                  at each step, with possible duplicates (overwrites transmission type)\n");
	printf("                  -loss=5:<n1>    simulate reception of the first target number of packets and trim whatever follows\n");
	printf("                                  (WARNING: this is a number of packets to receive, not to erase)\n");
	printf("    -tx_type=<n>\n");
	printf("                set the transmission type:\n");
	printf("                  0: randomly send all source + repair symbols (default)\n");
	printf("                  1: randomly send a few source symbols (not necessarily received) + all repair symbols\n");
	printf("                  2: randomly send few src symbols first (always received), then randomly all repair symbols\n");
	printf("                  3: randomly send only repair symbols (non systematic)\n");
	printf("                  4: sequentially send all src symbols first, then repair symbols\n");
	printf("                  5: sequentially send all repair symbols first, then src symbols\n");
	printf("                  6: sequentially send all src symbols first, then randomly repair symbols\n");
	printf("                  7: sequentially send all repair symbols first, then randomly src symbols\n");
	printf("                  8: simulate broadcast tx + unicast repair: send all repair randomly, subject\n");
	printf("                     to packet losses, then send the first <rx_src_nb> source packets sequentially,\n");
	printf("		     not, subject to any loss (3G connection)\n");
	//printf("                  9: Send source and repair symbol in an interleaved way\n");
	printf("    -seed=<n>   sets the global PRNG seed to n (to reproduce experiments).\n");
	printf("                   Guaranties the same matrix is created (LDPC-*), the same transmission order is\n");
	printf("                   used and the same losses occur\n");
	printf("    -rx_src_ratio=<n>\n");
	printf("                percentage of received source symbols when -t1 or -t2 is used\n");
	printf("    -rx_src_nb=<n>\n");
	printf("                number of received source symbols when -t1 or -t2 is used\n");
#ifdef  READ_MATRIX_FILE
	printf("I/O RELATED OPTIONS:\n");
	printf("    -matrix_file=<file_name>\n");
	printf("                set the path of the file containing the parity check matrix\n");
#endif
	printf("\nExamples:\n");
	printf("* a basic example first, with default code and transmission parameters:\n");
	printf("\t %s -v=1\n", cmdName);
	printf("* two examples, one with Reed-Solomon, the other with LDPC-Staircase\n");
	printf("  (NB: compare the speed...):\n");
	printf("\t %s -codec=1 -tot_src=5000 -tot_rep=2500 -tx_type=0\n", cmdName);
	printf("\t %s -codec=3 -tot_src=5000 -tot_rep=2500 -tx_type=0\n", cmdName);
	printf("* a third example, with LDPC-Staircase, specifying the object size (in this case a big 20MBytes\n");
	printf("  object), the code rate (2/3) and the transmission parameters in such a way to trigger ML\n");
	printf("  decoding) (look at the inefficiency ratio, < 1%%)\n");
	printf("\t %s -codec=3 -obj_sz=20992000 -rate=0.666 -tx_type=0 -loss=2:33.0 -seed=1\n", cmdName);
	exit(0);
}


/* Parse the command line and its options */
of_status_t	parse_command_line (int argc, char *argv[])
{
	int	c;
	char *OptList = "c:f:k:r:l:s:t:u:v:h:o:m:n:";
#if defined(WIN32)
	char *optarg = NULL;
#endif

	if (argc < 1)
		printUsage(argv[0]);

#ifdef WIN32
	while ((c = GetOption(argc, argv, OptList, &optarg)) != 0)
#else
	while ((c = getopt(argc, argv, OptList)) != EOF)
#endif
	{
		switch (c) {
		case 'c':
			if (!strncmp(optarg, "odec=", 5) && isdigit((int)*(optarg + 5))) {
				codec_id = (of_codec_id_t)atoi(optarg + 5);
			} else {
				OF_PRINT_ERROR(("bad argument -c%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'f':
			if (!strncmp(optarg, "ind_min_overhead", 16)) {
				find_min_overhead_mode = true;
			} else {
				OF_PRINT_ERROR(("bad argument -f%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'k':
			if (!strncmp(optarg, "=", 1) && isdigit((int)*(optarg + 1))) {
				tot_nb_source_symbols	= atoi(optarg + 1);
				updated_tot_nb_source_symbols = true;
			} else {
				OF_PRINT_ERROR(("bad argument -k%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'r':
			if (!strncmp(optarg, "=", 1) && isdigit((int)*(optarg + 1))) {
				tot_nb_repair_symbols	= atoi(optarg + 1);
				updated_tot_nb_repair_symbols = true;
			} else if (!strncmp(optarg, "ate=", 4) && isdigit((int)*(optarg + 4))) {
					code_rate	= atof(optarg + 4);
					fec_ratio	= 1.0 / code_rate;
					updated_code_rate = true;
			} else if (!strncmp(optarg, "s_m=", 4) && isdigit((int)*(optarg + 4))) {
					rs_m_param	= atoi(optarg + 4);
			} else if (!strncmp(optarg, "x_src_ratio=", 12) && isdigit((int)*(optarg + 12))) {
				src_pkt_ratio		= atoi(optarg + 12);
				use_src_pkt_ratio	= true;
			} else if (!strncmp(optarg, "x_src_nb=", 9) && isdigit((int)*(optarg + 9))) {
				src_pkt_nb		= atoi(optarg + 9);
				use_src_pkt_ratio	= false;
			} else {
				OF_PRINT_ERROR(("bad argument -r%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'l':
			if (!strncmp(optarg, "oss=0", 5)) {
				loss_model	= 0;
			} else if (!strncmp(optarg, "oss=1", 5)) {
				char	tmp_str[256];
				char		*p;
				strncpy(tmp_str, optarg + 5, sizeof(tmp_str));
				if ((p = strchr(tmp_str, ':')) != NULL) {
					*p = '\0';
					p_success_when_losses = (double)atoi(p + 3);
					p_loss_when_ok = (double) atoi(tmp_str);
				} /* else keep the default values */
				loss_model	= 1;
			} else if (!strncmp(optarg, "oss=2:", 6) && isdigit((int)*(optarg + 6))) {
				loss_model	= 2;
				p_loss		= atof(optarg + 6);
			} else if (!strncmp(optarg, "oss=3:", 6) && isdigit((int)*(optarg + 6))) {
				loss_model	= 3;
				nb_loss		= atoi(optarg + 6);
			} else if (!strncmp(optarg, "oss=4", 5)) {
				loss_model	= 4;
			} else if (!strncmp(optarg, "oss=5:", 6) && isdigit((int)*(optarg + 6))) {
				loss_model	= 5;
				trim_after_this_nb_rx_pkts	= atoi(optarg + 6);
			} else if (!strncmp(optarg, "dpc_N1=", 7) && isdigit((int)*(optarg + 7))) {
				ldpc_N1 = atoi(optarg + 7);
			} else {
				OF_PRINT_ERROR(("bad argument -l%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;
#ifdef READ_MATRIX_FILE
		case 'm':
			if (!strncmp(optarg, "atrix_file=", 11) ) {
				char	tmp_str[1024];

				strncpy(tmp_str, optarg+11, sizeof(tmp_str));
				printf("tmp_str : \"%s\" \n", tmp_str);
				ldpc_matrix_file_name = (char*)malloc(sizeof(tmp_str));
				strncpy(ldpc_matrix_file_name, tmp_str, sizeof(tmp_str));
				updated_matrix_file = true;
			} else {
				OF_PRINT_ERROR(("bad argument -m%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;
#endif /* READ_MATRIX_FILE */

		case 's':
			if (!strncmp(optarg, "ymb_sz=", 1) && isdigit((int)*(optarg + 7))) {
				symbol_size	= atoi(optarg + 7);
				updated_symbol_size = true;
			} else if (!strncmp(optarg, "eed=", 4) && isdigit((int)*(optarg + 4))) {
				suggested_seed	= atoi(optarg + 4);
			} else {
				OF_PRINT_ERROR(("bad argument -s%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 't':
			if (!strncmp(optarg, "ot_src=", 7) && isdigit((int)*(optarg + 7))) {
				tot_nb_source_symbols	= atoi(optarg + 7);
				updated_tot_nb_source_symbols = true;
			} else if (!strncmp(optarg, "ot_rep=", 7) && isdigit((int)*(optarg + 7))) {
				tot_nb_repair_symbols	= atoi(optarg + 7);
				updated_tot_nb_repair_symbols = true;
			} else if (!strncmp(optarg, "x_type=", 7) && isdigit((int)*(optarg + 7))) {
				//if (loss_model != 2)
				tx_mode = (tx_mode_t)atoi(optarg + 7);
			} else {
				OF_PRINT_ERROR(("bad argument -t%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;
		case 'u':
			if (!strncmp(optarg, "se_callbacks", 12) ) {
				use_callback = true;
			}
			break;
		case 'v':
			if (!strncmp(optarg, "=", 1) && isdigit((int)*(optarg + 1))) {
				of_verbosity = atoi(optarg + 1);
				if (of_verbosity != 0 && of_verbosity != 1 && of_verbosity != 2) {
					OF_PRINT_ERROR(("bad argument -v%s, value out of range (only {0, 1, 2} are accepted)\n", optarg))
					return OF_STATUS_FATAL_ERROR;
				}
			} else {
				OF_PRINT_ERROR(("bad argument -v%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'o':
			if (!strncmp(optarg, "bj_sz=", 6) && isdigit((int)*(optarg + 6))) {
				object_size = atoi(optarg + 6);
				updated_object_size = true;
			} else {
				OF_PRINT_ERROR(("bad argument -o%s\n", optarg))
				return OF_STATUS_FATAL_ERROR;
			}
			break;

		case 'h':
			printUsage(argv[0]);
			break;

		default:
			/*
			 * NB: getopt returns '?' when finding an
			 * unknown argument; avoid the following
			 * error msg in that case
			 */
			if (c != '?') {
				fprintf(stderr, "ERROR, bad argument\n");
			}
			printUsage(argv[0]);
			break;
		}
	}
	return OF_STATUS_OK;
}


of_status_t
finish_init_command_line_params ()
{
	/* check a few erroneous combinations first, for sanity purposes */
	if (updated_tot_nb_source_symbols && updated_object_size) {
		OF_PRINT_ERROR(("ERROR: cannot specify both tot_nb_source_symbols and object_size\n"))
		goto error;
	}
	if (updated_tot_nb_repair_symbols && updated_object_size) {
		OF_PRINT_ERROR(("ERROR: cannot specify both updated_tot_nb_repair_symbols and object_size\n"))
		goto error;
	}
	if (updated_tot_nb_repair_symbols && updated_code_rate) {
		OF_PRINT_ERROR(("ERROR: cannot specify both tot_nb_repair_symbols and code_rate\n"))
		goto error;
	}
	if (updated_tot_nb_repair_symbols && updated_code_rate) {
		OF_PRINT_ERROR(("ERROR: cannot specify both tot_nb_repair_symbols and code_rate\n"))
		goto error;
	}
	if (updated_matrix_file && (updated_code_rate || updated_tot_nb_source_symbols || updated_tot_nb_repair_symbols || updated_object_size || updated_code_rate  )) {
		// TODO: fix this by checking if the value specified match the dimension of the matrix
	/* 	OF_PRINT_ERROR(("ERROR: cannot specify matrix_file with tot_nb_source_symbols or tot_nb_repair_symbols or code_rate or object_size  \n")) */
/* 		goto error; */
	}
	if (updated_symbol_size && (symbol_size == 0)) {
		OF_PRINT_ERROR(("ERROR: invalid symbol size 0 (must be >= 1).\n"))
		goto error;
	}
	if (updated_code_rate && ((code_rate <= 0.0) || (code_rate > 1.0))) {
		OF_PRINT_ERROR(("ERROR: invalid code rate %f<n>\n", code_rate))
		goto error;
	}
	if (updated_object_size && !(object_size % symbol_size == 0))
	{
		OF_PRINT_ERROR(("ERROR: object size (%d) must be a multiple of symbol size (%d).\n", object_size, symbol_size))
		goto error;
	}
	/* set the set of linked variables now, depending on what the user has specified */
	if (updated_object_size) {
		tot_nb_source_symbols	= (UINT32)((double)object_size / (double)symbol_size);
		tot_nb_encoding_symbols	= (UINT32)ceil((double)tot_nb_source_symbols / code_rate);
		tot_nb_repair_symbols	= tot_nb_encoding_symbols - tot_nb_source_symbols;
		code_rate		= (double)tot_nb_source_symbols / (double)tot_nb_encoding_symbols;
	} else if (updated_tot_nb_source_symbols) {
		tot_nb_encoding_symbols	= tot_nb_source_symbols + tot_nb_repair_symbols;
		object_size		= tot_nb_source_symbols * symbol_size;
		code_rate		= (double)tot_nb_source_symbols / (double)(tot_nb_encoding_symbols);
	}
#ifdef READ_MATRIX_FILE
	else if (updated_matrix_file) {
		/* TODO read in the matrix file */

		if(of_get_pck_matrix_dimensions_from_file(ldpc_matrix_file_name, &tot_nb_repair_symbols, &tot_nb_encoding_symbols) != OF_STATUS_OK){
			OF_PRINT_ERROR(("ERROR: cannot read pck matrix dimension from file\n"))
			goto error;
		}
		tot_nb_source_symbols	= tot_nb_encoding_symbols - tot_nb_repair_symbols;
		object_size		= tot_nb_source_symbols * symbol_size;
		code_rate		= (double)tot_nb_source_symbols / (double)(tot_nb_encoding_symbols);
	}
#endif /* READ_MATRIX_FILE */
	if (updated_symbol_size && !updated_tot_nb_source_symbols && !updated_object_size) {
		object_size		= tot_nb_source_symbols * symbol_size;
	}
	if (updated_code_rate && !updated_tot_nb_source_symbols && !updated_object_size) {
		tot_nb_encoding_symbols	= (UINT32)ceil((double)tot_nb_source_symbols / code_rate);
		tot_nb_repair_symbols	= tot_nb_encoding_symbols - tot_nb_source_symbols;
	}
	if (updated_tot_nb_repair_symbols && !updated_tot_nb_source_symbols && !updated_object_size) {
		tot_nb_encoding_symbols	= tot_nb_source_symbols + tot_nb_source_symbols;
		code_rate		= (double)tot_nb_source_symbols / (double)(tot_nb_encoding_symbols);
	}
	fec_ratio = 1.0 / code_rate;
	if (find_min_overhead_mode == true) {
		/* when eperftool is used iteratively in order to find the mininum decoding overhead,
		 * start with k, assuming there's a single block... */
		trim_after_this_nb_rx_pkts = tot_nb_source_symbols;
		if (suggested_seed == 0) {
			OF_PRINT_ERROR(("ERROR: seed must be specified in find_min_overhead mode\n"))
			goto error;
		}
	}
	OF_PRINT(("tot_nb_source_symbols=%i  tot_nb_repair_symbols=%i  symbol_size=%i  ldpc_N1=%i  rs_m=%i\n",
		tot_nb_source_symbols, tot_nb_repair_symbols, symbol_size, ldpc_N1, rs_m_param))
	OF_PRINT(("codec_id=%d\n",codec_id))

	switch (tx_mode)
	{
		case 0:
			OF_PRINT(("transmission_type=randomly_send_all_source_and_repair_symbols\n"))
			break;
		case 1:
			OF_PRINT(("transmission_type=randomly_send_a_few_source_symbols_and_repair_symbols\n"))
			break;
		case 2:
			OF_PRINT(("transmission_type=randomly_send_a_few_src_symbols_first_then_randomly_all_repair_symbols\n"))
			break;
		case 3:
			OF_PRINT(("transmission_type=randomly_send_only_repair_symbols\n"))
			break;
		case 4:
			OF_PRINT(("transmission_type=sequentially_send_all_src_symbols_first_then_repair_symbols\n"))
			break;
		case 5:
			OF_PRINT(("transmission_type=sequentially_send_all_repair_symbols_first_then_src_symbols\n"))
			break;
		case 6:
			OF_PRINT(("transmission_type=sequentially_send_all_src_symbols_first_then_randomly_src_symbols\n"))
			break;
		case 7:
			OF_PRINT(("transmission_type=sequentially_send_all_repair_symbols_first_then_randomly_src_symbols\n"))
			break;
		case 8:
			OF_PRINT(("simulate broadcast transmission with unicast repair\n"))
			break;
	}
	/* Warning: the tot_nb_encoding_symbols depends on the blocking structure, which means
	 * it will probably change... */
	return OF_STATUS_OK;

error:
	return OF_STATUS_ERROR;
}

