/* $Id: of_reed-solomon_gf_2_m_api.h 115 2014-04-09 14:00:27Z roca $ */
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

#ifdef OF_USE_REED_SOLOMON_2_M_CODEC

#ifndef OF_REED_SOLOMON_GF_2_M_API_H
#define OF_REED_SOLOMON_GF_2_M_API_H


/**
 * \struct of_rs_parameters_t
 * \brief Reed-Solomon stable codec specific FEC parameter structure.
 * This structure contains the pieces of information required to initialize a codec instance,
 * using the of_set_fec_parameters() function.
 */
typedef struct of_rs_2_m_parameters
{
	UINT32		nb_source_symbols;	/* must be 1st item */
	UINT32		nb_repair_symbols;	/* must be 2nd item */
	UINT32		encoding_symbol_length; /* must be 3rd item */
	/*
	* FEC codec id specific attributes follow...
	*/
	UINT16		m;			/* WARNING: was bit_size */

} of_rs_2_m_parameters_t;

#endif /* OF_REED_SOLOMON_GF_2_M_API_H */

#endif /*#ifdef OF_USE_REED_SOLOMON_CODEC_2_M */
