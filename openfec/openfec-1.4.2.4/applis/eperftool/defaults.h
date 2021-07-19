/* $Id: defaults.h 99 2013-11-07 02:20:38Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2010 INRIA - All rights reserved
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

/*
 * Default parameter values.
 */

/** Default total number of source symbols for the object.  It is equal to k when there
 *  is a single source block for the whole object. */
#define DFLT_TOT_NB_SOURCE	2000

/** Default total number of repair symbols for the object.  It is equal to n-k when there
 *  is a single source block for the whole object. */
#define DFLT_TOT_NB_REPAIR	1000

/** Default symbol size, in bytes. */
#define DFLT_SYMBOL_SZ		1024

/** Default verbosity level. 0 means no trace, > 0 means traces. */
#define DFLT_VERBOSITY		0


/*
 * Codec specific default values.
 */

/** Target M parameter for RS codes over GF(2^M) (See RFC 5510). */
#define DFLT_RS_M_PARAM		8	

/** Default LDPC N1 parameter (see RFC 5170). */
#define DFLT_LDPC_N1		5

/** Default code rate for ldpc in case gldpc code  */
#define	DFLT_CODE_RATE_LDPC_STAIRCASE_FOR_GLDPC 0.5

/** Reed-Solomon over GF(2^8) constant. */
#define MAX_N_FOR_RS_GF_2_8	255

/* Reed-Solomon over GF(2^16) constant. */
#define MAX_N_FOR_RS_GF_2_16	65535


/*
 * If defined, then check if rebuilt packets match the original ones.
 * With validated codecs, this constant can be left undefined to improve the simulation speed.
 */
#define CHECK_INTEGRITY 1


/*
 * number of (simulated) losses in % when the previous packet was OK (default value)
 */
#define P_LOSS_WHEN_OK	1

/*
 * number of succesful tx in % when the previous packet was LOST (default value)
 */
#define P_SUCCESS_WHEN_LOSSES	25

