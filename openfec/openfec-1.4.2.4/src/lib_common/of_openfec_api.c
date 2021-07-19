/* $Id: of_openfec_api.c 214 2014-12-12 23:14:02Z roca $ */
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
#include "of_openfec_api.h"

#ifdef OF_USE_REED_SOLOMON_CODEC
#include "../lib_stable/reed-solomon_gf_2_8/of_reed-solomon_gf_2_8_includes.h"
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
#include "../lib_stable/reed-solomon_gf_2_m/of_reed-solomon_gf_2_m_includes.h"
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
#include "../lib_stable/ldpc_staircase/of_ldpc_includes.h"
#endif	
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
#include "../lib_stable/2d_parity_matrix/of_2d_parity_includes.h"
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
#include "../lib_advanced/ldpc_from_file/of_ldpc_ff_includes.h"
#endif

UINT32	of_verbosity;


of_status_t	of_create_codec_instance (of_session_t**	ses,
					  of_codec_id_t		codec_id, 
					  of_codec_type_t	codec_type, 
					  UINT32		verbosity)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	of_verbosity = verbosity;
	/**
	 * each codec must realloc control block.
	 */
	(*ses) = of_calloc (1, sizeof (of_cb_t));
	if (*ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, of_calloc failed\n"))
		goto error2;
	}
	((of_cb_t*) (*ses))->codec_type	= codec_type;
	((of_cb_t*) (*ses))->codec_id	= codec_id;
	
	switch (codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status =  of_rs_create_codec_instance ( (of_rs_cb_t**) ses);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status =  of_rs_2_m_create_codec_instance ( (of_rs_2_m_cb_t**) ses);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC	
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_create_codec_instance ( (of_ldpc_staircase_cb_t**) ses);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC	
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_create_codec_instance ( (of_2d_parity_cb_t**) ses);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_ff_create_codec_instance ( (of_ldpc_ff_cb_t**) ses);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	of_free(*ses);
	*ses = NULL;
error2:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_release_codec_instance (of_session_t*	ses)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses != NULL)
	{
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC			
	case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
		status = of_rs_release_codec_instance ( (of_rs_cb_t*) ses);
		break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC			
	case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
		status = of_rs_2_m_release_codec_instance ( (of_rs_2_m_cb_t*) ses);
		break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC		
	case OF_CODEC_LDPC_STAIRCASE_STABLE:
		status = of_ldpc_staircase_release_codec_instance ( (of_ldpc_staircase_cb_t*) ses);
		break;		
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
	case OF_CODEC_2D_PARITY_MATRIX_STABLE:
		status = of_2d_parity_release_codec_instance( (of_2d_parity_cb_t*) ses);
		break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC	
	case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
		status = of_ldpc_staircase_release_codec_instance ( (of_ldpc_staircase_cb_t*) ses);
		break;		
#endif
	default:
		OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
		goto error;
	}
	of_free(ses);
	ses = NULL;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_set_fec_parameters  (of_session_t* ses,	of_parameters_t*	params)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if ((ses == NULL) || (params == NULL))
	{
		OF_PRINT_ERROR ( ("Error, bad ses or params pointer (null)\n"))
		goto error;
	}
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
	if (( (of_cb_t*) ses)->codec_id != OF_CODEC_LDPC_FROM_FILE_ADVANCED &&
	    ((params->nb_source_symbols <= 0) || (params->nb_repair_symbols <= 0)) ||
	    (params->encoding_symbol_length <= 0))
	{
		OF_PRINT_ERROR ( ("Error, bad parameters:"))
		if(params->nb_source_symbols <= 0)
		{
			OF_PRINT_ERROR ( ("nb_source_symbols %d <= 0\n", params->nb_source_symbols))
		}
		if(params->nb_repair_symbols <= 0)
		{
			OF_PRINT_ERROR ( ("nb_repair_symbols %d <= 0\n", params->nb_repair_symbols))
		}
		if(params->encoding_symbol_length <= 0)
		{
			OF_PRINT_ERROR ( ("encoding_symbol_length %d <= 0\n", params->encoding_symbol_length))
		}
		goto error;
	}
