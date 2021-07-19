/* $Id: globals.c 199 2014-10-21 14:25:02Z roca $ */
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

/* AL-FEC extended performance evaluation tool */

#include "eperftool.h"


/*
 * Global variables.
 */

/** Total number of source symbols for the object.  It is equal to k when there
 *  is a single source block for the whole object. */
UINT32	tot_nb_source_symbols	= DFLT_TOT_NB_SOURCE;

/** Total number of repair symbols for the object.  It is equal to n-k when there
 *  is a single source block for the whole object. */
UINT32	tot_nb_repair_symbols	= DFLT_TOT_NB_REPAIR;

/** Total number of encoding symbols for the object.  It is equal to n when there
 *  is a single source block for the whole object. */
UINT32	tot_nb_encoding_symbols	= DFLT_TOT_NB_SOURCE + DFLT_TOT_NB_REPAIR;

/** Symbol size, in bytes. */
UINT32	symbol_size	= DFLT_SYMBOL_SZ;

/** The object size, when considered globally. */
UINT32	object_size	= DFLT_TOT_NB_SOURCE * DFLT_SYMBOL_SZ;

/** The target code rate, equal to k/n for a given block. */
double	code_rate	= (double)DFLT_TOT_NB_SOURCE / (double)(DFLT_TOT_NB_SOURCE + DFLT_TOT_NB_REPAIR);
double	fec_ratio	= (double)(DFLT_TOT_NB_SOURCE + DFLT_TOT_NB_REPAIR) / (double)DFLT_TOT_NB_SOURCE;


/*
 * Code/codec specific global variables.
 */

/** Target M parameter for RS codes over GF(2^M) (See RFC 5510). */
UINT16	rs_m_param = DFLT_RS_M_PARAM;

/** Target number of 1s per column in H1, the left part of the LDPC H matrix (See RFC 5170). */
UINT32	ldpc_N1 = DFLT_LDPC_N1;

/** Filename that contains the whole H matrix (LDPC-*) or the quasi-cyclic part (QC-LDPC). */
char	*ldpc_matrix_file_name = NULL;

/** Identifies the FEC code/codec being used. */
of_codec_id_t	codec_id = OF_CODEC_LDPC_STAIRCASE_STABLE;

tx_mode_t	tx_mode	= TX_MODE_ALL_RANDOM; /* transmission mode */

UINT32	suggested_seed	= 0;		  /* by default seed is choosen randomly*/

INT32	max_decoding_steps = -1;

bool	find_min_overhead_mode = false;

//INT32	find_min_overhead_nb_rx_pkts = -1;

bool use_callback=false; /* by default, we don't use callbacks */

/*
 * Control the number of source symbols sent
 */
bool	use_src_pkt_ratio= true;/* if use_src_pkt_ratio is true, then use the value of src_pkt_ratio instead of
				 * the value of src_pkt_nbr */
UINT32	src_pkt_ratio	= 20;	/* % of source data packets are sent default value */
UINT32	src_pkt_nb	= 0;	/* % of source data packets are sent   */

UINT32	loss_model 	= 0;
UINT32	tx_simul_loss	= 0;	/* number of simulated losses */
INT32	trim_after_this_nb_rx_pkts	= -1;


double	p_loss_when_ok 	= P_LOSS_WHEN_OK;
double	p_success_when_losses	= P_SUCCESS_WHEN_LOSSES;

double	p_loss = P_LOSS;
UINT32	nb_loss = NB_LOSS;


/*
 * timers used for processing time evaluation.
 */
#ifdef WIN32
LARGE_INTEGER	tv0;		/* start */
LARGE_INTEGER	tv1;		/* end */
LARGE_INTEGER	freq;
#else
struct timeval	tv0;		/* start */
struct timeval	tv1;		/* end */
struct timeval	tv_delta;	/* tv1 - tv0 difference */
#endif	// OS_DEP

/* blocking variables */
of_blocking_struct_t	bs;
UINT32			max_k;
UINT32			max_n;
UINT32			tot_nb_blocks;
UINT32			k_for_this_blk;


/**
 * orig_symb: table of all symbols (source/repair), in sequential order, with
 * 	all source symbols first, followed by all repair symbols. Each entry
 * 	contains a pointer to the associated symbol buffer. This list remains
 *	the same regardless whether there's a single or multiple source blocks.
 *
 *              0     tot_nb_source_symbols      tot_nb_repair_symbols
 * orig_symb:   [---------------[--------------------------[--> index numbers
 *              ^    ^    ^     ^    ^    ^
 *              |    |    |     |    |    |
 *              blk0_src  |     blk0_repair
 *                   blk1_src        blk1_repair
 *                        blk2_src        blk2_repair
 *
 * symb_tab: table of all symbol control structures. Follows the same
 * 	organization as orig_symb.
 *	Two pointers/index are used:
 * 	 - pkt/pkt_idx (current location/index of source pkt struct)
 * 	 - fec_pkt/fec_idx (current location/index of fec pkt struct)
 *
 * NB: symbol seq numbers (source block number, or SBN for short, and Encodingsymbol ESI in the block) are different
 * 	 from the packet indexes.
 */
char	**orig_symb;

/**
 *
 */
char	**avail_symb;

/**
 * Table containing the block "control block" structures.
 * Each entry describes the associated block.
 * In theory, this table is constructed independantly by both the sender and the receiver,
 * thanks to the FEC Object Transmission Information (FEC OTI) information carried between
 * them. In practice, in eperf_tool, we simply construct the table at the sender and give
 * it to the receiver too.
 */
block_cb_t	*blk_cb_tab;

/**
 * Table containing the symbol "control block" structures.
 * Each entry describes the associated symbol.
 * In theory, this table is constructed independantly by both the sender and the receiver,
 * thanks to the FEC Object Transmission Information (FEC OTI) information carried between
 * them. In practice, in eperf_tool, we simply construct the table at the sender and give
 * it to the receiver too.
 */
symbol_cb_t	*symb_cb_tab;

