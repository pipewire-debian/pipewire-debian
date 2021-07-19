/* $Id: eperftool.h 100 2013-11-07 02:54:55Z roca $ */
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


/* AL-FEC extended performance tool */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef WIN32		/* Windows specific includes */
#include <Winsock2.h>
#include <windows.h>
#else	/* UNIX */	/* Unix specific includes */
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>	/* for gettimeofday */
#endif	/* OS */

#define OF_USE_ENCODER
#define OF_USE_DECODER
#include "../../src/lib_common/of_openfec_api.h"
#include "../../src/lib_common/of_mem.h"	/* for of_dump_buffer declaration only */

#ifdef WIN32
#include "../src/getopt.h"
#endif

#include "defaults.h"
#include "blocking_struct.h"
#include "callbacks.h"

/* useful macros */
#define min(a,b)	((a) <= (b) ? (a) : (b))
#define max(a,b)	((a) < (b) ? (b) : (a))


/*
 * Various transmission modes.
 * Be careful to keep the order compliant with the eperf_tool -t<n> argument.
 */
typedef enum {
	TX_MODE_ALL_RANDOM = 0,
	TX_MODE_FEW_SRC_SYMBOLS,
	TX_MODE_FEW_SRC_SYMBOLS_FIRST,
	TX_MODE_NON_SYSTEMATIC,
	TX_MODE_SEQUENTIAL,
	TX_MODE_INVERSE_SEQUENTIAL,
	TX_MODE_SEQUENTIAL_SRC_THEN_RANDOM_REPAIR,
	TX_MODE_SEQUENTIAL_REPAIR_THEN_RANDOM_SRC,
	TX_MODE_SIMUL_BCAST_WITH_UNICAST_REPAIR
	//TX_MODE_PKT_INTERLEAVING
} tx_mode_t;


/**
 * Control block describing a block within the object.
 */
typedef struct block_cb {
	UINT32		sbn;
	UINT32		k;
	UINT32		n;
	/** Index of first source symbol in the global orig_symb[] table. This is not its ESI! */
	UINT32		first_src_symbol_idx;
	/** Index of first repair symbol in the global orig_symb[] table. This is not its ESI! */
	UINT32		first_repair_symbol_idx;

	/*
	 * Receiver specific fields.
	 */
	/** used by a rx: true once it has been decoded. */
	bool		is_decoded;
	/** used by a rx: true if the decoding for this block is abandonned (no need to consider additional symbols then). */
	bool		is_abandoned;
	/** number of symbols received for this block until decoding succeeded. */
	UINT32		nb_symbols_received;
	/** decoder session, closed once the block has been decoded. */
	of_session_t	*ses;

	/*
	 * LDPC specific fields.
	 */
	UINT32		ldpc_seed;	/** PRNG seed (see RFC 5170). */
	UINT32		ldpc_N1;	/** N1 parameter of LDPC-* codes (see RFC 5170). */
	bool		ldpc_dont_send_last_repair;	/** with ldpc-staircase, if N1 is even, this symbol is often
							 * null, so do not sent it, the codec already knows it */
} block_cb_t;


/**
 * Control block describing a symbol within a block.
 */
typedef struct symbol_cb {
	UINT32		esi;	/** Encoding Symbol ID, i.e. position in block. */
	UINT32		sbn;	/** Source Block Number, i.e. block position in object. */
} symbol_cb_t;


#include "globals.h"
#include "codec_instance_mgmt.h"


/*
 * Function prototypes
 */


/**
 * Returns the decoding status: -1 in case of fatal error, 0 if decoding successful, 1 if decoding failed.
 */
int		start_enc_dec_test ();

/**
 * Print the command line arguments and some information about the OpenFEC library.
 */
void		print_preamble		(char *command_line);

of_status_t 	parse_command_line	(INT32 argc, char *argv[]);

/**
 * Finishes the initialization of the user's parameters.
 * However, some parameters that depend on the codec, will not be initialized yet
 * by this function.
 */
of_status_t	finish_init_command_line_params	(void);

of_status_t	init_prng_with_seed	(UINT32 suggested_seed);

of_status_t	init_sender		(void);

of_status_t	encode			(void);

of_status_t	init_receiver		(void);

of_status_t	receive_and_decode	(void);

of_status_t	init_tx_simulator	(void);

symbol_cb_t *	get_next_symbol_received (void);

of_status_t	close_tx_simulator	(void);

#ifdef OF_DEBUG
void		print_rx_stats		(void);
#endif

of_status_t	print_usage		(char *cmdName);

/** 
 * Define our own PRNG function in order to have predictable results
 * that do not depend on the target OS/platform.
 */
UINT32		myrand			(void);
