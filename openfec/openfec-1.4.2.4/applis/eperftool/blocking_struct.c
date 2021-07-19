/* $Id: blocking_struct.c 148 2014-07-08 08:01:56Z roca $ */
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

#include "eperftool.h"


static UINT32	double_to_closest_int (double value);


/**
 * Compute source block structure.
 * => See header file for more informations.
 */
void
of_compute_blocking_struct (UINT32			B,
			     UINT32			L,
			     UINT32			E,
			     of_blocking_struct_t	*bs)
{
	UINT32	T;	/* number of source symbols in the object */
	double	A;	/* average length of a source block in number of symbols */
	double	A_fraction;

	T		= (UINT32)ceil((double)L / (double)E);
	bs->nb_blocks	= (INT32)ceil((double)T / (double)B);
	A		= (double)T/(double)bs->nb_blocks; /* average block size; non integer */
	bs->A_large	= (UINT32)ceil((double)A);
	bs->A_small	= (UINT32)floor((double)A);
	A_fraction	= A - bs->A_small;
	bs->I		= double_to_closest_int(A_fraction * (double)bs->nb_blocks);
#if 1
	printf("of_compute_blocking_struct: B=%d, L=%d, E=%d\n\t\tnb_blocks=%d, I=%d, A_large=%d, A_small=%d\n",
		B, L, E, bs->nb_blocks, bs->I, bs->A_large, bs->A_small);
#endif
	ASSERT(bs->A_large <= B);
}


/**
 * This function converts double value to the closest integer value.
 * Usefull if the double value can have small calculation errors.
 * @params value	value to be converted.
 * @return		converted integer value.
 */
static UINT32
double_to_closest_int (double	value)
{
	double ceil_value;
	double floor_value;
	double abs_diff1;
	double abs_diff2;

	ceil_value	= ceil(value);
	floor_value	= floor(value);
	abs_diff1	= fabs(value - ceil_value);
	abs_diff2	= fabs(value - floor_value);
	if (abs_diff1 < abs_diff2) {
		return ((UINT32)ceil_value);
	} else {
		return ((UINT32)floor_value);
	}
}

