/* $Id: of_ml_decoding.c 218 2014-12-13 14:15:11Z roca $ */
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

#include "../of_linear_binary_code.h"


#ifdef OF_USE_DECODER
#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS
#ifdef ML_DECODING


/******  Static Functions  ****************************************************/


/**
 * This function prepars the linear system (H) to be further simplified.
 *
 * @brief			creates a copy of H matrix and update all ML deconding variables
 * @param ofcb			(IN/OUT) Linear-Binary-Code control-block.
 * @return			OF_STATUS_OK if it's OK
 */
static of_status_t
of_linear_binary_code_prepar_linear_system (of_linear_binary_code_cb_t	*ofcb);

/**
 * This function simplifies the linear system (H): delete rows and cols when feasible.
 * This step is needed as IT decoding performs in a lazzy way, avoiding to sum the available symbols
 * (received or decoded) to the constant term of the equations they are involved in unless it is the
 * last symbol of this equation. Here we need to do that explicitely in order to simplify the system
 * as much as possible.
 *
 * @brief			symplifies the linear system
 * @param ofcb 			(IN/OUT) Linear-Binary-Code control-block.
 * @param new_symbol		(IN/OUT) pointer to new symbol to process
 * @param new_symbol_esi	(IN) Encoding Symbol Index
 * @return			OF_STATUS_OK if it's OK
 */
static of_status_t
of_linear_binary_code_simplify_linear_system_with_a_symbol  (of_linear_binary_code_cb_t	*ofcb,
								const void			*new_symbol,
								UINT32				new_symbol_esi);

/**
 * This function creates the simplified linear system, considering only the non empty columns.
 * This funtion creates the new ofcb->pchk_matrix_simplified matrix.
 *
 * @brief			creates the simplified linear system
 * @param ofcb			(IN/OUT) Linear-Binary-Code control-block.
 * @return			OF_STATUS_OK if it's OK
 */
static of_status_t
of_linear_binary_code_create_simplified_linear_system (of_linear_binary_code_cb_t	*ofcb);


/******************************************************************************/


