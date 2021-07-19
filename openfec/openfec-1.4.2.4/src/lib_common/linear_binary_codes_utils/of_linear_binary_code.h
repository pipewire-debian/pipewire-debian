/* $Id: of_linear_binary_code.h 209 2014-12-12 18:49:58Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2011 INRIA - All rights reserved
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

#ifndef LINEAR_BINARY_CODE_H
#define LINEAR_BINARY_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#ifdef USE_NEON
//#include <arm_neon.h>
//#endif

#include "../of_openfec_api.h"		// includes of_type.h, of_debug.h

#ifdef IL_SUPPORT
#include <IL/il.h>
#endif

#ifdef ASSEMBLY_SSE_OPT
#include <xmmintrin.h>
#endif

#include "../of_rand.h"
#include "../of_cb.h"
#include "../of_mem.h"
#include "of_symbol.h"
#include "../statistics/of_statistics.h"

#include "binary_matrix/of_matrix_sparse.h"
#include "binary_matrix/of_matrix_dense.h"
#include "of_create_pchk.h"

#include "binary_matrix/of_matrix_convert.h"
#include "binary_matrix/of_hamming_weight.h"
#include "binary_matrix/of_tools.h"


/**
 * Linear-Binary-Code stable codec specific control block structure.
 */
typedef struct of_linear_binary_code_cb
{
/*****************************************************************************
*                              of_cb_t                                       *
******************************************************************************/
	of_codec_id_t	codec_id;		/* must begin with fec_codec_id          */
	of_codec_type_t	codec_type;		/* must be 2nd item                      */
	UINT32		nb_source_symbols;	/** k parameter (AKA code dimension).    */
	UINT32		nb_repair_symbols;	/** r = n - k parameter.                 */
	UINT32		encoding_symbol_length;	/** symbol length.                   */
/*****************************************************************************/

	UINT32		nb_total_symbols;	/** n parameter (AKA code length). */
	/* parity check matrix */
	of_mod2sparse*	pchk_matrix;

	/** statistics for this codec instance. */
	of_symbol_stats_op_t	*stats_xor;
#ifdef OF_DEBUG
	of_symbols_stats_t	*stats_symbols;
#endif

	UINT32		nb_source_symbol_ready; // Number of source symbols ready
	UINT32		nb_repair_symbol_ready; // Number of parity symbols ready

#ifdef ML_DECODING
	UINT32		*index_rows;	// Indirection index to access initial m_chekValues array
	UINT32		*index_cols;	// Indirection index to access initial symbol array
	UINT32		remain_cols;	// Nb of non empty remaining cols in the future simplified matrix
	UINT32		remain_rows;	// Nb of non empty remaining rows in the future simplified matrix

	of_mod2sparse	*pchk_matrix_simplified; // Simplified Parity Check Matrix in sparse mode format
	of_mod2sparse	*original_pchkMatrix;
	of_mod2sparse	*pchk_matrix_gauss;	// Parity Check matrix in sparse mode format.
	
	UINT32		dec_step;		// Current step in the Gauss decoding algorithm
	UINT32		threshold_simplification;// threshold (number of symbols) above which we
						// run the Gaussian Elimination algorithm
#endif

#ifdef OF_USE_DECODER /* { */
	/** table of all check values, i.e. that contain the constant term of each equation. */
	void**		tab_const_term_of_equ;
	/** table containing the number of encoding symbols of each equation. */
	UINT16*		tab_nb_enc_symbols_per_equ;
	/** table containing the number of unknow symbols (i.e. neither received nor decoded
	 *  at that time) of each equation. */
	UINT16*		tab_nb_unknown_symbols;
	/** table containing the number of equations in which a repair symbol is included. */
	UINT16*		tab_nb_equ_for_repair;
	
	void		** repair_symbols_values;
	void		** tmp_tab_symbols;
	UINT16		nb_tmp_symbols;
#endif /* } OF_USE_DECODER */

	void 		**encoding_symbols_tab;

	/** callbacks registered by the application. */
	void*	(*decoded_source_symbol_callback) (void	*context,
						   UINT32	size,	/* size of decoded source symbol */
						   UINT32	esi);	/* encoding symbol ID in {0..k-1} */
	void*	(*decoded_repair_symbol_callback) (void	*context,
						   UINT32	size,	/* size of decoded repair symbol */
						   UINT32	esi);	/* encoding symbol ID in {0..k-1} */
	void*	context_4_callback;
} of_linear_binary_code_cb_t;


#include "it_decoding/of_it_decoding.h"
#include "ml_decoding/of_ml_decoding.h"
#include "ml_decoding/of_ml_tool.h"


#endif
