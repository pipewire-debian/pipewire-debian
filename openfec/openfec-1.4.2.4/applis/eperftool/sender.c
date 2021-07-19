/* $Id: sender.c 207 2014-12-10 19:47:50Z roca $ */
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
 * local variables
 */
static void		**encoding_symbols_tab;	/* temporary symbol array needed by the FEC encoder */


of_status_t
init_sender (void)
{
	of_session_t	*ses;		/* pointer to a codec instance */
	block_cb_t	*blk;		/* temporary pointer within the blk_cb_tab[] */
	UINT32		sbn;		/* block sequence number */
	UINT32		k;		/* k parameter for a given block. Warning, the last block might be shorter */
	UINT32		n;		/* n parameter for a given block. Warning, the last block might be shorter */
	UINT32		esi;		/* Encoding Symbol ID */
	UINT32		src_idx;	/* index for a source symbol in the orig_symb[] table */
	UINT32		rep_idx;	/* index for a repair symbol in the orig_symb[] table */
	symbol_cb_t	*src_symb_cb;	/* pointer to a source symbol in the orig_symb[] table */
	symbol_cb_t	*rep_symb_cb;	/* pointer to a repair symbol in the orig_symb[] table */
	UINT32		tmp_max_k;	/* temporary value for max_k */
	UINT32		max_n_4_any_blk;/* maximum n value for any block */


#if 0
#ifdef WIN32
	QueryPerformanceCounter(&tv0);
	OF_PRINT(("init_start=%lI64f\n", (double)tv0.QuadPart/(double)freq.QuadPart))
#else
	gettimeofday(&tv0, NULL);
	OF_PRINT(("init_start=%ld.%d\n", tv0.tv_sec, tv0.tv_usec))
#endif
#endif
	/*
	 * determine the blocking structure, which requires to create a temporary FEC session.
	 */
	if (of_create_codec_instance(&ses, codec_id, OF_ENCODER, of_verbosity) != OF_STATUS_OK) {
		OF_PRINT_ERROR(("init_sender: ERROR: of_create_codec_instance() failed\n"))
		goto error;
	}
	if (codec_id == OF_CODEC_REED_SOLOMON_GF_2_M_STABLE) {
		if (of_set_control_parameter(ses, OF_RS_CTRL_SET_FIELD_SIZE, (void*)&rs_m_param, sizeof(rs_m_param)) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("init_sender: ERROR: of_set_control_parameter() failed\n"))
			goto error;
		}
	}
	if (of_get_control_parameter(ses, OF_CTRL_GET_MAX_K, (void*)&max_k, sizeof(max_k)) != OF_STATUS_OK) {
		OF_PRINT_ERROR(("init_sender: ERROR: of_get_control_parameter() failed\n"))
		goto error;
	}
	if (of_get_control_parameter(ses, OF_CTRL_GET_MAX_N, (void*)&max_n, sizeof(max_n)) != OF_STATUS_OK) {
		OF_PRINT_ERROR(("init_sender: ERROR: of_get_control_parameter() failed\n"))
		goto error;
	}
	if (of_release_codec_instance(ses) != OF_STATUS_OK) {
		OF_PRINT_ERROR(("init_sender: ERROR: of_release_codec_instance() failed\n"))
		goto error;
	}
#if 0
#ifdef WIN32
	QueryPerformanceCounter(&tv1);
	OF_PRINT(("init_end=%I64f  init_time=%I64f\n",
		(double)tv1.QuadPart / (double)freq.QuadPart,
		(double)(tv1.QuadPart-tv0.QuadPart) / (double)freq.QuadPart ))
#else
	gettimeofday(&tv1, NULL);
	timersub(&tv1, &tv0, &tv_delta);
	OF_PRINT(("init_end=%ld.%d  init_time=%ld.%06d\n",
		tv1.tv_sec, tv1.tv_usec, tv_delta.tv_sec, tv_delta.tv_usec))