#endif	
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_set_fec_parameters ( (of_rs_cb_t*) ses, (of_rs_parameters_t*) params);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_set_fec_parameters ( (of_rs_2_m_cb_t*) ses, (of_rs_2_m_parameters_t*) params);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC	
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_set_fec_parameters ( (of_ldpc_staircase_cb_t*) ses, (of_ldpc_parameters_t*) params);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC		
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_set_fec_parameters( (of_2d_parity_cb_t*) ses, (of_2d_parity_parameters_t*) params);
			break;
#endif	
#ifdef OF_USE_LDPC_FROM_FILE_CODEC		
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_ff_set_fec_parameters ((of_ldpc_ff_cb_t*) ses, (of_ldpc_ff_parameters_t*) params);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_set_callback_functions (of_session_t	*ses,
					void* (*decoded_source_symbol_callback) (void	*context,
										UINT32	size,
										UINT32	esi),
					void* (*decoded_repair_symbol_callback) (void	*context,
										UINT32	size,
										UINT32	esi),
					void*	context_4_callback)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if ((decoded_source_symbol_callback == NULL) && (decoded_repair_symbol_callback == NULL))
	{
		OF_PRINT_ERROR ( ("decoded_source_symbol_callback and decoded_repair_symbol_callback or both NULL\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_set_callback_functions((of_rs_cb_t*) ses,
							decoded_source_symbol_callback,
							decoded_repair_symbol_callback,
							context_4_callback);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_set_callback_functions((of_rs_2_m_cb_t*) ses,
							decoded_source_symbol_callback,
							decoded_repair_symbol_callback,
							context_4_callback);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_set_callback_functions((of_ldpc_staircase_cb_t*) ses,
							decoded_source_symbol_callback,
							decoded_repair_symbol_callback,
							context_4_callback);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_set_callback_functions( (of_2d_parity_cb_t*) ses,
							decoded_source_symbol_callback,
							decoded_repair_symbol_callback,
							context_4_callback);
			break;
#endif	
#ifdef OF_USE_LDPC_FROM_FILE_CODEC	
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_staircase_set_callback_functions((of_ldpc_staircase_cb_t*) ses,
							decoded_source_symbol_callback,
							decoded_repair_symbol_callback,
							context_4_callback);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_more_about (of_session_t*	ses,
				char**		version_str,
				char**		copyrights_str)
{
	OF_ENTER_FUNCTION
	static char	of_version_string[] = "OpenFEC.org - Version 1.4.2, December 16th, 2014\n";
	static char	of_copyrights_string[] ="\n\
    OpenFEC.org - Because free, open source AL-FEC codes and codecs matter\n\
    Copyright (c) 2003-2014 INRIA - All rights reserved\n\
	\n\
    LDPC codecs:\n\
	Copyright (c) 2003-2014 INRIA - All rights reserved\n\
	This library contains code from R. Neal:\n\
	Copyright (c) 1995-2003 by Radford M. Neal\n\
	\n\
    Reed-Solomon codecs:\n\
	This library contains code from L. Rizzo:\n\
	Copyright (c) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)\n\
	Portions derived from code by Phil Karn (karn@ka9q.ampr.org), \n\
	Robert Morelos-Zaragoza (robert@spectra.eng.hawaii.edu) and Hari \n\
	Thirumoorthy (harit@spectra.eng.hawaii.edu), Aug 1995 \n\
	\n\
    See the associated LICENCE.TXT file for licence information\n";
	
	if (version_str)
	{
		*version_str = of_version_string;
	}
	if (copyrights_str)
	{
		*copyrights_str = of_copyrights_string;
	}
	OF_EXIT_FUNCTION
	return OF_STATUS_OK;
}


#ifdef OF_USE_ENCODER

of_status_t	of_build_repair_symbol (of_session_t*	ses, void*	encoding_symbols_tab[],UINT32	esi_of_symbol_to_build)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if (!(((of_cb_t*) ses)->codec_type & OF_ENCODER))
	{
		OF_PRINT_ERROR ( ("Error, bad codec_type\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC		
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_build_repair_symbol ((of_rs_cb_t*) ses, encoding_symbols_tab, esi_of_symbol_to_build);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC		
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_build_repair_symbol ((of_rs_2_m_cb_t*) ses, encoding_symbols_tab, esi_of_symbol_to_build);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_build_repair_symbol ((of_ldpc_staircase_cb_t*) ses, encoding_symbols_tab, esi_of_symbol_to_build);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_build_repair_symbol( (of_2d_parity_cb_t*) ses, encoding_symbols_tab, esi_of_symbol_to_build);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_ff_build_repair_symbol ((of_ldpc_ff_cb_t*) ses, encoding_symbols_tab, esi_of_symbol_to_build);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}

#endif //OF_USE_ENCODER


#ifdef OF_USE_DECODER

of_status_t	of_decode_with_new_symbol (of_session_t*	ses,
									   void* const	new_symbol_buf,
									   UINT32		new_symbol_esi)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if ( new_symbol_esi >= (((of_cb_t*) ses)->nb_source_symbols + ((of_cb_t*) ses)->nb_repair_symbols) )
	{
		OF_PRINT_ERROR ( ("Error, bad parameters new_symbol_esi(%d) out of range\n",new_symbol_esi))
		goto error;
	}
	if (new_symbol_buf == NULL ||
	    new_symbol_esi >= (((of_cb_t*) ses)->nb_source_symbols + ((of_cb_t*) ses)->nb_repair_symbols) ||
	    !(( (of_cb_t*) ses)->codec_type & OF_DECODER))
	{
		OF_PRINT_ERROR ( ("Error, bad parameters\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_decode_with_new_symbol ( (of_rs_cb_t*) ses, new_symbol_buf, new_symbol_esi);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_decode_with_new_symbol ( (of_rs_2_m_cb_t*) ses, new_symbol_buf, new_symbol_esi);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_decode_with_new_symbol ( (of_ldpc_staircase_cb_t*) ses, new_symbol_buf, new_symbol_esi);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_decode_with_new_symbol( (of_2d_parity_cb_t*) ses, new_symbol_buf, new_symbol_esi);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_staircase_decode_with_new_symbol ( (of_ldpc_staircase_cb_t*) ses, new_symbol_buf, new_symbol_esi);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_set_available_symbols (of_session_t*	ses,
									  void* const	encoding_symbols_tab[])
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if (encoding_symbols_tab == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad encoding_symbols_tab (null)\n"))
		goto error;
	}
	if (!((of_cb_t*) ses)->codec_type & OF_DECODER)
	{
		OF_PRINT_ERROR ( ("Error, bad codec_type\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_set_available_symbols ( (of_rs_cb_t*) ses, encoding_symbols_tab);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_set_available_symbols ( (of_rs_2_m_cb_t*) ses, encoding_symbols_tab);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_set_available_symbols ( (of_ldpc_staircase_cb_t*) ses, encoding_symbols_tab);
			break;
#endif		
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_staircase_set_available_symbols ( (of_ldpc_staircase_cb_t*) ses, encoding_symbols_tab);
			break;
#endif		
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_set_available_symbols ( (of_2d_parity_cb_t*) ses, encoding_symbols_tab);
			break;
#endif	
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_finish_decoding (of_session_t*	ses)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if (!((of_cb_t*) ses)->codec_type & OF_DECODER)
	{
		OF_PRINT_ERROR ( ("Error, bad codec_type\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC		
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status =  of_rs_finish_decoding ( (of_rs_cb_t*) ses);
			break;
#endif	
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC		
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status =  of_rs_2_m_finish_decoding ( (of_rs_2_m_cb_t*) ses);
			break;
#endif	
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status =  of_ldpc_staircase_finish_decoding ( (of_ldpc_staircase_cb_t*) ses);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_finish_decoding ( (of_2d_parity_cb_t*) ses);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status =  of_ldpc_staircase_finish_decoding ( (of_ldpc_staircase_cb_t*) ses);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


bool of_is_decoding_complete (of_session_t*	ses)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if (!((of_cb_t*) ses)->codec_type & OF_DECODER)
	{
		OF_PRINT_ERROR ( ("Error, bad codec_type\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_is_decoding_complete ( (of_rs_cb_t*) ses);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_is_decoding_complete ( (of_rs_2_m_cb_t*) ses);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_is_decoding_complete ( (of_ldpc_staircase_cb_t*) ses);
			break;	
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_staircase_is_decoding_complete ( (of_ldpc_staircase_cb_t*) ses);
			break;	
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_is_decoding_complete ( (of_2d_parity_cb_t*) ses);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	/* should be a fatal error since we arrive here because something wrong happen,
	 but this is not permitted */
	return false;
}


of_status_t	of_get_source_symbols_tab (of_session_t*	ses,
									   void*		source_symbols_tab[])
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	if (!((of_cb_t*) ses)->codec_type & OF_DECODER)
	{
		OF_PRINT_ERROR ( ("Error, bad codec_type\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_get_source_symbols_tab ( (of_rs_cb_t*) ses, source_symbols_tab);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_get_source_symbols_tab ( (of_rs_2_m_cb_t*) ses, source_symbols_tab);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC	
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_get_source_symbols_tab ( (of_ldpc_staircase_cb_t*) ses, source_symbols_tab);
			break;	
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC		
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_get_source_symbols_tab ( (of_2d_parity_cb_t*) ses, source_symbols_tab);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC		
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_staircase_get_source_symbols_tab ( (of_ldpc_staircase_cb_t*) ses, source_symbols_tab);
			break;	
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_set_control_parameter (of_session_t*	ses,
					  UINT32	type,
					  void*		value,
					  UINT32	length)
{
	of_status_t status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			//status = of_rs_set_control_parameter ( (of_rs_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_set_control_parameter ( (of_rs_2_m_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC	
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_set_control_parameter ( (of_ldpc_staircase_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_set_control_parameter ( (of_2d_parity_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_ff_set_control_parameter ( (of_ldpc_ff_cb_t*) ses, type, value, length);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}


of_status_t	of_get_control_parameter (of_session_t*	ses, UINT32	type, void* value, UINT32	length)
{
	of_status_t	status;
	
	OF_ENTER_FUNCTION
	if (ses == NULL)
	{
		OF_PRINT_ERROR ( ("Error, bad ses pointer (null)\n"))
		goto error;
	}
	switch ( ( (of_cb_t*) ses)->codec_id)
	{
#ifdef OF_USE_REED_SOLOMON_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_8_STABLE:
			status = of_rs_get_control_parameter ( (of_rs_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
		case OF_CODEC_REED_SOLOMON_GF_2_M_STABLE:
			status = of_rs_2_m_get_control_parameter ( (of_rs_2_m_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
		case OF_CODEC_LDPC_STAIRCASE_STABLE:
			status = of_ldpc_staircase_get_control_parameter ( (of_ldpc_staircase_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC		
		case OF_CODEC_LDPC_FROM_FILE_ADVANCED:
			status = of_ldpc_ff_get_control_parameter ( (of_ldpc_ff_cb_t*) ses, type, value, length);
			break;
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
		case OF_CODEC_2D_PARITY_MATRIX_STABLE:
			status = of_2d_parity_get_control_parameter ( (of_2d_parity_cb_t*) ses, type, value, length);
			break;
#endif
		default:
			OF_PRINT_ERROR ( ("Error, codec %d non available\n", ((of_cb_t*)ses)->codec_id))
			goto error;
	}
	OF_EXIT_FUNCTION
	return status;
	
error:
	OF_EXIT_FUNCTION
	return OF_STATUS_FATAL_ERROR;
}

#endif //OF_USE_DECODER
