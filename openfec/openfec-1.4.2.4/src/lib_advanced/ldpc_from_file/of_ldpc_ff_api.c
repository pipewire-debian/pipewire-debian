/* $Id: of_ldpc_ff_api.c 182 2014-07-15 09:27:51Z roca $ */
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

#include "of_ldpc_ff_includes.h"

#ifdef OF_USE_LDPC_FROM_FILE_CODEC


of_status_t of_ldpc_ff_create_codec_instance(of_ldpc_ff_cb_t**	of_cb)
{
	OF_ENTER_FUNCTION
	of_codec_type_t		codec_type;	/* temporary value */
	of_ldpc_ff_cb_t* ff_cb = (of_ldpc_ff_cb_t*) of_realloc(*of_cb, sizeof(of_ldpc_ff_cb_t));
	of_ldpc_staircase_cb_t* cb = 	(of_ldpc_staircase_cb_t*) ff_cb;

	*of_cb=ff_cb;
	/* realloc does not initialize the additional buffer space, so do that manually,
	 * then re-initialize a few parameters */
	codec_type			= cb->codec_type;
	memset(cb, 0, sizeof(*cb));
	//*of_cb				= cb;

	cb->codec_id			= OF_CODEC_LDPC_FROM_FILE_ADVANCED;
	cb->codec_type			= codec_type;
	cb->max_nb_source_symbols	= OF_LDPC_FROM_FILE_MAX_NB_SOURCE_SYMBOLS_DEFAULT;	/* init it immediately... */
	cb->max_nb_encoding_symbols	= OF_LDPC_FROM_FILE_MAX_NB_ENCODING_SYMBOLS_DEFAULT;	/* init it immediately... */
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


of_status_t	of_ldpc_ff_set_fec_parameters (of_ldpc_ff_cb_t*	cb,
					       of_ldpc_ff_parameters_t*	params)
{
	of_mod2entry	*e;
	UINT32		row;
	UINT32		seq;
	UINT32		matrix_nb_par;
	UINT32		matrix_nb_src;
	UINT32		*p_matrix_nb_par;
	UINT32		*p_matrix_nb_src;
	char		* m_matrix_file;
	FILE		*pFile;

	OF_ENTER_FUNCTION
	p_matrix_nb_src = &matrix_nb_src;
	p_matrix_nb_par = &matrix_nb_par;
#ifdef OF_DEBUG
	cb->cb1.stats_xor = of_calloc(1, sizeof(of_symbol_stats_op_t));
#endif
	/* open the matrix file */
	m_matrix_file=	params->pchk_file;
	of_ldpc_staircase_cb_t* ofcb = 	(of_ldpc_staircase_cb_t*) cb;
	pFile = fopen (m_matrix_file,"r");
	if (pFile == NULL)
	{
		OF_PRINT_ERROR(("of_ldpc_ff_set_fec_parameters : ERROR, cannot open matrix file %s",m_matrix_file))
		goto error;
	}
	ofcb->pchk_matrix = of_mod2sparse_read_human_readable(pFile, p_matrix_nb_src, p_matrix_nb_par);
	fclose(pFile);

	if (ofcb->pchk_matrix == NULL)
	{
		OF_PRINT_ERROR(("of_ldpc_ff_set_fec_parameters : ERROR, parity check matrix can't be created with this parameters.."))
		goto error;
	}

	cb->H2_is_identity_with_lower_triangle = true;

	of_mod2sparse_matrix_stats(stdout, ofcb->pchk_matrix, *p_matrix_nb_src, *p_matrix_nb_par);

	/* set ofcb attribute specific to from file code*/
	if ((ofcb->nb_source_symbols = matrix_nb_src ) > ofcb->max_nb_source_symbols) {
		OF_PRINT_ERROR(("of_ldpc_staircase_set_fec_parameters: ERROR, invalid nb_source_symbols parameter (got %d, maximum is %d)",
				ofcb->nb_source_symbols, ofcb->max_nb_source_symbols));
		goto error;
	}
	ofcb->nb_repair_symbols =matrix_nb_par;
	ofcb->nb_total_symbols = matrix_nb_src + matrix_nb_par;
	if (ofcb->nb_total_symbols > ofcb->max_nb_encoding_symbols) {
		OF_PRINT_ERROR(("of_ldpc_staircase_set_fec_parameters: ERROR, invalid number of encoding symbols (got %d, maximum is %d)",
				ofcb->nb_total_symbols, ofcb->max_nb_encoding_symbols));
		goto error;
	}

	params->nb_source_symbols = ofcb->nb_source_symbols;
	params->nb_repair_symbols = ofcb->nb_repair_symbols;

	/*set ofcb attribute non-specific to from file code */
	ofcb->encoding_symbol_length = params->encoding_symbol_length;

	OF_TRACE_LVL (1, ("%s: k=%u, n-k=%u, n=%u, symbol_length=%u, PRNG seed=%u, N1=%u\n", __FUNCTION__,
			ofcb->nb_source_symbols, ofcb->nb_repair_symbols, ofcb->nb_total_symbols,
			ofcb->encoding_symbol_length, ofcb->prng_seed, ofcb->N1))
#ifdef ML_DECODING
	ofcb->pchk_matrix_simplified = NULL;
#endif
	if ((ofcb->encoding_symbols_tab = (void**) of_calloc(ofcb->nb_total_symbols, sizeof(void*))) == NULL) {
		goto no_mem;
	}

#ifdef OF_USE_DECODER
	if (ofcb->codec_type & OF_DECODER)
	{
		ofcb->tab_nb_unknown_symbols = (UINT16*) of_calloc(ofcb->nb_repair_symbols, sizeof(UINT16));
		ofcb->tab_const_term_of_equ = (void**) of_calloc(ofcb->nb_repair_symbols, sizeof(void*));
		ofcb->tab_nb_equ_for_repair = (UINT16*) of_calloc(ofcb->nb_repair_symbols, sizeof(UINT16));
		ofcb->tab_nb_enc_symbols_per_equ = (UINT16*) of_calloc(ofcb->nb_repair_symbols, sizeof(UINT16));
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
#ifdef ML_DECODING
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


of_status_t	of_ldpc_ff_build_repair_symbol (of_ldpc_ff_cb_t*	ofcb,
						void*			encoding_symbols_tab[],
						UINT32			esi_of_symbol_to_build)
{
	if (ofcb->H2_is_identity_with_lower_triangle)
	{
		/* encoding is exactly the same as that of ldpc staircase codes */
		return of_ldpc_staircase_build_repair_symbol((of_ldpc_staircase_cb_t*)ofcb, encoding_symbols_tab, esi_of_symbol_to_build);
	}
	else
	{
		/* encoding requires to work on the generator matrix */
		OF_EXIT_FUNCTION
		return OF_STATUS_ERROR;
	}
}


of_status_t	of_ldpc_ff_set_control_parameter  (of_ldpc_ff_cb_t*	ofcb,
						UINT32			type,
						void*			value,
						UINT32			length)
{
	OF_PRINT_ERROR(("of_ldpc_ff_set_control_parameter: ERROR, not implemented...\n"))
	return OF_STATUS_ERROR;
}


of_status_t	of_ldpc_ff_get_control_parameter  (of_ldpc_ff_cb_t*	ofcb,
						UINT32			type,
						void*			value,
						UINT32			length)
{
	of_ldpc_staircase_cb_t	*ofcb_staircase;

	OF_ENTER_FUNCTION
	ofcb_staircase = (of_ldpc_staircase_cb_t*)ofcb;
	switch (type) {
	case OF_CTRL_GET_MAX_K:
		if (value == NULL || length != sizeof(UINT32)) {
			OF_PRINT_ERROR(("%s: OF_CTRL_GET_MAX_K ERROR: null value or bad length (got %d, expected %ld)\n",
				__FUNCTION__, length, sizeof(UINT32)))
			goto error;
		}
		*(UINT32*)value = ofcb_staircase->max_nb_source_symbols;
		OF_TRACE_LVL(1, ("%s: OF_CTRL_GET_MAX_K (%d)\n", __FUNCTION__, *(UINT32*)value))
		break;

	case OF_CTRL_GET_MAX_N:
		if (value == NULL || length != sizeof(UINT32)) {
			OF_PRINT_ERROR(("%s: OF_CTRL_GET_MAX_N ERROR: null value or bad length (got %d, expected %ld)\n",
				__FUNCTION__, length, sizeof(UINT32)))
			goto error;
		}
		*(UINT32*)value = ofcb_staircase->max_nb_encoding_symbols;
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


of_status_t  of_get_pck_matrix_dimensions_from_file(char * matrix_file,UINT32 * n_rows, UINT32 *n_cols){

	FILE * f;
	char * pch;
	char line[1024];

	f = fopen (matrix_file,"r");
	if (f == NULL) {
		OF_PRINT_ERROR(("Cannot open file %s\n",matrix_file))
		goto error;
	}
	// get the number of row of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		*n_rows = atoi (pch);
		//printf("nrows = %d\n",n_rows);
	}
	// get the number of columns of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		*n_cols = atoi (pch);
		//printf("ncol = %d\n",n_cols);
	}
	fclose(f);
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;

error:
	OF_EXIT_FUNCTION
	return OF_STATUS_ERROR;

}

#endif //OF_USE_LDPC_FROM_FILE_CODEC
