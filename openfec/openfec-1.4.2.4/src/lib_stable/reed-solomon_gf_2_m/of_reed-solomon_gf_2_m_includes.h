/* $Id: of_reed-solomon_gf_2_m_includes.h 189 2014-07-16 08:53:50Z roca $ */
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

#ifndef OF_REED_SOLOMON_2M_INCLUDES_H
#define OF_REED_SOLOMON_2M_INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../lib_common/of_openfec_api.h"

/* 
 * the remaining includes will only be considered if of_codec_profile.h is
 * included above by of_openfec_api.h => of_openfec_profile.h
 */
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC

#include "../../lib_common/linear_binary_codes_utils/of_linear_binary_code.h"

#include "of_reed-solomon_gf_2_m_api.h"
#include "of_reed-solomon_gf_2_m.h"
#include "galois_field_codes_utils/of_galois_field_code.h"
#include "galois_field_codes_utils/algebra_2_4.h"
#include "galois_field_codes_utils/algebra_2_8.h"


#endif //OF_USE_REED_SOLOMON_2_M_CODEC
#endif //OF_REED_SOLOMON_INCLUDES_H
