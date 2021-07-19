/* $Id: of_2d_parity_api.c 185 2014-07-15 09:57:16Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
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

#include "of_2d_parity_includes.h"

#ifdef OF_USE_2D_PARITY_MATRIX_CODEC



of_status_t	of_2d_parity_create_codec_instance (of_2d_parity_cb_t**	of_cb)
{
	of_codec_type_t		codec_type;	/* temporary value */
	of_2d_parity_cb_t	*cb;

	OF_ENTER_FUNCTION
	cb = (of_2d_parity_cb_t*) of_realloc (*of_cb, sizeof (of_2d_parity_cb_t));
	/* realloc does not initialize the additional buffer space, so do that manually,
	 * then re-initialize a few parameters */
	codec_type			= cb->codec_type;
	memset(cb, 0, sizeof(*cb));
	*of_cb				= cb;
	cb->codec_id			= OF_CODEC_2D_PARITY_MATRIX_STABLE;
	cb->codec_type			= codec_type;
	cb->max_nb_source_symbols	= OF_2D_PARITY_MATRIX_MAX_NB_SOURCE_SYMBOLS_DEFAULT;	/* init it immediately... */
	cb->max_nb_encoding_symbols	= OF_2D_PARITY_MATRIX_MAX_NB_ENCODING_SYMBOLS_DEFAULT;	/* init it immediately... */
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