#endif
#endif

	/*
	 * determine the practical maximum k and n parameters, taking into
	 * account the code/codec limitations and the desired code_rate.
	 * The idea is to have max_k maximum, given max_n and code_rate, for
	 * optimal erasure recovery performances.
	 * In any case, do not go below 1.
	 */
 	tmp_max_k = (UINT32)floor((double)max_n * code_rate);
	max_k = min(tmp_max_k, max_k);
	max_k = max(1, max_k);
	max_n = min((UINT32)((double)max_k / code_rate), max_n);
	max_n = max(1, max_n);
	/* we can now compute the required blocking structure */
	of_compute_blocking_struct(max_k, object_size, symbol_size, &bs);
	tot_nb_blocks = bs.nb_blocks;
	/*
	 * adjust tot_nb_encoding_symbols and tot_nb_encoding_symbols variables, now we know
	 * the exact blocking structure.
	 */
	tot_nb_encoding_symbols = (bs.I * (int)floor((double)(bs.A_large) / code_rate)) +
				  ((bs.nb_blocks - bs.I) * (int)floor((double)(bs.A_small) / code_rate));

	ASSERT(tot_nb_encoding_symbols <= tot_nb_source_symbols + tot_nb_repair_symbols);
	tot_nb_repair_symbols = tot_nb_encoding_symbols - tot_nb_source_symbols;

	OF_PRINT_LVL(1, ("Blocking_struct:\n\ttot_nb_source_symbols=%d, tot_nb_repair_symbols=%d, tot_nb_encoding_symbols=%d, code_rate=%.3f\n\tI=%d, tot_nb_blocks=%d, A_large=%d, A_small=%d\n",
		tot_nb_source_symbols, tot_nb_repair_symbols, tot_nb_encoding_symbols, code_rate,
		bs.I, tot_nb_blocks, bs.A_large, bs.A_small))

	/*
	 * check there is a single block in -find_min_overhead or -loss=5:nb modes, otherwise results are hazardous...
	 */
	if ((tot_nb_blocks > 1) && (trim_after_this_nb_rx_pkts >= 0))
	{
		OF_PRINT_ERROR(("ERROR: there are multiple blocks, which is not supported in -find_min_overhead or -loss=5:nb modes to avoid hazardous results!\n"))
		goto error;
	}

	/*
	 * allocate and initialize the original source and repair symbol buffers.
	 */
	if ((orig_symb = (char**)calloc(tot_nb_encoding_symbols, sizeof(char*))) == NULL) {
		OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
		goto no_mem;
	}
	/* source symbol buffers first... */
	for (src_idx = 0; src_idx < tot_nb_source_symbols; src_idx++) {
		char	*symb;
		UINT32	i;
		/*
		 * buffer is 0'ed... Leave it like that, except for the first
		 * four bytes where we copy the pkt seq number.
		 */
		if ((symb = (char*)calloc(1, symbol_size)) == NULL) {
			OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
			goto no_mem;
		}
		orig_symb[src_idx] = symb;
		/* fill each source symbol with some random content, except the first
		 * word which is equal to the symbol ID. This is useful to test the symbol
		 * integrity after decoding */
		for (i = 0; i < symbol_size; i++) {
			symb[i] = (char)rand();

		}
		*(UINT32 *)symb = (UINT32)src_idx;
		//symb[src_idx%symbol_size]=1;
		//of_print_composition(symb, symbol_size);
	}
	/* ... and then repair symbol buffers */
	for (rep_idx = tot_nb_source_symbols; rep_idx < tot_nb_encoding_symbols; rep_idx++) {
			orig_symb[rep_idx] = (char*)calloc(1, symbol_size);
		if (orig_symb[rep_idx] == NULL)  {
			OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
			goto no_mem;
		}
		/* repair symbols will be initialized later... */
	}
	/*
	 * now allocate the block and symbol control structures.
	 */
	if (!(blk_cb_tab = (block_cb_t*)calloc(tot_nb_blocks, sizeof(block_cb_t)))) {
		OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
		goto no_mem;
	}
	if (!(symb_cb_tab = (symbol_cb_t*)calloc(tot_nb_encoding_symbols, sizeof(symbol_cb_t)))) {
		OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
		goto no_mem;
	}
	/* ...and initialize the various block/symbol control structures */
	src_idx		= 0;
	src_symb_cb	= symb_cb_tab;
	rep_idx		= tot_nb_source_symbols;
	rep_symb_cb	= &(symb_cb_tab[tot_nb_source_symbols]);
	max_n_4_any_blk	= 0;
	for (sbn = 0, blk = blk_cb_tab; sbn < tot_nb_blocks; sbn++, blk++) {
		if (sbn < (UINT32)bs.I) {
			k = bs.A_large;
		} else {
			k = bs.A_small;
		}
		n = (UINT32)floor((double)k / code_rate);
		max_n_4_any_blk = (n < max_n_4_any_blk) ? max_n_4_any_blk : n;
		/* init block control block */
		blk->sbn			= sbn;
		blk->k				= k;
		blk->n				= n;
		blk->first_src_symbol_idx	= src_idx;
		blk->first_repair_symbol_idx	= rep_idx;
		blk->is_decoded			= false;
		blk->nb_symbols_received	= 0;
		OF_TRACE_LVL(1, ("init_sender: block: sbn=%d, k=%d, n=%d, first_src_symbol_idx=%d, 1st_rep_symbol_idx=%d\n",
			sbn, blk->k, blk->n,
			blk->first_src_symbol_idx, blk->first_repair_symbol_idx))
		/* init source symbols control block */
		for (esi = 0; esi < k; esi++, src_symb_cb++, src_idx++) {
			src_symb_cb->esi = esi;
			src_symb_cb->sbn = sbn;
		}
		/* and init repair symbols control block */
		for (esi = k; esi < n; esi++, rep_symb_cb++, rep_idx++) {
			rep_symb_cb->esi = esi;
			rep_symb_cb->sbn = sbn;
		}
	}
	/*
	 * allocate the table containing the various symbols of a block. This table
	 * is allocated once and reused by all blocks of the object, with pointers to
	 * different symbols of course, for encoding purposes.
	 */
	if (!(encoding_symbols_tab = (void**)calloc(max_n_4_any_blk, sizeof(void*)))) {
		OF_PRINT_ERROR(("init_sender: ERROR: out of memory\n"))
		goto no_mem;
	}
	return OF_STATUS_OK;
