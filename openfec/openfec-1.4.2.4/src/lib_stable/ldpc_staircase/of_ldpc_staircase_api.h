/* $Id: of_ldpc_staircase_api.h 72 2012-04-13 13:27:26Z detchart $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL license under French law and
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

#ifdef OF_USE_LDPC_STAIRCASE_CODEC

#ifndef OF_CODEC_STABLE_LDPC_STAIRCASE_API
#define OF_CODEC_STABLE_LDPC_STAIRCASE_API


#include "../../lib_common/of_types.h"

/**
 * @struct of_ldpc_parameters_t
 * @brief LDPC-Staircase stable codec specific FEC parameter structure.
 *
 * This structure contains the pieces of information required to initialize a codec instance,
 * using the of_set_fec_parameters() function.
 */
typedef struct of_ldpc_parameters
{
	UINT32		nb_source_symbols;	/* must be 1st item */
	UINT32		nb_repair_symbols;	/* must be 2nd item */
	UINT32		encoding_symbol_length; /* must be 3rd item */
	/*
	* FEC codec id specific attributes follow...
	*/
	INT32		prng_seed;
	UINT8		N1;
} of_ldpc_parameters_t;


/**
 * Control parameters for of_set_control_parameter()/of_get_control_parameter() functions:
 *   - range {0 .. 1023} inclusive are for generic parameters;
 *   - range {1024 .. above} inclusive are for code/codec specific parameters (and different
 *		codecs can re-use the same value);
 * The "void * value" type depends on the type.
 */

/**
 * Ask the OF library if the last repair symbol of an LDPC-Staircase code is equal to zero
 * or not. This is used for various optimizations (e.g. it's useless to transmit a symbol
 * that is known to be null). This call returns 1 if the property is true (i.e. the last
 * repair symbol is actually NULL), 0 otherwise.
 * Argument: bool
 */
#define	OF_CRTL_LDPC_STAIRCASE_IS_LAST_SYMBOL_NULL	1024


#endif  /* OF_CODEC_STABLE_LDPC_SCSTAIRCASE_API */

#endif /*#ifdef OF_USE_LDPC_STAIRCASE_CODEC */