of_status_t
of_linear_binary_code_finish_decoding_with_ml (of_linear_binary_code_cb_t	*ofcb)
{
	INT32		i;
	UINT32		*permutation_array		= NULL;
	of_mod2dense	*dense_pchk_matrix_simplified	= NULL;
	INT32		*column_idx			= NULL;
	void		**const_term			= NULL;
	void		**variable_member		= NULL;
	UINT32		nb_computed_repair_in_ml;
#ifdef DEBUG
	struct timeval	gdtv0;		/* start */
	struct timeval	gdtv1;		/* end */
	struct timeval	gdtv_delta;	/* difference tv1 - tv0 */
#endif

	OF_ENTER_FUNCTION
	OF_TRACE_LVL (1, ("ML decoding on parity check matrix\n"))
	/*
	 *  Step 0: Matrix simplification, where we remove known symbols from the system, adding their value
	 * to corresponding constant terms.
	 *
	 * This step is needed as IT decoding performs in a lazzy way, avoiding to sum the available symbols
	 * (received or decoded) to the constant term of the equations they are involved in unless it is the
	 * last symbol of this equation. Here we need to do that explicitely in order to simplify the system
	 * as much as possible.
	 */
	ofcb->remain_rows = ofcb->nb_repair_symbols;
	ofcb->remain_cols = ofcb->nb_source_symbols + ofcb->nb_repair_symbols;
	if (of_linear_binary_code_prepar_linear_system (ofcb) != OF_STATUS_OK)
	{
		if (ofcb->remain_cols == 0)
		{
			// in fact decoding is already finished!
			OF_TRACE_LVL (1, ("Create simplified linear system says it's finished!\n"))
			OF_EXIT_FUNCTION
			return OF_STATUS_FAILURE;
		}
		// but here it failed!
		OF_TRACE_LVL (1, ("Create simplified linear system failed, ofcb->remain_cols=%d\n", ofcb->remain_cols))
		OF_EXIT_FUNCTION
		return OF_STATUS_FAILURE;
	}
	/*
	 * Inject all known source symbols
	 */
	for (i = 0 ; i < ofcb->nb_source_symbols ; i++)
	{
		if (ofcb->encoding_symbols_tab[i] != NULL)
		{
			if (of_linear_binary_code_simplify_linear_system_with_a_symbol (ofcb, ofcb->encoding_symbols_tab[i], i) != OF_STATUS_OK)
			{
				OF_TRACE_LVL(0,("Simplifying the matrix with source symbols failed\n"))
				OF_EXIT_FUNCTION
				return OF_STATUS_FAILURE;
			}
		}
	}
	/*
	 * Inject all known repair symbols
	 */
	/* Randomize the repair symbols order before injecting them (it makes the decoding process more efficient). */
	permutation_array = (UINT32 *) of_malloc (ofcb->nb_repair_symbols * sizeof(UINT32));
	for (i = 0 ; i < ofcb->nb_repair_symbols ; i++)
	{
		permutation_array[i] = i;
	}
	for (i = 0 ; i < ofcb->nb_repair_symbols ; i++)
	{
		INT32	backup;
		INT32	rand_val;

		backup = permutation_array[i];
		rand_val = rand() % ofcb->nb_repair_symbols;
		permutation_array[i] = permutation_array[rand_val];
		permutation_array[rand_val] = backup;
	}
	/* Inject parity symbols following the random order given by permutation_array */
	for (i = 0 ; i < ofcb->nb_repair_symbols ; i++)
	{
		if (ofcb->encoding_symbols_tab[ofcb->nb_source_symbols+permutation_array[i]] != NULL)
		{
			if (of_linear_binary_code_simplify_linear_system_with_a_symbol (ofcb, ofcb->encoding_symbols_tab[ofcb->nb_source_symbols+permutation_array[i]],
									  of_get_symbol_esi ((of_cb_t*)ofcb, permutation_array[i])) != OF_STATUS_OK)
			{
				OF_TRACE_LVL(0,("Simplifying the matrix with parity symbols failed\n"))
				goto failure;
			}
		}
	}
	of_free (permutation_array);
	permutation_array = NULL;
	OF_TRACE_LVL (1, ("%s: ofcb->remain_rows=%d, ofcb->remain_cols=%d\n", __FUNCTION__, ofcb->remain_rows, ofcb->remain_cols))
	if (of_linear_binary_code_create_simplified_linear_system (ofcb) != OF_STATUS_OK)
	{
		OF_TRACE_LVL(0, ("Create Simplified Linear System failed\n"))
		goto failure;
	}
#ifdef IL_SUPPORT
	of_mod2sparse_print_bitmap(ofcb->pchk_matrix_simplified);
#endif
	/*
	 * It's now time to create the simplified matrix, in dense format.
	 */
	dense_pchk_matrix_simplified = of_mod2dense_allocate (ofcb->pchk_matrix_simplified->n_rows, ofcb->pchk_matrix_simplified->n_cols);
	of_mod2sparse_to_dense (ofcb->pchk_matrix_simplified, dense_pchk_matrix_simplified);
	/* and immediately free the now useless sparse version to save memory... */
	of_mod2sparse_free (ofcb->pchk_matrix_simplified);
	of_free (ofcb->pchk_matrix_simplified);
	ofcb->pchk_matrix_simplified = NULL;

#ifdef DEBUG
	gettimeofday (&gdtv0, NULL);
	OF_TRACE_LVL (1, ("gauss_decoding_start=%ld.%ld\n", gdtv0.tv_sec, gdtv0.tv_usec))
#endif
	if ((column_idx = (INT32 *) of_malloc (of_mod2dense_cols (dense_pchk_matrix_simplified) * sizeof (INT32))) == NULL)
	{
		goto no_mem;
	}
	for (i = 0; i < of_mod2dense_cols (dense_pchk_matrix_simplified); i++)
	{
		column_idx[i] = i;
	}
	if ((const_term = (void **) of_malloc (of_mod2dense_rows (dense_pchk_matrix_simplified) * sizeof (void*))) == NULL)
	{
		goto no_mem;
	}
	for (i = 0; i < of_mod2dense_rows (dense_pchk_matrix_simplified); i++)
	{
		const_term[i] = ofcb->tab_const_term_of_equ[ofcb->index_rows[i]];
		ofcb->tab_const_term_of_equ[ofcb->index_rows[i]] = NULL;
	}
	if ((variable_member = (void **) of_calloc (of_mod2dense_cols (dense_pchk_matrix_simplified), sizeof (void*))) == NULL)
	{
		goto no_mem;
	}
	/*
	 * And finally launch Gaussian Elimination.
	 */
	if (of_linear_binary_code_solve_dense_system (ofcb, dense_pchk_matrix_simplified, const_term, variable_member) != OF_STATUS_OK)
	{
		OF_TRACE_LVL(0,("Solve dense system failed\n"))
		goto failure;
	}
	/*
	 * the system has been solved, so store the result in the canvas
	 */
	OF_TRACE_LVL (1, ("Solve dense system successful\n"))
	nb_computed_repair_in_ml = ofcb->nb_repair_symbols - ofcb->nb_repair_symbol_ready; /* this is the number of repair found in ML */
	/* ignore the first nb_computed_repair_in_ml symbols found in ML as they are repair symbols and we don't need them */
	for (i = 0; i < nb_computed_repair_in_ml; i++)
	{
		if (variable_member[i] != NULL)
			of_free(variable_member[i]);
	}
	/* the remaining symbols found in ML are source symbols, so we copy pointers */
	for (i = 0; i < ofcb->nb_source_symbols; i++)
	{
		if (ofcb->encoding_symbols_tab[i] == NULL)
		{
			ofcb->encoding_symbols_tab[i] = variable_member[nb_computed_repair_in_ml];
			nb_computed_repair_in_ml++;
		}
	}
	while (nb_computed_repair_in_ml < of_mod2dense_cols (dense_pchk_matrix_simplified)) {
		of_free(variable_member[nb_computed_repair_in_ml]);
		nb_computed_repair_in_ml++;
	}
#ifdef DEBUG
	if (of_is_decoding_complete (ofcb))
	{
		OF_TRACE_LVL (1, ("ML decoding successful\n"))
	}
	gettimeofday (&gdtv1, NULL);
	timersub (&gdtv1, &gdtv0, &gdtv_delta);
	OF_TRACE_LVL (1, ("gauss_decoding_end=%ld.%ld   gauss_decoding_time=%ld.%06ld \n",
			gdtv1.tv_sec, gdtv1.tv_usec, gdtv_delta.tv_sec, gdtv_delta.tv_usec))
#endif
	if (const_term != NULL)
	{
		for (i = 0; i < of_mod2dense_rows (dense_pchk_matrix_simplified); i++)
		{
			if (const_term[i])
			{
				of_free(const_term[i]);
			}
		}
	}
	of_free(const_term);
	const_term = NULL;
	of_free(variable_member);
	variable_member = NULL;
	of_free(column_idx);
	column_idx = NULL;
	of_mod2dense_free(dense_pchk_matrix_simplified);
	dense_pchk_matrix_simplified = NULL;
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

failure:
	if (permutation_array != NULL)
	{
		of_free (permutation_array);
		permutation_array = NULL;
	}
	if (const_term != NULL)
	{
		for (i = 0; i < of_mod2dense_rows (dense_pchk_matrix_simplified); i++)
		{
			if (const_term[i])
			{
				of_free(const_term[i]);
			}
		}
		of_free(const_term);
		const_term = NULL;
	}
	if (variable_member != NULL)
	{
		of_free(variable_member);
		variable_member = NULL;
	}
	if (column_idx != NULL)
	{
		of_free(column_idx);
		column_idx = NULL;
	}
	if (dense_pchk_matrix_simplified != NULL)
	{
		of_mod2dense_free(dense_pchk_matrix_simplified);
		dense_pchk_matrix_simplified = NULL;
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_FAILURE;

no_mem:
	OF_PRINT_ERROR(("out of memory"))
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


/******  Static Functions  ****************************************************/


of_status_t
of_linear_binary_code_prepar_linear_system (of_linear_binary_code_cb_t		*ofcb)
{
	INT32	_row;
	INT32	_col;

	OF_ENTER_FUNCTION
	if (ofcb->index_rows == NULL)
	{
		ofcb->index_rows = (UINT32*) of_calloc (ofcb->nb_repair_symbols, sizeof (UINT32));
	}
	if (ofcb->index_cols == NULL)
	{
		ofcb->index_cols = (UINT32*) of_calloc (ofcb->nb_total_symbols, sizeof (UINT32));
	}
	for (_row = 0; _row < ofcb->nb_repair_symbols; _row++)
	{
		ofcb->index_rows[_row] = _row;
	}
	ofcb->remain_rows = _row;
	for (_col = 0; _col < ofcb->nb_total_symbols; _col++)
	{
		ofcb->index_cols[_col] = _col;
	}
	ofcb->remain_cols = _col;
	// Update the ML decoding variables too
	for (_row = 0 ; _row < ofcb->nb_repair_symbols ; _row++)
	{
		of_mod2entry	*e;

		ofcb->tab_nb_enc_symbols_per_equ[_row] = 0;
		ofcb->tab_nb_unknown_symbols[_row] = 0;
		for (e = of_mod2sparse_first_in_row (ofcb->pchk_matrix, _row); !of_mod2sparse_at_end (e); e = of_mod2sparse_next_in_row (e))
		{
			ofcb->tab_nb_enc_symbols_per_equ[_row]++;
			ofcb->tab_nb_unknown_symbols[_row]++;
		}
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


static of_status_t
of_linear_binary_code_simplify_linear_system_with_a_symbol (of_linear_binary_code_cb_t	*ofcb,
							    const void			*new_symbol,
							    UINT32			new_symbol_esi)
{
	of_mod2entry	*_e;		// entry ("1") in parity check matrix and entry to delete in row/column (temp)
	of_mod2entry	*_del_me;
	INT32		_row;

	OF_ENTER_FUNCTION
	if (of_mod2sparse_empty_col (ofcb->pchk_matrix, of_get_symbol_col((of_cb_t*)ofcb, new_symbol_esi)))
	{
		OF_TRACE_LVL (1, ((" nothing to do, col empty\n")))
		OF_EXIT_FUNCTION
		return OF_STATUS_OK;
	}
	if (of_is_source_symbol((of_cb_t*)ofcb, new_symbol_esi) && of_is_decoding_complete((of_session_t*)ofcb))
	{
		// Decoding is now finished, return...
		OF_TRACE_LVL (1, ((" decoding is finished!\n")))
		OF_EXIT_FUNCTION
		return OF_STATUS_OK;
	}
	_e = of_mod2sparse_first_in_col (ofcb->pchk_matrix, of_get_symbol_col((of_cb_t*)ofcb, new_symbol_esi));
	while (!of_mod2sparse_at_end_col (_e))
	{
		_row = of_mod2sparse_row (_e);
		// If the constant term buffer does not exist, create it
		if (ofcb->tab_const_term_of_equ[_row] == NULL)
		{
			if ((ofcb->tab_const_term_of_equ[_row] = of_malloc (ofcb->encoding_symbol_length)) == NULL)
			{
				goto no_mem;
			}
			memcpy(ofcb->tab_const_term_of_equ[_row], new_symbol, ofcb->encoding_symbol_length);
		}
		else
		{
			// Inject the symbol in the partial sum
			of_add_to_symbol((ofcb->tab_const_term_of_equ[_row]), new_symbol, ofcb->encoding_symbol_length
#ifdef OF_DEBUG
					, &(ofcb->stats_xor->nb_xor_for_ML)
#endif
					);
		}
		// Decrease the number of unknown symbols in the equation
		ofcb->tab_nb_unknown_symbols[_row]--;	// symbol is known
		// Delete the current element
		_del_me = _e;
		_e = of_mod2sparse_next_in_col (_e);
		of_mod2sparse_delete (ofcb->pchk_matrix, _del_me);
		// If there is only one more symbol in the line, reinject it and progress like that
		if (ofcb->tab_nb_unknown_symbols[_row] == 1)
		{
			of_mod2entry	*__r;
			UINT32		decoded_symbol_seqno;

			// Get the only one symbol in the equation
			__r = of_mod2sparse_first_in_row (ofcb->pchk_matrix, _row);
			decoded_symbol_seqno = of_get_symbol_esi ((of_cb_t*)ofcb, of_mod2sparse_col (__r));
			// Identify the symbol with its value, and reinject it.
			if (of_is_source_symbol ((of_cb_t*)ofcb, decoded_symbol_seqno))
			{
				if (ofcb->encoding_symbols_tab[decoded_symbol_seqno] == NULL)
				{
					if (ofcb->decoded_source_symbol_callback != NULL)
					{
						ofcb->encoding_symbols_tab[decoded_symbol_seqno] =
							ofcb->decoded_source_symbol_callback(ofcb->context_4_callback,
											     ofcb->encoding_symbol_length,
											     decoded_symbol_seqno);
					}
					else
					{
						ofcb->encoding_symbols_tab[decoded_symbol_seqno] = of_malloc (ofcb->encoding_symbol_length);
					}
					if (ofcb->encoding_symbols_tab[decoded_symbol_seqno] == NULL)
					{
						goto no_mem;
					}
					// Source symbol.
					memcpy((ofcb->encoding_symbols_tab[decoded_symbol_seqno]),
						 (ofcb->tab_const_term_of_equ[_row]),
						 ofcb->encoding_symbol_length);
					of_free (ofcb->tab_const_term_of_equ[_row]);
					ofcb->tab_const_term_of_equ[_row] = NULL;
					// It'll be known at the end of this step
					ofcb->tab_nb_unknown_symbols[_row]--;	// symbol is known
					of_mod2sparse_delete (ofcb->pchk_matrix, __r);
					of_linear_binary_code_simplify_linear_system_with_a_symbol (ofcb, ofcb->encoding_symbols_tab[decoded_symbol_seqno],
												    decoded_symbol_seqno);
					ofcb->nb_source_symbol_ready++;
				}
			}
			else
			{
				// Decoded symbol is a parity symbol
				if (ofcb->encoding_symbols_tab[decoded_symbol_seqno] == NULL)
				{
					if (ofcb->decoded_repair_symbol_callback != NULL)
					{
						ofcb->encoding_symbols_tab[decoded_symbol_seqno] =
							ofcb->decoded_repair_symbol_callback(ofcb->context_4_callback,
											     ofcb->encoding_symbol_length,
											     decoded_symbol_seqno);
					}
					else
					{
						ofcb->encoding_symbols_tab[decoded_symbol_seqno] = of_malloc (ofcb->encoding_symbol_length);
					}
					if (ofcb->encoding_symbols_tab[decoded_symbol_seqno] == NULL)
					{
						goto no_mem;
					}
					// copy the content...
					memcpy(ofcb->encoding_symbols_tab[decoded_symbol_seqno],
						ofcb->tab_const_term_of_equ[_row],
						ofcb->encoding_symbol_length);
					of_free (ofcb->tab_const_term_of_equ[_row]);
					ofcb->tab_const_term_of_equ[_row] = NULL;
					// It'll be known at the end of this step
					ofcb->tab_nb_unknown_symbols[_row]--;	// symbol is known
					ofcb->tab_nb_equ_for_repair[decoded_symbol_seqno - ofcb->nb_source_symbols]--;
					of_mod2sparse_delete (ofcb->pchk_matrix, __r);
					of_linear_binary_code_simplify_linear_system_with_a_symbol (ofcb, ofcb->encoding_symbols_tab[decoded_symbol_seqno],
												    decoded_symbol_seqno);
					ofcb->nb_repair_symbol_ready++;
				}
			}
			ofcb->remain_rows--;
		}
	}
	ofcb->remain_cols--;
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

no_mem:
	OF_PRINT_ERROR(("out of memory"))
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t of_linear_binary_code_create_simplified_linear_system (of_linear_binary_code_cb_t	*ofcb)
{
	INT32		_row;
	INT32		_col;
	of_mod2sparse	*__pchk_matrix_simplified_cols;
	UINT32		*index_rows;
	UINT32		*index_cols;

	OF_ENTER_FUNCTION
	ofcb->remain_rows = 0;
	ofcb->remain_cols = 0;
	if (ofcb->index_rows == NULL)
	{
		if ((ofcb->index_rows = (UINT32 *) of_calloc (ofcb->nb_repair_symbols, sizeof (UINT32))) == NULL)
		{
			goto no_mem;
		}
	}
	if (ofcb->index_cols == NULL)
	{
		if ((ofcb->index_cols = (UINT32 *) of_calloc (ofcb->nb_total_symbols - ofcb->nb_repair_symbol_ready - ofcb->nb_source_symbol_ready,
								sizeof (UINT32))) == NULL)
		{
			goto no_mem;
		}
	}
	index_rows = (UINT32*) of_malloc(ofcb->nb_repair_symbols * sizeof(UINT32));
	index_cols = (UINT32*) of_malloc(ofcb->nb_total_symbols  * sizeof(UINT32));
	if ((index_rows == NULL) || (index_cols == NULL))
	{
		goto no_mem;
	}
	// Get the index of cols to copy from the original matrix
	for (_col = 0 ; _col < ofcb->nb_total_symbols ; _col++)
	{
		if (!of_mod2sparse_empty_col (ofcb->pchk_matrix, _col))
		{
			/* copy this col */
			index_cols[_col] = ofcb->remain_cols;		// old location
			ofcb->index_cols[ofcb->remain_cols++] = _col;	// new location
		}
	}
	// Get the index of rows to copy from the original matrix
	for (_row = 0 ; _row < ofcb->nb_repair_symbols ; _row++)
	{
		if (!of_mod2sparse_empty_row (ofcb->pchk_matrix, _row))
		{
			/* copy this row */
			index_rows[_row] = ofcb->remain_rows;
			ofcb->index_rows[ofcb->remain_rows++] = _row;
		}
	}
	if (ofcb->remain_cols == 0)
	{
		// the initial matrix is completely simplified, nothing remains.
		OF_PRINT_LVL (1, ("%s: Failure, initial matrix is now empty, ofcb->remain_cols==0\n", __FUNCTION__))
		goto failure;
	}
	else if (ofcb->remain_rows < ofcb->remain_cols)
	{
		// impossible to continue as there are fewer rows than columns.
		OF_PRINT_LVL (1, ("%s: Failure, fewer rows than columns, ofcb->remain_rows (%d) < ofcb->remain_cols (%d)\n",
				__FUNCTION__, ofcb->remain_rows, ofcb->remain_cols))
		goto failure;
	}
	OF_TRACE_LVL (1, ("%s: Simplified Matrix: %dx%d; already decoded symbols: ready_src=%d, ready_parity=%d  remaining_rows=%d  remaining_cols=%d\n",
				__FUNCTION__, ofcb->remain_rows, ofcb->remain_cols, ofcb->nb_source_symbol_ready,
				ofcb->nb_repair_symbol_ready, ofcb->remain_rows, ofcb->remain_cols))

	// allocate the new pchk_matrix_simplified, copy the non empty rows/cols in it
	ofcb->pchk_matrix_simplified = of_mod2sparse_allocate (ofcb->nb_repair_symbols, ofcb->remain_cols);
	of_mod2sparse_copy_filled_matrix(ofcb->pchk_matrix, ofcb->pchk_matrix_simplified, index_rows, index_cols);
	of_mod2sparse_free (ofcb->pchk_matrix);
	of_free (ofcb->pchk_matrix);
	ofcb->pchk_matrix = NULL;

	of_free(index_rows);
	of_free(index_cols);
	OF_TRACE_LVL (1, ("ok\n"))
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

failure:
	if (ofcb->index_rows)
	{
		of_free (ofcb->index_rows);
		ofcb->index_rows = NULL;
	}
	if (ofcb->index_cols)
	{
		of_free (ofcb->index_cols);
		ofcb->index_cols = NULL;
	}
	if (index_rows)
		of_free(index_rows);
	if (index_cols)
		of_free(index_cols);
	OF_EXIT_FUNCTION
	return OF_STATUS_FAILURE;

no_mem:
	OF_PRINT_ERROR(("out of memory"))
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


#endif //ML_DECODING

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif //OF_USE_DECODER
