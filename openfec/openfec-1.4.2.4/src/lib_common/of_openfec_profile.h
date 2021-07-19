/* $Id: of_openfec_profile.h 207 2014-12-10 19:47:50Z roca $ */
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

#ifndef OF_OPENFEC_PROFILE_H
#define OF_OPENFEC_PROFILE_H

#define OF_USE_ENCODER
#define OF_USE_DECODER

/*
 * Edit as needed to define which codec(s) you need.
 * By default all codecs are considered.
 *
 * Commenting an include entry below prevents the compiler to read
 * the associated of_codec_profile.h file where the USE_*_
 * macro is defined. Therefore the associated codec will be ignored
 * during compilation.
 *
 * Removing codecs known to be useless can be important for highly
 * specialized usage of OpenFEC, like embedded systems.
 */
#include "../lib_stable/reed-solomon_gf_2_8/of_codec_profile.h"
#include "../lib_stable/reed-solomon_gf_2_m/of_codec_profile.h"
#include "../lib_stable/ldpc_staircase/of_codec_profile.h"
#include "../lib_stable/2d_parity_matrix/of_codec_profile.h"
//#include "../lib_advanced/ldpc_from_file/of_codec_profile.h"

/*
 * Edit as needed to define which core solving system to use.
 * By default all systems are considered.
 *
 * Note that the above codecs require certain solving systems.
 * For instance, LDPC-staircase requires IT decoding, and ML
 * decoding is highly recommended for improved performances.
 *
 * Removing solving systems known to be useless can be important
 * for highly specialized usage of OpenFEC, like embedded systems.
 */
#define OF_USE_LINEAR_BINARY_CODES_UTILS
#define OF_USE_GALOIS_FIELD_CODES_UTILS

/**
 * Define if you need SSE optimizations for XOR operations.
 * This is useful for PC usage, with processors that support this
 * extension (i.e. all the processors except the very old ones).
 */
//#define ASSEMBLY_SSE_OPT
 
/*
 * NB: if SSE is not defined, then we'll use regular XOR operations,
 * either on 32 bit or 64 bit integers depending on the operating
 * system.
 */

#endif // OF_OPENFEC_PROFILE_H
