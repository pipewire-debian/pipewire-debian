/* $Id: of_ldpc_includes.h 72 2012-04-13 13:27:26Z detchart $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
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

#ifndef OF_LDPC_INCLUDES
#define OF_LDPC_INCLUDES

#include <stdio.h>
//#include <malloc.h>
#include <math.h>
#include <sys/time.h>	/* for timersub */

#include "../../lib_common/of_types.h"
#include "../../lib_common/of_mem.h"
#include "../../lib_common/of_debug.h"
#include "../../lib_common/of_openfec_api.h"
#include "../../lib_common/of_cb.h"

#include "of_rand.h"
#include "of_tools.h"
#include "of_hamming_weight.h"
#include "of_ldpc_staircase.h"
#include "of_matrix_sparse.h"
#include "of_matrix_dense.h"
#include "of_matrix_convert.h"
#include "of_ldpc_staircase_api.h"
#include "of_symbol.h"
#include "of_create_pchk.h"
#include "of_ml_decoding.h"
#include "of_ml_tool.h"
#include "of_ml_tool_2.h"

#endif //OF_LDPC_INCLUDES

