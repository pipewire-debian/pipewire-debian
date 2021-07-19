/* $Id: globals.h 199 2014-10-21 14:25:02Z roca $ */
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


#ifndef	GLOBALS_H	/* { */
#define	GLOBALS_H

/*
 * Global variables.
 */

/** Total number of source symbols for the object.  It is equal to k when there
 *  is a single source block for the whole object. */
extern UINT32	tot_nb_source_symbols;

/** Total number of repair symbols for the object.  It is equal to n-k when there
 *  is a single source block for the whole object. */
extern UINT32	tot_nb_repair_symbols;

/** Total number of encoding symbols for the object.  It is equal to n when there
 *  is a single source block for the whole object. */
extern UINT32	tot_nb_encoding_symbols;

/** Symbol size, in bytes. */
extern UINT32	symbol_size;

/** The object size, when considered globally. */
extern UINT32	object_size;

/** The target code rate, equal to k/n for a given block. */
extern double	code_rate;
extern double	fec_ratio;


/*
 * Code/codec specific global variables.
 */

/** Target M parameter for RS codes over GF(2^M) (See RFC 5510). */
extern UINT16	rs_m_param;

/** Target number of 1s per column in H1, the left part of the LDPC H matrix (See RFC 5170). */
extern UINT32	ldpc_N1;

/** Filename that contains the whole H matrix (LDPC-*) or the quasi-cyclic part (QC-LDPC). */
extern char	*ldpc_matrix_file_name;

extern UINT32 qc_ldpc_exp_factor;

/** Identifies the FEC code/codec being used. */
extern of_codec_id_t	codec_id;

/** Transmission mode. */
extern tx_mode_t	tx_mode;

/** By default seed is choosen randomly. */
extern UINT32	suggested_seed;

/** Number of symbols received, once erasures have been applied. */
extern INT32	max_decoding_steps;

/** false by default, true if eperftool is used iteratively in order to find the mininum
 * decoding overhead. */
extern bool	find_min_overhead_mode;

/** when eperftool is used so as to find the mininum decoding overhead, the current nb of
 * received symbols to test. */
//extern INT32	find_min_overhead_nb_rx_pkts;

/** if use_callback is true, then eperftool uses callbacks from the OpenFEC library */
extern bool	use_callback;

/*
 * Control the number of source symbols sent
 */
/* if use_src_pkt_ratio is true, then use the value of src_pkt_ratio instead of the value of src_pkt_nbr */
extern bool	use_src_pkt_ratio;

/* % of source data packets are sent default value */
extern UINT32	src_pkt_ratio;

/* % of source data packets are sent   */
extern UINT32	src_pkt_nb;

/** */
extern UINT32	loss_model;

/** number of simulated losses */
extern UINT32	tx_simul_loss;

/** number of packets to receive, and trim whatever follows.
 * Used both by -loss=5 mode and in find_min_overhead mode */
extern INT32	trim_after_this_nb_rx_pkts;


extern double	p_loss_when_ok;
extern double	p_success_when_losses;

#define P_LOSS 0
extern double	p_loss;
#define NB_LOSS 0
extern UINT32	nb_loss;


/*
 * timers used for processing time evaluation.
 */
#ifdef WIN32
extern LARGE_INTEGER		tv0;		/* start */
extern LARGE_INTEGER		tv1;		/* end */
extern LARGE_INTEGER		freq;
#else
extern struct timeval		tv0;		/* start */
extern struct timeval		tv1;		/* end */
extern struct timeval		tv_delta;	/* tv1 - tv0 difference */
#endif	// OS_DEP

/* blocking variables */
extern of_blocking_struct_t	bs;
extern UINT32			max_k;
extern UINT32			max_n;
extern UINT32			tot_nb_blocks;


/* valgrind profiling requires including vg_profile.c */
//#include "vg_profile.c"

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
extern char	**orig_symb;

/**
 *
 */
extern char	**avail_symb;

/**
 * Table containing the block "control block" structures.
 * Each entry describes the associated block.
 * In theory, this table is constructed independantly by both the sender and the receiver,
 * thanks to the FEC Object Transmission Information (FEC OTI) information carried between
 * them. In practice, in eperf_tool, we simply construct the table at the sender and give
 * it (already initialized) to the receiver.
 */
extern block_cb_t	*blk_cb_tab;

/**
 * Table containing the symbol "control block" structures for all the source and repair
 * symbols, in the same order as that of the orig_symbol[] table.
 * In theory, this table is constructed independantly by both the sender and the receiver,
 * thanks to the FEC Object Transmission Information (FEC OTI) information carried between
 * them. In practice, in eperf_tool, we simply construct the table at the sender and give
 * it (already initialized) to the receiver.
 */
extern symbol_cb_t	*symb_cb_tab;


#endif	/* } GLOBALS_H */

