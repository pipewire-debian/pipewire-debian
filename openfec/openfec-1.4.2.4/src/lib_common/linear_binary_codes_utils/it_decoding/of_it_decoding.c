/* $Id: of_it_decoding.c 217 2014-12-13 13:55:01Z roca $ */
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


of_status_t of_linear_binary_code_decode_with_new_symbol (of_linear_binary_code_cb_t*	ofcb,
							  void*				new_symbol,
							  UINT32			new_symbol_esi)
{
	of_mod2entry	*e = NULL;			// entry ("1") in parity check matrix
	of_mod2entry	*mod_entry_to_delete;		// temp: entry to delete in row/column
	void		*const_term;			// temp: pointer to constant term, containing the sum of
							// all the known symbols of this equation
	UINT32		row;				// temp: current row value
	UINT32		*table_of_check_deg_1 = NULL;	// table of check nodes of degree
							// one after the processing of new_symbol
	INT32		table_of_check_deg_1_nb = 0;	// number of entries in table
	UINT32		size_of_table_of_check_deg_1 = 0; // size of the memory block
	UINT32		decoded_symbol_esi;		// sequence number of decoded symbol

	OF_ENTER_FUNCTION
	ASSERT(ofcb);
	ASSERT(new_symbol);
	ASSERT(new_symbol_esi < ofcb->nb_total_symbols);

	/*
	 * Step 0: check if this is a fresh symbol, otherwise return
	 */
	if (ofcb->encoding_symbols_tab[new_symbol_esi] != NULL)
	{
		OF_TRACE_LVL (1, ("%s: %s symbol (esi=%d) already received or rebuilt, ignored.\n", __FUNCTION__,
				(of_is_source_symbol ((of_cb_t*)ofcb, new_symbol_esi)) ? "source" : "parity", new_symbol_esi))
#ifdef OF_DEBUG
		if (of_is_source_symbol ((of_cb_t*)ofcb, new_symbol_esi))
			ofcb->stats_symbols->nb_source_symbols_ignored++;
		else
			ofcb->stats_symbols->nb_repair_symbols_ignored++;
#endif
		OF_EXIT_FUNCTION
		return OF_STATUS_OK;
	}
	
	//of_mod2sparse_print_bitmap(ofcb->pchk_matrix);
	
	/*
	 * Step 1: Store the symbol in a permanent array.
	 */
	if (of_is_source_symbol ((of_cb_t*)ofcb, new_symbol_esi))
	{
		ofcb->nb_source_symbol_ready++;
#ifdef OF_DEBUG
		ofcb->stats_symbols->nb_source_symbols_received++;
#endif
		ofcb->encoding_symbols_tab[new_symbol_esi] = new_symbol;
		if (of_is_decoding_complete ((of_session_t*)ofcb))
		{
			OF_TRACE_LVL (1, ("%s: decoding finished\n", __FUNCTION__))
			OF_EXIT_FUNCTION
			return OF_STATUS_OK;
		}
	}
	else
	{
		ofcb->nb_repair_symbol_ready++;
#ifdef OF_DEBUG
		ofcb->stats_symbols->nb_repair_symbols_received++;
#endif
		// in ML decoding, we need to store all parity symbols, whereas with pure IT
		// decoding this is not necessary.
		if ((ofcb->encoding_symbols_tab[new_symbol_esi] = (void *)
						of_malloc (ofcb->encoding_symbol_length)) == NULL)
		{
			goto no_mem;
		}
		// copy the content...
		memcpy (ofcb->encoding_symbols_tab[new_symbol_esi], new_symbol, ofcb->encoding_symbol_length);
	}
	OF_TRACE_LVL (1, ("%s: new %s symbol (esi=%d), total of %d/%d source/repair symbols ready\n",
			__FUNCTION__, (of_is_source_symbol ((of_cb_t*)ofcb, new_symbol_esi)) ? "source" : "parity",
			new_symbol_esi, ofcb->nb_source_symbol_ready, ofcb->nb_repair_symbol_ready))

	/*
	 * Step 2: Inject the symbol value in each equation it is involved
	 */
	// (if partial sum already exists or if partial sum should be created)
	for (e = of_mod2sparse_first_in_col (ofcb->pchk_matrix, of_get_symbol_col ((of_cb_t*)ofcb, new_symbol_esi)); !of_mod2sparse_at_end_col (e); )
	{
		// for a given row, ie for a given equation where this symbol
		// is implicated, do the following:
		row = e->row;
		ofcb->tab_nb_unknown_symbols[row]--;		// symbol is known
		const_term = ofcb->tab_const_term_of_equ[row];	// associated partial sum buffer (if any)
		if ((const_term == NULL) && ((ofcb->tab_nb_unknown_symbols[row] == 1)))
		{
			// we need to allocate a partial sum (i.e. check node)
			// and add the symbol to it, because it is the
			// last missing symbol of this equation.
			const_term = (void*) of_calloc (1, ofcb->encoding_symbol_length);
			if ((ofcb->tab_const_term_of_equ[row] = const_term) == NULL)
			{
				goto no_mem;
			}
		}
		if (const_term != NULL)
		{
			mod_entry_to_delete = e;
			of_mod2entry	*tmp_e;		// current symbol in this equation
			UINT32		tmp_esi;	// corresponding esi
			void		*tmp_symbol;	// corresponding symbol pointer

			// there's a partial sum for this row...
			if (ofcb->tab_nb_enc_symbols_per_equ[row] > 1)
			{
				//ofcb->tmp_tab_symbols[ofcb->nb_tmp_symbols++] = new_symbol;
				// we can add the symbol content to this constant term
				of_add_to_symbol (const_term, new_symbol, ofcb->encoding_symbol_length
#ifdef OF_DEBUG
						, &(ofcb->stats_xor->nb_xor_for_IT)
#endif
						);
			}
			// else this is useless, since new_symbol is the last
			// symbol of this equation, and its value is necessarilly
			// equal to the constant term. Their sum must be 0 (we don't check it).
			// Remove the symbol from the equation since this entry is now useless.
			e = of_mod2sparse_next_in_col (e);
			of_mod2sparse_delete (ofcb->pchk_matrix, mod_entry_to_delete);
			ofcb->tab_nb_enc_symbols_per_equ[row]--;
			if (of_is_repair_symbol ((of_cb_t*)ofcb, new_symbol_esi))
			{
				ofcb->tab_nb_equ_for_repair[new_symbol_esi - ofcb->nb_source_symbols]--;
			}
			// Inject all permanently stored symbols
			// (source and repair) into this partial sum.
			// Requires to scan the equation (i.e. row).
			for (tmp_e = of_mod2sparse_first_in_row (ofcb->pchk_matrix, row); !of_mod2sparse_at_end_row (tmp_e); )
			{
				tmp_esi = of_get_symbol_esi ((of_cb_t*)ofcb, tmp_e->col);
				tmp_symbol = ofcb->encoding_symbols_tab[tmp_esi];
				if (tmp_symbol != NULL)
				{
					// add the symbol content now
					of_add_to_symbol (const_term, tmp_symbol, ofcb->encoding_symbol_length
#ifdef OF_DEBUG
							, &(ofcb->stats_xor->nb_xor_for_IT)
#endif
							);
					// delete the entry
					mod_entry_to_delete = tmp_e;
					tmp_e =  of_mod2sparse_next_in_row (tmp_e);
					of_mod2sparse_delete (ofcb->pchk_matrix, mod_entry_to_delete);
					ofcb->tab_nb_enc_symbols_per_equ[row]--;
					if (of_is_repair_symbol ((of_cb_t*)ofcb, tmp_esi))
					{
						ofcb->tab_nb_equ_for_repair[tmp_esi - ofcb->nb_source_symbols]--;
#ifndef ML_DECODING
						// we can delete parity symbol altogether if in IT decoding only,
						// if ML decoding is needed, then keep the repair symbol
						if (ofcb->tab_nb_equ_for_repair[tmp_esi - ofcb->nb_source_symbols] == 0)
						{
							of_free (tmp_symbol);
							ofcb->encoding_symbols_tab[tmp_esi] = NULL;
						}
#endif
					}
				}
				else
				{
					// this symbol is not yet known, switch to next one in equation
					tmp_e =  of_mod2sparse_next_in_row (tmp_e);
				}
			}
		}
		else
		{
			// here m_checkValues[row] is NULL, ie. the partial
			// sum has not been allocated
			e = of_mod2sparse_next_in_col (e);
		}
		if (ofcb->tab_nb_enc_symbols_per_equ[row] == 1)
		{
			// register this entry for step 3 since the symbol
			// associated to this equation can now be decoded...
			if (table_of_check_deg_1 == NULL)
			{
				// allocate memory for the table first
				size_of_table_of_check_deg_1 = 4;
				if ((table_of_check_deg_1 = (UINT32*) of_calloc (size_of_table_of_check_deg_1, sizeof (UINT32*))) == NULL)
				{
					goto no_mem;
				}
			}
			else if (table_of_check_deg_1_nb == size_of_table_of_check_deg_1)
			{
				// not enough size in table, add some more
				size_of_table_of_check_deg_1 += 4;
				if ((table_of_check_deg_1 = (UINT32*) of_realloc (table_of_check_deg_1, size_of_table_of_check_deg_1 * sizeof (UINT32*))) == NULL)
				{
					goto no_mem;
				}
			}
			table_of_check_deg_1[table_of_check_deg_1_nb++] = row;
		}
	}

	/*
	 * Step 3: Check if a new symbol has been decoded and take appropriate measures ...
	 */
	for (table_of_check_deg_1_nb--; table_of_check_deg_1_nb >= 0; table_of_check_deg_1_nb--)
	{
		if (of_is_decoding_complete ((of_session_t*)ofcb))
		{
			/* decoding has just finished, no need to do anything else.
			 * Stop this decoding loop and return immediately. */
			break;
		}
		// get the index (ie row) of the partial sum concerned
		row = table_of_check_deg_1[table_of_check_deg_1_nb];
		if (ofcb->tab_nb_enc_symbols_per_equ[row] == 1)
		{
			// A new decoded symbol is available...
			// NB: because of the recursion below, we need to
			// check that all equations mentioned in the
			// table_of_check_deg_1 list are __still__ of degree 1.
			e = of_mod2sparse_first_in_row (ofcb->pchk_matrix, row);
			ASSERT (!of_mod2sparse_at_end_row (e) && of_mod2sparse_at_end_row (e->right))
			decoded_symbol_esi = of_get_symbol_esi ((of_cb_t*)ofcb, e->col);
			// remove the entry from the matrix
			const_term = ofcb->tab_const_term_of_equ[row];	// remember it
			ofcb->tab_const_term_of_equ[row] = NULL;
			ofcb->tab_nb_enc_symbols_per_equ[row]--;
			if (of_is_repair_symbol ((of_cb_t*)ofcb, decoded_symbol_esi))
			{
				ofcb->tab_nb_equ_for_repair[decoded_symbol_esi - ofcb->nb_source_symbols]--;
			}
			of_mod2sparse_delete (ofcb->pchk_matrix, e);
			OF_TRACE_LVL (1, ("%s: REBUILT %s symbol %d\n", __FUNCTION__,
					  (of_is_repair_symbol ((of_cb_t*)ofcb, decoded_symbol_esi)) ? "Parity" : "Source",
					  decoded_symbol_esi));
			if (of_is_source_symbol ((of_cb_t*)ofcb, decoded_symbol_esi))
			{
				// source symbol.
				void	*decoded_symbol_dst;	// temp variable used to store symbol
#ifdef OF_DEBUG
				ofcb->stats_symbols->nb_source_symbols_built_with_it++;
				ofcb->stats_symbols->nb_source_symbols_received--;
#endif
				// First copy it into a permanent buffer.
				if (ofcb->decoded_source_symbol_callback != NULL)
				{
					// Call the associated callback (that allocates a buffer) and then
					// copy the symbol content in it.
					if ((decoded_symbol_dst = ofcb->decoded_source_symbol_callback(
										ofcb->context_4_callback,
										ofcb->encoding_symbol_length,
										decoded_symbol_esi)) != NULL)
					{
						// if the application has allocated a buffer, copy the symbol into it.
						memcpy (decoded_symbol_dst, const_term, ofcb->encoding_symbol_length);
						// we don't need the const_term buffer any more, so free it.
						of_free (const_term);
					}
					else
					{
						// else reuse the const_term buffer in order to save extra malloc/memcpy.
						decoded_symbol_dst = const_term;
					}
				}
				else
				{
					// else reuse the const_term buffer in order to save extra malloc/memcpy.
					decoded_symbol_dst = const_term;
				}
				// And finally call this function recursively
				of_linear_binary_code_decode_with_new_symbol (ofcb, decoded_symbol_dst, decoded_symbol_esi);
			}
			else
			{
#ifdef OF_DEBUG
				ofcb->stats_symbols->nb_repair_symbols_built_with_it++;
				ofcb->stats_symbols->nb_repair_symbols_received--;
#endif
				// Parity symbol.
				if (ofcb->decoded_repair_symbol_callback != NULL)
				{
					ofcb->decoded_repair_symbol_callback   (ofcb->context_4_callback,
										ofcb->encoding_symbol_length,
										decoded_symbol_esi);
				}
				// Call this function recursively first...
				of_linear_binary_code_decode_with_new_symbol (ofcb, const_term, decoded_symbol_esi);
				// ...then free the partial sum which is no longer needed.
				of_free (const_term);
			}
		}
	}
	if (table_of_check_deg_1 != NULL)
	{
		of_free (table_of_check_deg_1);
	}
	//OF_TRACE_LVL(1,("max:%u,cur:%u\n",ofcb->stats->maximum_mem,ofcb->stats->current_mem))

	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

no_mem:
	OF_PRINT_ERROR(("out of memory\n"));
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif //OF_USE_DECODER
