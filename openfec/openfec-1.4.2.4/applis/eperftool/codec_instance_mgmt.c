/* $Id: codec_instance_mgmt.c 115 2014-04-09 14:00:27Z roca $ */
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

#include "../../src/lib_common/of_openfec_api.h"

of_session_t *
create_and_init_codec_instance (of_codec_id_t	codec_id,
				UINT32		enc_dec_type,
				UINT32		k,
				UINT32		n,
				block_cb_t	*blk)
{
	of_session_t	*ses;		/* pointer to a codec instance */

	/*
	 * create the codec instance and initialize it accordingly.
	 */
	if (of_create_codec_instance(&ses, codec_id, enc_dec_type, of_verbosity) != OF_STATUS_OK) {
		OF_PRINT_ERROR(("ERROR: of_create_codec_instance() failed for codec_id %d\n", codec_id))
		goto error;
	}

#ifdef OF_USE_REED_SOLOMON_CODEC
	if (codec_id == OF_CODEC_REED_SOLOMON_GF_2_8_STABLE)
	{
		of_rs_parameters_t		params;
		params.nb_source_symbols	= k;
		params.nb_repair_symbols	= n - k;
		params.encoding_symbol_length	= symbol_size;
		if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed for codec_id %d\n", codec_id))
			goto error;
		}
	} else
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
	if (codec_id == OF_CODEC_REED_SOLOMON_GF_2_M_STABLE)
	{
		of_rs_2_m_parameters_t		params;
		params.nb_source_symbols	= k;
		params.nb_repair_symbols	= n - k;
		params.encoding_symbol_length	= symbol_size;
		params.m			= rs_m_param;
		if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed for codec_id %d\n", codec_id))
			goto error;
		}
	} else
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
	if (codec_id == OF_CODEC_LDPC_STAIRCASE_STABLE)
	{
		struct of_ldpc_parameters	params;
		params.nb_source_symbols	= k;
		params.nb_repair_symbols	= n - k;
		params.encoding_symbol_length	= symbol_size;
		if (enc_dec_type == OF_ENCODER) {
			if (suggested_seed != 0) {
				/* reuse the seed provided as eperftool argument */
				params.prng_seed = suggested_seed;
			} else {
				/* else use a new seed (and therefore new FEC code) each time */
				params.prng_seed = myrand();
			}
			params.N1			= ldpc_N1;
			blk->ldpc_seed			= params.prng_seed;	/* remember... */
			blk->ldpc_N1			= params.N1;		/* remember... */
		} else {
			params.prng_seed		= blk->ldpc_seed;
			params.N1			= blk->ldpc_N1;
		}
		if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed for codec_id %d\n", codec_id))
			goto error;
		}
	} else
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
	if (codec_id == OF_CODEC_2D_PARITY_MATRIX_STABLE)
	{
		of_2d_parity_parameters_t	params;
		params.nb_source_symbols	= k;
		params.nb_repair_symbols	= n - k;
		params.encoding_symbol_length	= symbol_size;
		if (of_set_fec_parameters(ses, (of_parameters_t*)&params) != OF_STATUS_OK) {
			OF_PRINT_ERROR(("ERROR: of_set_fec_parameters() failed for codec_id %d\n", codec_id))
			goto error;
		}
	} else 
#endif
	{
		OF_PRINT_ERROR(("ERROR: FEC codec %d not supported!\n", codec_id))
		goto error;
	}
	if (use_callback) {
		of_set_callback_functions(ses, decode_source_symbol_callback, NULL, NULL);
	}

	return ses;

error:
	return (of_session_t *)NULL;
}

