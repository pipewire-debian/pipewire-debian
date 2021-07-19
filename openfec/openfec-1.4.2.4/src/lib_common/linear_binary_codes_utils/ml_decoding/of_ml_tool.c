/* $Id: of_ml_tool.c 213 2014-12-12 22:36:41Z roca $ */
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

#include "../of_linear_binary_code.h"

#ifdef OF_USE_DECODER
#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS
#ifdef ML_DECODING


/******  Static Functions  ****************************************************/


/**
 * This function transforms the matrix into a triangular matrix. It goes through each column and calls of_linear_binary_code_col_forward_elimination()
 * to find the pivot and eliminate '1's below the pivot.
 *
 * @brief			triangularize the dense system
 * @param m			(IN/OUT) address of the dense matrix.
 * @param constant_tab		(IN/OUT)
 * @param ofcb			(IN) Linear-Binary-Code control-block.
 * @return			1 if it's OK, or 0 if an error took place.
 */
static INT32
of_linear_binary_code_triangularize_dense_system (of_linear_binary_code_cb_t	*ofcb,
						  of_mod2dense			*m,
						  void				**constant_tab);


/**
 * This function finds the pivot for this column and eliminates '1's below this pivot.
 *
 * @brief			eliminates "1" entries in parity check matrix
 * @param m 			(IN/OUT) address of the dense matrix.
 * @param constant_tab		(IN/OUT) pointer to all constant members
 * @param ofcb			(IN/OUT) Linear-Binary-Code control-block.
 * @param col_idx		(IN) column index
 * @param stats			(IN)
 * @return			1 if it's OK, or 0 if an error took place.
 */
static INT32
of_linear_binary_code_col_forward_elimination  (of_linear_binary_code_cb_t	*ofcb,
						of_mod2dense			*m,
						void				**constant_tab,
						INT32				col_idx);


/**
 * This function computes the actual values of the symbols, starting from the bottom row up to the first one. It assumes the parity check matrix has
 * already been transformed into a triangular matrix.
 *
 * @brief			solve system with backward substitution
 * @param variable_tab		(IN/OUT) address of the dense matrix.
 * @param constant_tab
 * @param m 			(IN/OUT) address of the dense matrix.
 * @param ofcb			(IN/OUT) Linear-Binary-Code control-block.
 * @return			1 if it's OK, or 0 if an error took place.
 */
static INT32
of_linear_binary_code_backward_substitution (of_linear_binary_code_cb_t	*ofcb,
					     of_mod2dense		*m,
					     void			*variable_tab[],
					     void			*constant_tab[]);


/******************************************************************************/



/**
 * This function solves the system: first triangularize the system, then for each column,
 * do a forward elimination, then do the backward elimination.
 */
