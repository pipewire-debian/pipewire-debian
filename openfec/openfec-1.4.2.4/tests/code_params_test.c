/* $Id: code_params_test.c 530 2010-10-25 14:43:42Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 INRIA - All rights reserved
 * Main authors:	Mathieu Cunche (INRIA)
 *			Jonathan Detchart (INRIA)
 *			Julien Laboure (INRIA)
 *			Christoph Neumann (INRIA)
 *			Vincent Roca (INRIA)
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

/**
 * This small program creates a high number of sessions of each type
 * (code/codec), creates a certain number of repair packets for each
 * session, and then closes the session.
 * This test is useful to check, with a very coarse grain, that there
 * is no big issue (e.g. memory leak) in one of the provided codecs.
 *
 * This is also a small application showing the common API.
 */

#define OF_USE_ENCODER
#define OF_USE_DECODER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lib_common/of_openfec_api.h"
#include "../src/lib_common/of_debug.h"

#define MIN(a,b)	((a) <= (b) ? (a) : (b))

#define MAX_CODEC_ID	11
#define MAX_ITER	100		/* MAX_ITER iterations per code/codec */
#define SYMBOL_LEN	1024

int main()
{
	of_codec_id_t	codec_id;
	UINT32		i, j;
	UINT32		k, n;
	UINT32		max_k, max_n;
	UINT32		iter;
	UINT32		esi;
	of_session_t	*ses;
	of_status_t	ret;
	char		**orig_symb_tab;
	void		**encoding_symb_tab;
	of_parameters_t	*params = NULL;
	
	for (codec_id = 1; codec_id <= MAX_CODEC_ID; codec_id++)
	{
		printf("\nCodec %d ", codec_id);
		switch ((int)codec_id)
		{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE: {
			printf("(OF_CODEC_REED_SOLOMON_GF_2_8_STABLE):\n\t");
			break;
			}
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE: {
			printf("(OF_CODEC_REED_SOLOMON_GF_2_M_STABLE):\n\t");
			break;
			}
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE: {
			printf("(OF_CODEC_LDPC_STAIRCASE_STABLE):\n\t");
			break;
			}
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE: {
			printf("(OF_CODEC_2D_PARITY_MATRIX_STABLE):\n\t");
			break;
			}
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			// The "from file" codec needs a valid pointer to file in params structure.
			// So, we skip this codec for the moment...
			printf("(OF_CODEC_LDPC_FROM_FILE_ADVANCED):\n\tSkipped...\n");
			continue;
#endif
		default:
			printf("unknown codec:\n\tIgnored...\n");
			continue;
		}

		/* create, initalize and release MAX_ITER codec instances in sequence */
		for (iter = 0; iter <= MAX_ITER; iter++)
		{
			printf("%d ", iter);
			fflush(stdout);
			if ((ret = of_create_codec_instance(&ses, codec_id, OF_ENCODER, 0)) != OF_STATUS_OK)
			{
				printf("of_create_codec_instance: ERROR for codec %d\n", codec_id);
				goto error;
			}
	#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
			if (codec_id == OF_CODEC_REED_SOLOMON_GF_2_M_STABLE)
			{
				UINT16	fs = 8;	/* let's use GF(2^8) */
				/* in this particular case, we need to set the field parameter first before accessing MAX_K or MAX_N! */
				if (of_set_control_parameter(ses, OF_RS_CTRL_SET_FIELD_SIZE, (void*)&fs, sizeof(fs)) != OF_STATUS_OK)
				{
					printf("of_set_control_parameter(): ERROR for codec %d, OF_RS_CTRL_SET_FIELD_SIZE failed\n", codec_id);
					goto error;			
				}
			}
	#endif
			if (of_get_control_parameter(ses, OF_CTRL_GET_MAX_K, (void*)&max_k, sizeof(max_k)) != OF_STATUS_OK)
			{
				printf("of_get_control_parameter(): ERROR for codec %d\n", codec_id);
				goto error;			
			}
			if (of_get_control_parameter(ses, OF_CTRL_GET_MAX_N, (void*)&max_n, sizeof(max_n)) != OF_STATUS_OK)
			{
				printf("of_get_control_parameter(): ERROR for codec %d\n", codec_id);
				goto error;				
			}	
			/* code/codec specific code follows (this is the only place) */
			switch ((int)codec_id)
			{
#ifdef OF_USE_REED_SOLOMON_CODEC
			case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE: {
				of_rs_parameters_t	*my_params;

				n = max_n;
				k = MIN(max_k, n * 0.5); /* test with code rate 1/2 */
				my_params = (of_rs_parameters_t *)calloc(1, sizeof(* my_params));
				if (my_params == NULL)
				{
					printf("calloc: ERROR, no memory for codec %d\n", codec_id);
					goto error;
				}
				params = (of_parameters_t *) my_params;
				break;
				}
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE: {
				of_rs_2_m_parameters_t	*my_params;

				n = max_n;
				k = MIN(max_k, n * 0.5); /* test with code rate 1/2 */
				my_params = (of_rs_2_m_parameters_t *)calloc(1, sizeof(* my_params));
				if (my_params == NULL)
				{
					printf("calloc: ERROR, no memory for codec %d\n", codec_id);
					goto error;
				}
				my_params->m = 8;
				params = (of_parameters_t *) my_params;
				break;
				}
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
			case OF_CODEC_LDPC_STAIRCASE_STABLE: {
				of_ldpc_parameters_t	*my_params;

				n = max_n;
				k = MIN(max_k, n * 0.5); /* test with code rate 1/2 */
				my_params = (of_ldpc_parameters_t *)calloc(1, sizeof(* my_params));
				if (my_params == NULL)
				{
					printf("calloc: ERROR, no memory for codec %d\n", codec_id);
					goto error;
				}
				my_params->prng_seed	= rand();
				my_params->N1		= 5;
				params = (of_parameters_t *) my_params;
				break;
				}
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC					
			case OF_CODEC_2D_PARITY_MATRIX_STABLE: {
				of_2d_parity_parameters_t	*my_params;

				/* there are very few possibilities here, so stick with what was returned */
				n = max_n;
				k = max_k;
				my_params = (of_2d_parity_parameters_t *)calloc(1, sizeof(* my_params));
				if (my_params == NULL)
				{
					printf("calloc: ERROR, no memory for codec %d\n", codec_id);
					goto error;
				}
				params = (of_parameters_t *) my_params;
				break;
				}
#endif
			default:
				printf("ERROR, unknown codec %d", codec_id);
				goto error;
			}
			params->nb_source_symbols	= k;
			params->nb_repair_symbols	= n - k;
			params->encoding_symbol_length	= SYMBOL_LEN;
			if ((ret = of_set_fec_parameters(ses, params)) != OF_STATUS_OK)
			{
				printf("ERROR, of_set_fec_parameters() failed for codec=%d \n", codec_id);
				goto error;
			}
			/*
			 * release everything to finish.
			 */
			if ((ret = of_release_codec_instance(ses)) != OF_STATUS_OK)
			{
				printf("ERROR, of_release_codec_instance() failed for codec=%d \n", codec_id);
				goto error;
			}
			free(params);
		}
	}
	printf("code_params_test: OK\n");
	return 0;
error:
	printf("code_params_test: ERROR\n");
	return -1;
}
