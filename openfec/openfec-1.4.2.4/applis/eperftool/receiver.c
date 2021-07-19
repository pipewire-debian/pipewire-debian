/* $Id: receiver.c 207 2014-12-10 19:47:50Z roca $ */
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


/* AL-FEC extended performance evaluation tool */

#include "eperftool.h"


of_status_t
init_receiver (void)
{
	/*
	 * allocate and initialize the available (i.e. received and/or decoded) source and repair symbol buffers.
	 */
	if ((avail_symb = (char**)calloc(tot_nb_encoding_symbols, sizeof(char*))) == NULL) {
		goto no_mem;
	}
#if 0
	/*
	 * allocate the table containing the various symbols of a block. This table
	 * is allocated once and reused by all blocks of the object, with pointers to
	 * different symbols of course, for encoding purposes.
	 */
	if (!(encoding_symbols_tab = (void**)malloc(bs.A_large * sizeof(char*)))) {
		goto no_mem;
	}
#endif
	return OF_STATUS_OK;

no_mem:
	return OF_STATUS_ERROR;
}


of_status_t
receive_and_decode (void)
{
	//of_session_t	*ses;		/* pointer to a codec instance */
	block_cb_t	*blk;		/* temporary pointer within the blk_cb_tab[] */
	UINT32		sbn;		/* block sequence number */
	//UINT32	k;		/* k parameter for a given block. Warning, the last block might be shorter */
	//UINT32	n;		/* n parameter for a given block. Warning, the last block might be shorter */
	//UINT32	esi;		/* Encoding Symbol ID */
	//UINT32	i;
	UINT32		tot_nb_recvd_symbols;	/* total number of source/repair symbols received so far */
	UINT32		tot_nb_decoded_blocks;	/* total number of blocks that have been successfuly decoded so far */
	UINT32		new_symb_idx;	/* index of the new symbol just received in the orig_symb[] table */
	symbol_cb_t	*new_symb_cb;	/* pointer to the symbol just received */

	tot_nb_recvd_symbols = 0;
	tot_nb_decoded_blocks = 0;
#ifdef WIN32
	QueryPerformanceCounter(&tv0);
	OF_PRINT(("decoding_start=%lI64f\n", (double)tv0.QuadPart/(double)freq.QuadPart))
#else
	gettimeofday(&tv0, NULL);
	OF_PRINT(("decoding_start=%ld.%d\n", tv0.tv_sec, tv0.tv_usec))
#endif

	while ((new_symb_cb = get_next_symbol_received()) != NULL) {

		blk = &blk_cb_tab[new_symb_cb->sbn];
		/*
		 * with LDPC-Staircase, if N1 is even, the last repair symbol is often null (if the code rate
		 * is above a certain threshold), so do not sent it since the codec already knows it.
		 */
		if ((blk->ldpc_dont_send_last_repair) && (new_symb_cb->esi == blk->n - 1)) {
			/* skip this "last repair of a block" symbol as if it had not been sent.
			 * It's null and already known by the decoder. */
			OF_TRACE_LVL(1, ("receive_and_decode: skipped last repair (esi=%d)\n", new_symb_cb->esi))
			continue;
		}
		/* a new symbol is available... */
		tot_nb_recvd_symbols++;
		if (new_symb_cb->esi < blk->k) {
			new_symb_idx = blk->first_src_symbol_idx + new_symb_cb->esi;
			OF_TRACE_LVL(1, ("receive_and_decode: new source symbol available (sbn=%d, esi=%d)\n",
				new_symb_cb->sbn, new_symb_cb->esi))
		} else {
			new_symb_idx = blk->first_repair_symbol_idx + (new_symb_cb->esi - blk->k);
			OF_TRACE_LVL(1, ("receive_and_decode: new repair symbol available (sbn=%d, esi=%d)\n",
				new_symb_cb->sbn, new_symb_cb->esi))
		}
		if (blk->is_decoded || blk->is_abandoned) {
			/* block already decoded or skipped (e.g. after an error), ignore new packets for it */
			continue;
		}
		avail_symb[new_symb_idx] = orig_symb[new_symb_idx];
		blk->nb_symbols_received++;
		if (blk->ses == NULL) {
			/*
			 * create the codec instance and initialize it accordingly.
			 * The case of a parity check matrix given in a file is handled
			 * differently...
			 */
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
			if (codec_id == OF_CODEC_LDPC_FROM_FILE_ADVANCED) {
				if (of_create_codec_instance(&blk->ses, codec_id, OF_DECODER, of_verbosity) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_create_codec_instance() failed for codec_id %d\n", codec_id))
					goto error;
				}				
				 of_ldpc_ff_parameters_t	params;
				 params.encoding_symbol_length	= symbol_size;
				 params.pchk_file		= ldpc_matrix_file_name;
				 /* TODO: make this properly */
				 params.nb_source_symbols	= 1;	// set to 1 in order to pass the sanity checks
									// of of_set_fec_parameters
				 params.nb_repair_symbols	= 1;
				 if (of_set_fec_parameters(blk->ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed\n"))
					goto error;
				 }
				 //n=params.nb_source_symbols + params.nb_repair_symbols;
			} else 
#endif
			{
				blk->ses = create_and_init_codec_instance(codec_id, OF_DECODER, blk->k, blk->n, blk);
				if (blk->ses == NULL) {
					OF_PRINT_ERROR(("ERROR: create_and_init_codec_instance() failed for codec_id %d/OF_ENCODER\n", codec_id))
					goto error;
				}
			}
		}

		if (of_decode_with_new_symbol(blk->ses, avail_symb[new_symb_idx], new_symb_cb->esi) == OF_STATUS_ERROR) {
			/* an error occured, stop everything */
			of_release_codec_instance(blk->ses);
			OF_PRINT_ERROR(("receive_and_decode: ERROR: of_decode_with_new_symbol() failed\n"))
			goto error;
		}
		/* check if completed if we received nb_source packets or more */
		if ((blk->nb_symbols_received >= blk->k) && (of_is_decoding_complete(blk->ses) == true)) {
			/*
			 * done, at least for this block...
			 */
			tot_nb_decoded_blocks++;
			OF_TRACE_LVL(1, ("receive_and_decode: block sbn=%d decoded, total of %d blocks decoded after receiving %d symbols\n",
					blk->sbn, tot_nb_blocks, tot_nb_recvd_symbols))
			/* get a copy of the source symbols, those received (that we already know)
			 * and those decoded */
			if (of_get_source_symbols_tab(blk->ses, (void**)&(avail_symb[blk->first_src_symbol_idx])) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
				goto error;
			}
			/* then release the codec */
			if (of_release_codec_instance(blk->ses) != OF_STATUS_OK) {
				OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
				goto error;
			}
			blk->ses = NULL;
			blk->is_decoded = true;
			/* finally check if we finished everything or not */
			if (tot_nb_decoded_blocks == tot_nb_blocks) {
				/* it's almost finished, all blocks have been decoded */
				OF_TRACE_LVL(1, ("receive_and_decode: done, the %d blocks have been decoded after receiving %d symbols\n",
						tot_nb_blocks, tot_nb_recvd_symbols))
				break;
			}
		}
	}
	if (tot_nb_decoded_blocks < tot_nb_blocks) {
		/*
		 * one or several blocks have not been decoded... Let's try to finish decoding
		 * by calling of_finish_decoding() (e.g. with LDPC codes, this results in using
		 * ML decoding...
		 */
		for (sbn = 0, blk = blk_cb_tab; sbn < tot_nb_blocks; sbn++, blk++) {
			of_status_t	ret;
			if (blk->is_decoded == true || blk->is_abandoned == true) {
				continue;
			}
			if (blk->nb_symbols_received == 0) {
				/* this block is not decoded, sure, but we didn't receive
				 * anything, so let's give up... */
				continue;
			}
			ASSERT(blk->ses);
			ret = of_finish_decoding(blk->ses);
			if (ret == OF_STATUS_ERROR || ret == OF_STATUS_FATAL_ERROR) {
				OF_PRINT_ERROR(("ERROR: of_finish_decoding() failed with error (%d)\n", ret))
				if (of_release_codec_instance(blk->ses) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
					goto error;
				}
				blk->ses = NULL;
				continue;
			}
			if ((blk->nb_symbols_received >= blk->k) && (of_is_decoding_complete(blk->ses) == true)) {
				/*
				 * done, at least for this block...
				 */
				tot_nb_decoded_blocks++;
				OF_TRACE_LVL(1, ("receive_and_decode: block sbn=%d decoded after calling of_finish_decoding()\n", sbn))
				/* get a copy of the source symbols, those received (that we already know)
				 * and those decoded */
				if (of_get_source_symbols_tab(blk->ses, (void**)&(avail_symb[blk->first_src_symbol_idx])) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_get_source_symbols_tab() failed\n"))
					goto error;
				}
				/* then release the codec */
				if (of_release_codec_instance(blk->ses) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
					goto error;
				}
				blk->ses = NULL;
				blk->is_decoded = true;
				/* finally check if we finished everything or not */
				if (tot_nb_decoded_blocks == tot_nb_blocks) {
					/* it's almost finished, all blocks have been decoded */
					OF_TRACE_LVL(1, ("receive_and_decode: done, the %d blocks have been decoded after receiving %d symbols\n",
							tot_nb_blocks, tot_nb_recvd_symbols))
					break;
				}
			} else {
				/*
				 * decoding did not succeed for this block, but we need to release everything.
				 */
				OF_TRACE_LVL(1, ("receive_and_decode: block sbn=%d decoding failed after calling of_finish_decoding()\n", sbn))
				/* get a copy of the source symbols, those received (that we already know)
				 * and those decoded (even if some of them have not been decoded) */
					if (of_get_source_symbols_tab(blk->ses, (void**)&(avail_symb[blk->first_src_symbol_idx])) != OF_STATUS_OK) {
						OF_PRINT_ERROR(("ERROR: of_get_source_symbols_tab() failed\n"))
						goto error;
					}
				/* then release the codec */
				if (of_release_codec_instance(blk->ses) != OF_STATUS_OK) {
					OF_PRINT_ERROR(("ERROR: of_release_codec_instance() failed\n"))
					goto error;
				}
				blk->ses = NULL;
				blk->is_decoded = false;
			}
		}
	}
	if (tot_nb_decoded_blocks == tot_nb_blocks) {

		/* decoding successful */
#ifdef WIN32
		QueryPerformanceCounter(&tv1);
		OF_PRINT(("decoding_end=%I64f  decoding_time=%I64f  nb_received_symbols=%d  inefficiency_ratio=%.6f\n",
			(double)tv1.QuadPart/(double)freq.QuadPart,
			(double)(tv1.QuadPart-tv0.QuadPart)/(double)freq.QuadPart,
			tot_nb_recvd_symbols, (double)tot_nb_recvd_symbols/(double)tot_nb_source_symbols))
#else
		gettimeofday(&tv1, NULL);
		timersub(&tv1, &tv0, &tv_delta);
		OF_PRINT(("decoding_end=%ld.%d  decoding_time=%ld.%06d  nb_received_symbols=%d  inefficiency_ratio=%.6f\n",
			tv1.tv_sec, tv1.tv_usec,
			tv_delta.tv_sec, tv_delta.tv_usec,
			tot_nb_recvd_symbols, (double)tot_nb_recvd_symbols/(double)tot_nb_source_symbols))
#endif
#ifdef CHECK_INTEGRITY
		/*
		 * check that data received/recovered is the
		 * same as data sent
		 */
		{
		bool	integrity = true;
		OF_TRACE_LVL(1, ("receive_and_decode: now checking object integrity...\n"))
		for (new_symb_idx = 0; new_symb_idx < tot_nb_source_symbols; new_symb_idx++) {
			/*printf("### %i ###\n",new_symb_idx);
			of_print_composition(orig_symb[new_symb_idx], symbol_size);
			of_print_composition(avail_symb[new_symb_idx], symbol_size);
			printf("### %i ###\n",new_symb_idx);*/
			if (orig_symb[new_symb_idx] == NULL)
			{
				OF_PRINT_ERROR(("orig_symb[%d] NULL whereas decoding is finished.\n", new_symb_idx))
				integrity = false;
			}
			if (avail_symb[new_symb_idx] == NULL)
			{
				OF_PRINT_ERROR(("avail_symb[%d] NULL whereas decoding is finished.\n", new_symb_idx))
				integrity = false;
			}
			if (memcmp(orig_symb[new_symb_idx], avail_symb[new_symb_idx], symbol_size) != 0 ) {
				OF_PRINT_ERROR(("receive_and_decode: ERROR: symbol %d received/rebuilt doesn\'t match original\n", new_symb_idx))		
				integrity = false;
			}
		}
		if (integrity == false)
			goto error;
		OF_TRACE_LVL(1, ("receive_and_decode: finished, all source symbols are okay :-) ...\n"))
		}
#else  // CHECK_INTEGRITY
		OF_PRINT(("receive_and_decode: WARNING: decoding integrity is not checked (set CHECK_INTEGRITY for that)\n"))
#endif // CHECK_INTEGRITY
	} else {
		/* decoding failure */
		OF_PRINT(("FAILURE, did not manage to finish decoding even after receiving %d symbols, inefficiency_ratio>%.6f\n",
			tot_nb_recvd_symbols, (double)tot_nb_recvd_symbols / (double)tot_nb_source_symbols))
	}
	if (use_callback) {
		OF_TRACE_LVL(1, ("decode_source_symbol_callback has been called %i times\n",
				nb_decoded_src_symb_callback_calls))
	}

	/*
	 * Close and free everything.
	 */
	/* free buffer allocated internally during decoding */
	for (new_symb_idx = 0; new_symb_idx < tot_nb_source_symbols; new_symb_idx++) {
		if ((avail_symb[new_symb_idx] != NULL) &&
		    (avail_symb[new_symb_idx] != orig_symb[new_symb_idx])) {
			/* this symbol has been decoded by the codec */
			free(avail_symb[new_symb_idx]);
			avail_symb[new_symb_idx] = NULL;
		}
	}
	free(avail_symb);
	/* free all data and FEC packets created by the source */
	for (new_symb_idx = 0; new_symb_idx < tot_nb_encoding_symbols; new_symb_idx++) {
		free(orig_symb[new_symb_idx]);
		orig_symb[new_symb_idx] = NULL;
	}
	free(orig_symb);
	free(blk_cb_tab);
	free(symb_cb_tab);
	if (tot_nb_decoded_blocks != tot_nb_blocks)
	{
		return OF_STATUS_FAILURE;
	}
	else
	{
		return OF_STATUS_OK;
	}

error:
	/*
	 * Close and free everything.
	 */
	/* free buffer allocated internally during decoding */
	for (new_symb_idx = 0; new_symb_idx < tot_nb_source_symbols; new_symb_idx++) {
		if ((avail_symb[new_symb_idx] != NULL) &&
		    (avail_symb[new_symb_idx] != orig_symb[new_symb_idx])) {
			/* this symbol has been decoded by the codec */
			free(avail_symb[new_symb_idx]);
			avail_symb[new_symb_idx] = NULL;
		}
	}
	free(avail_symb);
	/* free all data and FEC packets created by the source */
	for (new_symb_idx = 0; new_symb_idx < tot_nb_encoding_symbols; new_symb_idx++) {
		free(orig_symb[new_symb_idx]);
	}
	free(orig_symb);
	free(blk_cb_tab);
	free(symb_cb_tab);
	return OF_STATUS_ERROR;
}

