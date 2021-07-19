/* $Id: of_matrix_convert.c 186 2014-07-16 07:17:53Z roca $ */
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

#include "../of_linear_binary_code.h"


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

/* CONVERT A MOD2 MATRIX FROM SPARSE TO DENSE FORM.  */

void	of_mod2sparse_to_dense (of_mod2sparse *m, 	/* Sparse matrix to convert */
				of_mod2dense *r)	/* Place to store result */
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	UINT32 i;

	if (of_mod2sparse_rows (m) > of_mod2dense_rows (r)
			|| of_mod2sparse_cols (m) > of_mod2dense_cols (r))
	{
		OF_PRINT_ERROR(("mod2sparse_to_dense: Dimension of result matrix is less than source\n"))
		OF_EXIT_FUNCTION
		return;
	}

	of_mod2dense_clear (r);

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);
		while (!of_mod2sparse_at_end (e))
		{
			of_mod2dense_set (r, i, of_mod2sparse_col (e), 1);
			e = of_mod2sparse_next_in_row (e);
		}
	}
}


/* CONVERT A MOD2 MATRIX FROM DENSE TO SPARSE FORM.  */

void	of_mod2dense_to_sparse (of_mod2dense *m, 	/* Dense matrix to convert */
				of_mod2sparse *r)	/* Place to store result */
{
	UINT32 i, j;

	if (of_mod2dense_rows (m) > of_mod2sparse_rows (r)
			|| of_mod2dense_cols (m) > of_mod2sparse_cols (r))
	{
		OF_PRINT_ERROR(("mod2dense_to_sparse: Dimension of result matrix is less than source\n"))
		OF_EXIT_FUNCTION
		return;
	}
	of_mod2sparse_clear (r);
	for (i = 0; i < of_mod2dense_rows (m); i++)
	{
		for (j = 0; j < of_mod2dense_cols (m); j++)
		{
			if (of_mod2dense_get (m, i, j))
			{
				of_mod2sparse_insert (r, i, j);
			}
		}
	}
}

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