of_status_t
of_linear_binary_code_solve_dense_system (of_linear_binary_code_cb_t	*ofcb,
					  of_mod2dense			*m,
					  void				**constant_tab,
					  void				**variable_tab)
{
	OF_ENTER_FUNCTION
	if (!of_linear_binary_code_triangularize_dense_system (ofcb, m, constant_tab))
	{
		OF_TRACE_LVL(0,("%s: triangularize_dense_system failed for system with %d rows, %d cols\n",
				__FUNCTION__, of_mod2dense_rows(m), of_mod2dense_cols(m)))
		OF_EXIT_FUNCTION
		return OF_STATUS_FAILURE;
	}
	//of_mod2dense_print_bitmap(m);
	if (!of_linear_binary_code_backward_substitution (ofcb, m, variable_tab, constant_tab))
	{
		OF_TRACE_LVL(0,("%s: backward_substitution failed\n", __FUNCTION__))
		OF_EXIT_FUNCTION
		return OF_STATUS_FAILURE;
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


/******  Static Functions  ****************************************************/


static
INT32	of_linear_binary_code_triangularize_dense_system (of_linear_binary_code_cb_t	*ofcb,
							  of_mod2dense			*m,
							  void				**constant_tab)
{
	INT32 i, n;

	OF_ENTER_FUNCTION
	n = of_mod2dense_cols (m);
	/* for each row */
	for (i = 0; i < n; i++)
	{
		if (!of_linear_binary_code_col_forward_elimination(ofcb, m, constant_tab, i))
		{
			OF_EXIT_FUNCTION
			return 0;
		}
	}
	OF_EXIT_FUNCTION
	return 1;
}


static
INT32	of_linear_binary_code_col_forward_elimination  (of_linear_binary_code_cb_t	*ofcb,
								of_mod2dense			*m,
								void				**constant_tab,
								INT32				col_idx)
{
	of_mod2word	*s;
	of_mod2word	*t;
	INT32		i, j, k;
	INT32		n, p;
	INT32		w;
	INT32		w0;
	INT32		b0;
	INT32		symbol_size;
	void		*tmp_buffer;

	OF_ENTER_FUNCTION
	symbol_size = ofcb->encoding_symbol_length;
	n = of_mod2dense_cols (m);
	p = of_mod2dense_rows (m);
	w = m->n_words;
	i = col_idx;
	//printf("of_col_forward_elimination of col %d\n",i);
	w0 = i >> of_mod2_wordsize_shift;	// word index of the ith bit
	b0 = i & of_mod2_wordsize_mask;		// bit index of the ith bit in the w0-th word
	/* search for the first non-null element of that column (pivot) */
	for (j = i; j < p; j++)
	{
		if (of_mod2_getbit(m->row[j][w0], b0))
			break;
	}
	/* now j is the index of the first non null element in column i */
	if (j == p)
	{
		/* it's a failure, it's not possible to choose a pivot for this empty column */
		OF_EXIT_FUNCTION
		return 0;
	}
	if (j != i)
	{
		// swap columns i and j in the two matrices
		//printf("swap rows %i and %i\n",j,i);
		t = m->row[i];
		m->row[i] = m->row[j];
		m->row[j] = t;
		// swap the partial sum
		tmp_buffer = constant_tab[i];
		constant_tab[i] = constant_tab[j];
		constant_tab[j] = tmp_buffer;
		//of_mod2dense_print(stdout,m);
	}
	/* we have found the pivot and made sure that it is at row i (potentially after swapping two rows).
	 * Now eliminate the other '1's below this pivot... */
	ofcb->nb_tmp_symbols = 0;
	for (j = i + 1; j < p; j++)
	{
		if (of_mod2_getbit(m->row[j][w0], b0))
		{
			/* there's a '1' below the pivot, so XOR the two rows (word by word to go faster). */
			//printf("\n\nxor in %i, %i\n",j,i);
			s = m->row[j];
			t = m->row[i];
			s += w0;
			t += w0;
			for (k = w0; k < w; k++)
			{
				*s ^= *t;
				s++;
				t++;
			}
			//of_mod2dense_print(stdout,m);
			if (constant_tab[i] != NULL)
			{
				/* if the buffer of line i is NULL there is nothing to add.
				 * add the constant term of the i to the constant term of line j */
				if (constant_tab[j] == NULL)
				{
					constant_tab[j] = of_malloc (symbol_size);
					/* copy data directly, there's no XOR to perform */
					memcpy (constant_tab[j], constant_tab[i], symbol_size);
				}
				else
				{
					/* add constant term i to constant term j */
					ofcb->tmp_tab_symbols[ofcb->nb_tmp_symbols++] = constant_tab[j];
				}
			}
			else
			{
				constant_tab[i] = of_calloc (1, symbol_size);
			}
		}
	}
	if (ofcb->nb_tmp_symbols != 0)
	{
		if (ofcb->nb_tmp_symbols == 1)
		{
			of_add_to_symbol (ofcb->tmp_tab_symbols[0], constant_tab[i], symbol_size OP_ARG_VAL);
		}
		else
		{
			of_add_to_multiple_symbols (ofcb->tmp_tab_symbols,
						    constant_tab[i],
						    ofcb->nb_tmp_symbols,
						    ofcb->encoding_symbol_length
#ifdef OF_DEBUG
						    , &(ofcb->stats_xor->nb_xor_for_ML)
#endif
						   );
		}
	}
	OF_EXIT_FUNCTION
	return 1;
}


/* old trivial backward substitution algorithm. */
static
INT32	of_linear_binary_code_backward_substitution    (of_linear_binary_code_cb_t	*ofcb,
							of_mod2dense			*m,
							void				*variable_tab[],
							void				*constant_tab[])
{
	INT32	i;		/* current variable index for which we apply backward substition. It's also the row index. */
	INT32	j;		/*  */
	INT32	n;
	INT32	w0;		/* dense matrix word index for variable j */
	INT32	b0;		/* dense matrix bit index in word of index w0 */

	OF_ENTER_FUNCTION
	n = of_mod2dense_cols (m);
	/* go through all the rows, starting from the last one... */
	for (i = n - 1; i >= 0; i--)
	{
		ASSERT(variable_tab[i] == NULL);
		//if (variable_tab[i] == NULL)
		{
			of_mod2word	*row = m->row[i];		// row corresponding to variable i
#ifdef OF_DEBUG
			w0 = i >> of_mod2_wordsize_shift;	// word index of the ith bit
			b0 = i & of_mod2_wordsize_mask;		// bit index of the ith bit in the w0-th word
			ASSERT(of_mod2_getbit(row[w0], b0))
#endif
			/*
			 * the missing source symbol in col i is equal to the sum of the constant term of this
			 * equation (i.e. row i) plus all the variables of this equation.
			 */
			//OF_TRACE_LVL(1, ("%s: rebuilding source symbol %d with col %d\n", __FUNCTION__, col_index[i], i))
			ASSERT(variable_tab[i] == NULL);
			variable_tab[i] = constant_tab[i];
			constant_tab[i] = NULL;
			/* determine the list of symbols to add to compute the decoded source symbol */
			ofcb->nb_tmp_symbols = 0;
			for (j = i + 1; j < n; j++)
			{
				w0 = j >> of_mod2_wordsize_shift;	// word index of the ith bit
				b0 = j & of_mod2_wordsize_mask;		// bit index of the ith bit in the w0-th word
				/* search for the non-null element in row i */
				if (of_mod2_getbit(row[w0], b0))
				{
					/* since the bit is set, add the variable_tab[j] symbol */
					ofcb->tmp_tab_symbols[ofcb->nb_tmp_symbols++] = variable_tab[j];
				}
			}
			if (ofcb->nb_tmp_symbols != 0)
			{
				of_add_from_multiple_symbols(variable_tab[i], (const void**)ofcb->tmp_tab_symbols,
							     ofcb->nb_tmp_symbols, ofcb->encoding_symbol_length
#ifdef OF_DEBUG
							     , &(ofcb->stats_xor->nb_xor_for_ML)
#endif
								);
			}
		}
	}
	OF_EXIT_FUNCTION
	return 1;
}


#endif //ML_DECODING
#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
#endif //OF_USE_DECODER
