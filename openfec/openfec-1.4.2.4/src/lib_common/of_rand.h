/* $Id: of_rand.h 186 2014-07-16 07:17:53Z roca $ */
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

#ifndef OF_RAND
#define OF_RAND


/**
 * \fn void		of_rfc5170_srand (UINT64 s)
 * \brief Initialize the PRNG with a of_seed between 1 and 0x7FFFFFFE
 * (2^^31-2) inclusive.
 * \param s	(IN) seed
 * \return 	void
 */
void		of_rfc5170_srand (UINT64 s);


/**
 * \fn UINT64	of_rfc5170_rand (UINT64	maxv)
 * \brief Derived from rand31pmc, Robin Whittle, Sept 20th 2005.
 * http://www.firstpr.com.au/dsp/rand31/
 *	16807		multiplier constant (7^^5)
 *	0x7FFFFFFF	modulo constant (2^^31-1)
 * The inner PRNG produces a value between 1 and 0x7FFFFFFE
 * (2^^31-2) inclusive.
 * This value is then scaled between 0 and maxv-1 inclusive.
 * This is the PRNG required by the LDPC-staircase RFC 5170.
 * \return Returns a random integer between 0 and maxv-1 inclusive.
 */
UINT64	of_rfc5170_rand (UINT64	maxv);

#endif //OF_RAND
