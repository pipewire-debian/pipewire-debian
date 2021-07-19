/* $Id: blocking_struct.h 73 2012-04-13 13:27:48Z detchart $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2012 INRIA - All rights reserved
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


/**
 * Blocking structure. Defines the number and size of each block, as
 * defined in the FLUTE document.
 */
typedef	struct of_blocking_struct {
	UINT32	nb_blocks;	/** number of source blocks into which the object
				 * is partitioned (N parameter of the FLUTE doc). */
	UINT32	I;		/** number of source blocks of length A_large symbols
				 * (pointer). These blocks are the first I blocks.
				 * The remaining N-I blocks are of size A_small. */
	UINT32	A_large;	/** length in source symbols of the first I large
				 * source blocks. */
	UINT32	A_small;	/** length in source symbols of the remaining N-I
				 * smaller blocks. */
} of_blocking_struct_t;


/**
 * Compute source block structure. Follows closely the algorithm specified
 * in FLUTE and similar specifications.
 *
 * @param B	(IN) maximum number of source symbols per source block.
 * @param L	(IN) transfer length (i.e. object size) in bytes.
 * @param E	(IN) encoding symbol length in bytes.
 * @param bs	(OUT) blocking structure, containing the N, I, A_large
 *		and A_small values. This structure must be allocated/freed
 *		by the caller.
 */
void	of_compute_blocking_struct (UINT32			B,
				    UINT32			L,
				    UINT32			E,
				    of_blocking_struct_t	*bs);


