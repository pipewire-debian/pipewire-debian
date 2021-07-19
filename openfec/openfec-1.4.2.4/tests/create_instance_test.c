/* $Id: create_instance_test.c 530 2010-10-25 14:43:42Z roca $ */
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

#define OF_USE_ENCODER
#define OF_USE_DECODER
#include <stdio.h>

#include "../src/lib_common/of_openfec_api.h"
#include "../src/lib_common/of_debug.h"

#define MAX_CODEC_ID 11

int main()
{
	of_codec_id_t	codec_id;
	of_session_t	*ses;
	of_status_t		ret;

	for (codec_id = 1; codec_id <= MAX_CODEC_ID; codec_id++)
	{
		switch ((int)codec_id)
		{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE: {
			printf("(OF_CODEC_REED_SOLOMON_GF_2_8_STABLE):\n\t");
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
#ifdef OF_USE_LDPC_STAIRCASE_ADVANCED_CODEC
		case OF_CODEC_LDPC_STAIRCASE_ADVANCED: {
			printf("(OF_CODEC_LDPC_STAIRCASE_ADVANCED):\n\t");
			break;
			}
#endif
		default:
			printf("unknown codec:\n\tIgnored...\n");
			continue;
		}
		if ((ret = of_create_codec_instance(&ses, codec_id, OF_ENCODER, 1)) != OF_STATUS_OK)
		{
			printf("ERROR: of_create_codec_instance()");
			//goto error;
		}
		if ((ret = of_release_codec_instance(ses)) != OF_STATUS_OK)
		{
			printf("ERROR: of_release_codec_instance()");
			//goto error;
		}
	}
	printf("OK\n");
	return 0;

error:
	printf("error\n");
	return -1;
}
