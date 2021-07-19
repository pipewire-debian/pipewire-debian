/* $Id: of_statistics.h 186 2014-07-16 07:17:53Z roca $ */
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

#ifndef STATISTICS_H
#define STATISTICS_H


#ifdef OF_DEBUG


/**
 * Symbol level statistics.
 */
typedef struct of_symbols_stats
{
	/** Number of source symbols received by decoder (they havn't been decoded). */
	UINT32		nb_source_symbols_received;
	/** Number of repair symbols received by decoder (they havn't been decoded). */
	UINT32		nb_repair_symbols_received;
	/** Number of source symbols decoded during IT decoding. */
	UINT32		nb_source_symbols_built_with_it; 
	/** Number of source symbols decoded during ML decoding. */
	UINT32		nb_source_symbols_built_with_ml;
	/** Number of repair symbols decoded during IT decoding. */
	UINT32		nb_repair_symbols_built_with_it;
	/** Number of repair symbols decoded during ML decoding. */
	UINT32		nb_repair_symbols_built_with_ml;
	/** Number of source symbols ...... */
	UINT32		nb_source_symbols_ignored;
	/** Number of repair symbols ...... */
	UINT32		nb_repair_symbols_ignored;
} of_symbols_stats_t;


/**
 * Print the statistics to the standard output.
 */
void	of_print_symbols_stats(of_symbols_stats_t*);


#endif // OF_DEBUG

#endif //STATISTICS_H