of_status_t	of_2d_parity_release_codec_instance (of_2d_parity_cb_t*	ofcb)
{
	UINT32 i;

	OF_ENTER_FUNCTION
	if (ofcb->pchk_matrix  != NULL)
	{
		of_mod2sparse_free(ofcb->pchk_matrix);
		of_free (ofcb->pchk_matrix);
		ofcb->pchk_matrix  = NULL;
	}
	if (ofcb->encoding_symbols_tab != NULL)
	{
		/* do not try to free source buffers, it's the responsibility of the application
		 * using this library to free everything. Only try to free repair symbols. */
		for (i = ofcb->nb_source_symbols; i < ofcb->nb_total_symbols; i++)
		{
			if (ofcb->encoding_symbols_tab[i] != NULL)
			{
				of_free (ofcb->encoding_symbols_tab[i]);
				ofcb->encoding_symbols_tab[i] = NULL;
			}
		}
		of_free (ofcb->encoding_symbols_tab);
		ofcb->encoding_symbols_tab = NULL;
	}
#ifdef OF_USE_DECODER
	if(ofcb->codec_type & OF_DECODER)
	{
		if (ofcb->tab_nb_enc_symbols_per_equ != NULL)
		{
			of_free (ofcb->tab_nb_enc_symbols_per_equ);
			ofcb->tab_nb_enc_symbols_per_equ = NULL;
		}
		if (ofcb->tab_nb_equ_for_repair != NULL)
		{
			of_free (ofcb->tab_nb_equ_for_repair);
			ofcb->tab_nb_equ_for_repair = NULL;
		}
		if (ofcb->tab_nb_unknown_symbols != NULL)
		{
			of_free (ofcb->tab_nb_unknown_symbols);
			ofcb->tab_nb_unknown_symbols = NULL;
		}
		if (ofcb->tab_const_term_of_equ != NULL)
		{
			for (i = 0; i < ofcb->nb_repair_symbols; i++)
			{
				if (ofcb->tab_const_term_of_equ[i] != NULL)
				{
					of_free (ofcb->tab_const_term_of_equ[i]);
					ofcb->tab_const_term_of_equ[i] = NULL;
				}
			}
			of_free(ofcb->tab_const_term_of_equ);
		}
	}
#endif

#ifdef OF_2D_PARITY_ML_DECODING
	if (ofcb->index_rows != NULL)
	{
		of_free (ofcb->index_rows);
		ofcb->index_rows = NULL;
	}
	if (ofcb->index_cols != NULL)
	{
		of_free (ofcb->index_cols);
		ofcb->index_cols = NULL;
	}
	if (ofcb->pchk_matrix_simplified != NULL)
	{
		of_mod2sparse_free (ofcb->pchk_matrix_simplified);
		of_free(ofcb->pchk_matrix_simplified);
		ofcb->pchk_matrix_simplified = NULL;
	}
	if (ofcb->original_pchkMatrix != NULL)
	{
		of_mod2sparse_free (ofcb->original_pchkMatrix);
		ofcb->original_pchkMatrix = NULL;
	}
	if (ofcb->pchk_matrix_gauss != NULL)
	{
		of_mod2sparse_free (ofcb->pchk_matrix_gauss);
		ofcb->pchk_matrix_gauss = NULL;
	}
#endif
#ifdef OF_DEBUG
	if (ofcb->stats_xor != NULL)
	{
		of_print_xor_symbols_statistics(ofcb->stats_xor);
		of_free(ofcb->stats_xor);
	}
#endif
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


of_status_t	of_2d_parity_set_fec_parameters (of_2d_parity_cb_t*	ofcb,
						      of_2d_parity_parameters_t*	params)
{
	of_mod2entry	*e;
	UINT32		row;
	UINT32		seq;

	OF_ENTER_FUNCTION

#ifdef OF_DEBUG
	ofcb->stats_xor		= of_calloc(1, sizeof(of_2d_symbol_stats_op_t));
	ofcb->stats_symbols	= of_calloc(1,sizeof(of_symbols_stats_t));
#endif
	if ((ofcb->nb_source_symbols = params->nb_source_symbols) > ofcb->max_nb_source_symbols) {
		OF_PRINT_ERROR(("of_2d_parity_set_fec_parameters: ERROR, invalid nb_source_symbols parameter (got %d, maximum is %d)",
				ofcb->nb_source_symbols, ofcb->max_nb_source_symbols));
		goto error;
	}
	ofcb->nb_repair_symbols = params->nb_repair_symbols;
	ofcb->nb_total_symbols = ofcb->nb_source_symbols + ofcb->nb_repair_symbols;
	if (ofcb->nb_total_symbols > ofcb->max_nb_encoding_symbols) {
		OF_PRINT_ERROR(("of_2d_parity_set_fec_parameters: ERROR, invalid number of encoding symbols (got %d, maximum is %d)",
				ofcb->nb_total_symbols, ofcb->max_nb_encoding_symbols));
		goto error;
	}
	ofcb->encoding_symbol_length = params->encoding_symbol_length;
	OF_TRACE_LVL (1, ("%s: k=%u, n-k=%u, n=%u, symbol_length=%u, PRNG seed=%u, N1=%u\n", __FUNCTION__,
			ofcb->nb_source_symbols, ofcb->nb_repair_symbols, ofcb->nb_total_symbols,
			ofcb->encoding_symbol_length, 0, 0))
	ofcb->pchk_matrix = of_create_pchk_matrix (ofcb->nb_repair_symbols, ofcb->nb_total_symbols, Evenboth,
						   0, 0, false, Type2DMATRIX, 1);

	if (ofcb->pchk_matrix == NULL)
	{
		OF_PRINT_ERROR(("of_2d_parity_set_fec_parameters : ERROR, parity check matrix can't be created with this parameters.."))
		goto error;
	}
#ifdef OF_DEBUG
	if (of_verbosity >= 2)
	{
		of_mod2sparse_matrix_stats(stdout, ofcb->pchk_matrix, ofcb->nb_source_symbols, ofcb->nb_repair_symbols);
	}
#endif

#ifdef OF_2D_PARITY_ML_DECODING
	ofcb->pchk_matrix_simplified = NULL;
#endif
	if ((ofcb->encoding_symbols_tab = (void**) of_calloc (ofcb->nb_total_symbols, sizeof (void*))) == NULL) {
		goto no_mem;
	}
#ifdef OF_USE_DECODER
	if (ofcb->codec_type & OF_DECODER)
	{
		ofcb->tab_nb_unknown_symbols = (UINT16*)
				of_calloc (ofcb->nb_repair_symbols, sizeof (UINT16));
		ofcb->tab_const_term_of_equ = (void**)
				of_calloc (ofcb->nb_repair_symbols, sizeof (void*));
		ofcb->tab_nb_equ_for_repair = (UINT16*)
				of_calloc (ofcb->nb_repair_symbols, sizeof (UINT16));
		ofcb->tab_nb_enc_symbols_per_equ = (UINT16*)
				of_calloc (ofcb->nb_repair_symbols, sizeof (UINT16));
		if (ofcb->tab_nb_unknown_symbols == NULL || ofcb->tab_const_term_of_equ == NULL ||
		    ofcb->tab_nb_equ_for_repair == NULL || ofcb->tab_nb_enc_symbols_per_equ == NULL) {
			goto no_mem;
		}
		// and update the various tables now
		for (row = 0; row < ofcb->nb_repair_symbols; row++)
		{
			for (e = of_mod2sparse_first_in_row (ofcb->pchk_matrix, row);
					!of_mod2sparse_at_end (e);
					e = of_mod2sparse_next_in_row (e))
			{
				ofcb->tab_nb_enc_symbols_per_equ[row]++;
				ofcb->tab_nb_unknown_symbols[row]++;
			}
		}
		for (seq = ofcb->nb_source_symbols; seq < (ofcb->nb_total_symbols); seq++)
		{
			for (e = of_mod2sparse_first_in_col (ofcb->pchk_matrix, of_get_symbol_col ((of_cb_t*)ofcb, seq));
					!of_mod2sparse_at_end (e);
					e = of_mod2sparse_next_in_col (e))
			{
				ofcb->tab_nb_equ_for_repair[seq - ofcb->nb_source_symbols]++;
			}
		}
	}
#endif //OF_USE_DECODER
	ofcb->nb_source_symbol_ready = 0; // Number of source symbols ready
	ofcb->nb_repair_symbol_ready = 0; // Number of parity symbols ready
	//ofcb->nb_non_dup_symbols_recvd = 0;// Number of non-duplicated symbols received
#ifdef OF_2D_PARITY_ML_DECODING
	ofcb->index_rows = NULL;	// Indirection index to access initial m_chekValues array
	ofcb->index_cols = NULL;	// Indirection index to access initial symbol array
	ofcb->remain_cols = 0;		// Nb of non empty remaining cols in the future simplified matrix
	ofcb->remain_rows = 0;		// Nb of non empty remaining rows in the future simplified matrix
	ofcb->pchk_matrix_simplified = NULL; // Simplified Parity Check Matrix in sparse mode format
	ofcb->original_pchkMatrix = NULL;
	ofcb->pchk_matrix_gauss = NULL;	// Parity Check matrix in sparse mode format. This matrix
					// is also used as a generator matrix in LDGM-* modes
	ofcb->dec_step = 0;		// Current step in the Gauss Elimination algorithm
	ofcb->threshold_simplification = 0; // threshold (number of symbols) above which we
					// run the Gauss Elimination algorithm
#endif

	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
no_mem:
	OF_PRINT_ERROR(("out of memory"));
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_2d_parity_set_callback_functions (
				of_2d_parity_cb_t*		ofcb,
				void* (*decoded_source_symbol_callback) (
								void	*context,
								UINT32	size,	/* size of decoded source symbol */
								UINT32	esi),	/* encoding symbol ID in {0..k-1} */
				void* (*decoded_repair_symbol_callback) (
								void	*context,
								UINT32	size,	/* size of decoded repair symbol */
								UINT32	esi),	/* encoding symbol ID in {k..n-1} */
				void*	context_4_callback)
{
	OF_ENTER_FUNCTION
	ofcb->decoded_source_symbol_callback	= 	decoded_source_symbol_callback;
	ofcb->decoded_repair_symbol_callback	= 	decoded_repair_symbol_callback;
	ofcb->context_4_callback		= 	context_4_callback;
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}

#ifdef OF_USE_ENCODER

of_status_t	of_2d_parity_build_repair_symbol (of_2d_parity_cb_t*		ofcb,
							void*				encoding_symbols_tab[],
							UINT32				esi_of_symbol_to_build)
{
	of_mod2entry	*e;
	UINT32		col_to_build;
	UINT32		esi;
	void		*to_add_buf;
	void		*parity_symbol;
	OF_ENTER_FUNCTION
	if (esi_of_symbol_to_build< ofcb->nb_source_symbols || esi_of_symbol_to_build >= ofcb->nb_total_symbols)
	{
		OF_PRINT_ERROR(("of_2d_parity_build_repair_symbol: Error, bad esi of encoding symbol (%d)", esi_of_symbol_to_build))
		goto error;
	}
	parity_symbol = encoding_symbols_tab[esi_of_symbol_to_build];
	memset (parity_symbol, 0, ofcb->encoding_symbol_length);
	col_to_build = of_get_symbol_col ((of_cb_t*)ofcb, esi_of_symbol_to_build);
	e = of_mod2sparse_first_in_row (ofcb->pchk_matrix, col_to_build);
	while (!of_mod2sparse_at_end (e))
	{
		// paritySymbol_index in {0.. n-k-1} range, so this test is ok
		if (e->col != col_to_build)
		{
			// don't add paritySymbol to itself
			esi = of_get_symbol_esi ((of_cb_t*)ofcb, e->col);
			to_add_buf = (void*) encoding_symbols_tab[esi];
			if (to_add_buf == NULL)
			{
				OF_PRINT_ERROR(("symbol %d is not allocated", esi));
				goto error;
			}
#ifdef OF_DEBUG
			of_add_to_symbol (parity_symbol, to_add_buf, ofcb->encoding_symbol_length, &(ofcb->stats_xor->nb_xor_for_IT));
#else
			of_add_to_symbol (parity_symbol, to_add_buf, ofcb->encoding_symbol_length);
#endif
		}
		e = of_mod2sparse_next_in_row (e);
	}
	OF_TRACE_LVL (1, ("%s: repair symbol (esi=%d) built\n", __FUNCTION__, esi_of_symbol_to_build))
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

error:
	OF_EXIT_FUNCTION
	return OF_STATUS_ERROR;
}

#endif //OF_USE_ENCODER

#ifdef OF_USE_DECODER

of_status_t	of_2d_parity_decode_with_new_symbol (of_2d_parity_cb_t*	ofcb,
							  void*				new_symbol,
							  UINT32			new_symbol_esi)
{
	OF_ENTER_FUNCTION
	return of_linear_binary_code_decode_with_new_symbol((of_linear_binary_code_cb_t*)ofcb,new_symbol,new_symbol_esi);
}


of_status_t	of_2d_parity_set_available_symbols (of_2d_parity_cb_t*	ofcb,
							 void* const			encoding_symbols_tab[])
{
	OF_ENTER_FUNCTION
	UINT32 i;
	for (i = 0; i < ofcb->nb_total_symbols; i++)
	{
		if (encoding_symbols_tab[i] != NULL)
		{
			ofcb->encoding_symbols_tab[i] = of_calloc (1, ofcb->encoding_symbol_length);
			memcpy (ofcb->encoding_symbols_tab[i], encoding_symbols_tab[i], ofcb->encoding_symbol_length);
		}
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


of_status_t	of_2d_parity_finish_decoding (of_2d_parity_cb_t*	ofcb)
{
	OF_ENTER_FUNCTION
#ifdef OF_2D_PARITY_ML_DECODING
	return of_linear_binary_code_finish_decoding_with_ml ((of_linear_binary_code_cb_t*)ofcb);
#else
	return OF_STATUS_ERROR;
#endif
}

bool	of_2d_parity_is_decoding_complete (of_2d_parity_cb_t*	ofcb)
{
	for (; ofcb->first_non_decoded < ofcb->nb_source_symbols; ofcb->first_non_decoded++)
	{
		if (ofcb->encoding_symbols_tab[ofcb->first_non_decoded] == NULL)
		{
			OF_TRACE_LVL (1, ("decoding not complete (%u source symbols ready, %u expected)\n",
					ofcb->nb_source_symbol_ready, ofcb->nb_source_symbols))
			//ASSERT(ofcb->nb_source_symbol_ready < ofcb->nb_source_symbols);
			return false;
		}
	}
	OF_TRACE_LVL (1, ("decoding is complete\n"))
	//ASSERT(ofcb->nb_source_symbol_ready == ofcb->nb_source_symbols);
	return true;
}


of_status_t		of_2d_parity_get_source_symbols_tab(of_2d_parity_cb_t* ofcb,
													void* source_symbols_tab[])
{
	OF_ENTER_FUNCTION
	memcpy (source_symbols_tab, ofcb->encoding_symbols_tab, sizeof (void *) * ofcb->nb_source_symbols);
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}

#endif //OF_USE_DECODER

of_status_t	of_2d_parity_set_control_parameter  (of_2d_parity_cb_t*	ofcb,
						UINT32			type,
						void*			value,
						UINT32			length)
{
	OF_PRINT_ERROR(("of_2d_parity_set_control_parameter: ERROR, not implemented...\n"))
	return OF_STATUS_ERROR;
}


of_status_t	of_2d_parity_get_control_parameter  (of_2d_parity_cb_t*	ofcb,
						UINT32			type,
						void*			value,
						UINT32			length)
{
	OF_ENTER_FUNCTION
	switch (type) {
	case OF_CTRL_GET_MAX_K:
		if (value == NULL || length != sizeof(UINT32)) {
			OF_PRINT_ERROR(("%s: OF_CTRL_GET_MAX_K ERROR: null value or bad length (got %d, expected %ld)\n",
				__FUNCTION__, length, sizeof(UINT32)))
			goto error;
		}
		*(UINT32*)value = ofcb->max_nb_source_symbols;
		OF_TRACE_LVL(1, ("%s: OF_CTRL_GET_MAX_K (%d)\n", __FUNCTION__, *(UINT32*)value))
		break;

	case OF_CTRL_GET_MAX_N:
		if (value == NULL || length != sizeof(UINT32)) {
			OF_PRINT_ERROR(("%s: OF_CTRL_GET_MAX_N ERROR: null value or bad length (got %d, expected %ld)\n",
				__FUNCTION__, length, sizeof(UINT32)))
			goto error;
		}
		*(UINT32*)value = ofcb->max_nb_encoding_symbols;
		OF_TRACE_LVL(1, ("%s: OF_CTRL_GET_MAX_N (%d)\n", __FUNCTION__, *(UINT32*)value))
		break;

	default:
		OF_PRINT_ERROR(("%s: unknown type (%d)\n", __FUNCTION__, type))
		goto error;
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

error:
	OF_EXIT_FUNCTION
	return OF_STATUS_ERROR;
}

#endif /* #ifdef OF_USE_2D_PARITY_MATRIX_CODEC */