no_mem:
error:
	return OF_STATUS_ERROR;
}


of_status_t
encode (void)
{
	of_session_t	*ses;		/* pointer to a codec instance */
	block_cb_t	*blk;		/* temporary pointer within the blk_cb_tab[] */
	UINT32		sbn;		/* block sequence number */
	UINT32		k;		/* k parameter for a given block. Warning, the last block might be shorter */
	UINT32		n;		/* n parameter for a given block. Warning, the last block might be shorter */
	UINT32		esi;		/* Encoding Symbol ID */
	UINT32		i;

	/*
	 * go through each block of the object, initialize all the structures
	 * and create repair symbols.
	 */
#ifdef WIN32
	QueryPerformanceCounter(&tv0);
	OF_PRINT(("encoding_start=%lI64f\n", (double)tv0.QuadPart / (double)freq.QuadPart))
#else
	gettimeofday(&tv0, NULL);
	OF_PRINT(("encoding_start=%ld.%d\n", tv0.tv_sec, tv0.tv_usec))
#endif
	for (sbn = 0, blk = blk_cb_tab; sbn < tot_nb_blocks; sbn++, blk++) {
		k = blk->k;
		n = blk->n;
		/* don't forget to initialize the encoding symbol tab, used by the
		 * FEC codec during encoding, since we cannot use the orig_symb table
		 * where the source/repair symbols of a block are not sequential :-( */
		for (esi = 0; esi < k; esi++) {
			encoding_symbols_tab[esi] = (void*)(orig_symb[blk->first_src_symbol_idx + esi]);
		}
		for (; esi < n; esi++) {
			encoding_symbols_tab[esi] = (void*)(orig_symb[blk->first_repair_symbol_idx + (esi - k)]);
		}
		/*
		 * create the codec instance and initialize it accordingly.
		 * The case of a parity check matrix given in a file is handled
		 * differently...
		 */
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		if (codec_id == OF_CODEC_LDPC_FROM_FILE_ADVANCED)
		{
			if (of_create_codec_instance(&ses, codec_id, OF_ENCODER, of_verbosity) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_create_codec_instance() failed for codec_id %d\n", codec_id))
				goto error;
			}
			of_ldpc_ff_parameters_t		params;
			params.encoding_symbol_length	= symbol_size;
			params.pchk_file		= ldpc_matrix_file_name;
			if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed for codec_id %d\n", codec_id))
				goto error;
			}
			k = params.nb_source_symbols;
			n = params.nb_source_symbols + params.nb_repair_symbols;
		} else
#endif
		{
			ses = create_and_init_codec_instance(codec_id, OF_ENCODER, k, n, blk);
			if (ses == NULL) {
				OF_PRINT_ERROR(("ERROR: create_and_init_codec_instance() failed for codec_id %d/OF_ENCODER\n", codec_id))
				goto error;
			}
		}
		/*
		 * perform encoding and finally release the FEC codec instance.
		 */
		for (esi = k; esi < n; esi++) {
			if (of_build_repair_symbol(ses, encoding_symbols_tab, esi) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_build_repair_symbol() failed\n"))
				goto error;
			}
		}
		/*
		 * with LDPC-Staircase, if N1 is even, the last repair symbol is often null (if the code rate
		 * is above a certain threshold), so do not sent it since the codec already knows it.
		 */
		blk->ldpc_dont_send_last_repair = false;		/* by default */
		if (codec_id == OF_CODEC_LDPC_STAIRCASE_STABLE) {
			bool	lib_says_its_null;	/* boolean */

			if (of_get_control_parameter(ses, OF_CRTL_LDPC_STAIRCASE_IS_LAST_SYMBOL_NULL,
							(void*)&lib_says_its_null, sizeof(lib_says_its_null)) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_get_control_parameter() failed\n"))
				goto error;
			}
			if (lib_says_its_null) {
				OF_PRINT(("LDPC-Staircase, N1=%d, last repair symbol is null, do not send it\n", blk->ldpc_N1))
				blk->ldpc_dont_send_last_repair = true;
			}
#ifdef OF_DEBUG
			if (lib_says_its_null == true) {
				/* check that this symbol is indeed null */
				char	*zerosymb;		/* zero'ed symbol for memcmp */

				if ((zerosymb = (char*)calloc(1, symbol_size)) == NULL) {
					OF_PRINT_ERROR(("ERROR: out of memory\n"))
					goto error;
				}
				if (memcmp(encoding_symbols_tab[n-1], zerosymb, symbol_size) != 0) {
					ASSERT(0);
				}
				OF_PRINT(("LDPC-Staircase: last repair is indeed null!\n"))
				free(zerosymb);
			}
#endif
		}
		if (of_release_codec_instance(ses) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
			goto error;
		}
	}
#ifdef WIN32
	QueryPerformanceCounter(&tv1);
	OF_PRINT(("encoding_end=%I64f  encoding_time=%I64f\n",
		(double)tv1.QuadPart/(double)freq.QuadPart,
		(double)(tv1.QuadPart-tv0.QuadPart)/(double)freq.QuadPart ))
#else
	gettimeofday(&tv1, NULL);
	timersub(&tv1, &tv0, &tv_delta);
	OF_PRINT(("encoding_end=%ld.%d  encoding_time=%ld.%06d\n",
		tv1.tv_sec, tv1.tv_usec, tv_delta.tv_sec, tv_delta.tv_usec))
#endif
	free(encoding_symbols_tab);
	return OF_STATUS_OK;

error:
	return OF_STATUS_ERROR;
}

