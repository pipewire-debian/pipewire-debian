/* $Id: of_statistics.c 207 2014-12-10 19:47:50Z roca $ */
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

#include <stdio.h>
#include "../of_types.h"
#include "../of_debug.h"
#include "../of_openfec_api.h"
#include "of_statistics.h"


#ifdef OF_DEBUG

 
void of_print_symbols_stats(of_symbols_stats_t* stats)
{
	OF_TRACE_LVL(0,("nb source symbols received: %u\n", stats->nb_source_symbols_received))
	OF_TRACE_LVL(0,("nb repair symbols received: %u\n", stats->nb_repair_symbols_received));
	OF_TRACE_LVL(0,("nb source symbols built during IT decoding: %i\n", stats->nb_source_symbols_built_with_it));
	OF_TRACE_LVL(0,("nb repair symbols built during IT decoding: %i\n", stats->nb_repair_symbols_built_with_it));
	OF_TRACE_LVL(0,("nb source symbols built during ML decoding: %i\n", stats->nb_source_symbols_built_with_ml));
	OF_TRACE_LVL(0,("nb repair symbols built during ML decoding: %i\n", stats->nb_repair_symbols_built_with_ml));
	OF_TRACE_LVL(0,("nb source symbols ignored: %i\n", stats->nb_source_symbols_ignored));
	OF_TRACE_LVL(0,("nb repair symbols ignored: %i\n", stats->nb_repair_symbols_ignored));
}
 
#endif
