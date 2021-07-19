/* $Id: of_it_decoding.h 186 2014-07-16 07:17:53Z roca $ */
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

#ifndef IT_DECODING_H
#define IT_DECODING_H


#ifdef OF_USE_DECODER
#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS


/**
 * @fn	  of_status_t	of_linear_binary_code_decode_with_new_symbol (of_linear_binary_code_cb_t*	ofcb, void*	const	new_symbol_buf, UINT32		new_symbol_esi)
 * @brief (try to) decode with a newly received symbol
 * @param ofcb			(IN) Pointer to the linear binary code control block.
 * @param new_symbol		(IN) Pointer to the encoding symbol now available (i.e. a new
 *				symbol received by the application, or a decoded symbol in case
 *				of a recursive call).
 * @param new_symbol_esi	(IN) Encoding symbol ID of the newly symbol available, in {0..n-1}.
 * @return			Error status (NB: this function does not return OF_STATUS_FAILURE).
 */
of_status_t of_linear_binary_code_decode_with_new_symbol(of_linear_binary_code_cb_t* ofcb,void* new_symbol,UINT32 new_symbol_esi);

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif //OF_USE_DECODER

#endif //IT_DECODING_H
