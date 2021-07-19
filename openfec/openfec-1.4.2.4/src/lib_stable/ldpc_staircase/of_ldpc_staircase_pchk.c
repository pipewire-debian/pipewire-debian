/* $Id: of_ldpc_staircase_pchk.c 184 2014-07-15 09:42:57Z roca $ */
/*
 * The contents of this directory and its sub-directories are
 * Copyright (c) 1995-2003 by Radford M. Neal
 *
 * Permission is granted for anyone to copy, use, modify, or distribute these
 * programs and accompanying documents for any purpose, provided this copyright
 * notice is retained and prominently displayed, along with a note saying
 * that the original programs are available from Radford Neal's web page, and
 * note is made of any changes made to these programs.  These programs and
 * documents are distributed without any warranty, express or implied.  As the
 * programs were written for research purposes only, they have not been tested
 * to the degree that would be advisable in any important application.  All use
 * of these programs is entirely at the user's own risk.
 */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2011 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL license under French law and
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

#include "of_ldpc_includes.h"
#ifdef OF_USE_LDPC_STAIRCASE_CODEC


/* RFC5170 compliant fonction. DO NOT MODIFY */
of_mod2sparse* of_create_pchck_matrix_rfc5170_compliant (UINT32		nb_rows,
							 UINT32		nb_cols,
							 UINT32		left_degree,
							 UINT32		seed,
							 of_ldpc_staircase_cb_t	*ofcb)
{
	OF_ENTER_FUNCTION
	of_mod2entry	*e;
	UINT32		added, uneven;
	INT32		i, j, k;
	INT32		t;			/* left limit within the list of possible choices u[] */
	UINT32		*u;			/* table used to have a homogeneous 1 distrib. */
	of_mod2sparse	*pchkMatrix = NULL;
	UINT32		skipCols = 0;		// avoid warning
	UINT32		nbDataCols = 0;		// avoid warning

	skipCols = nb_rows;
	nbDataCols = nb_cols - skipCols;
	/* a few sanity checks... */
	if (left_degree > nb_rows)
	{
		OF_PRINT_ERROR(("number of 1s per column (i.e. N1=%d parameter) is greater than total number of rows (i.e. n-k=%d)\n",
				left_degree, nb_rows));
		OF_EXIT_FUNCTION
		return NULL;
	}
	of_rfc5170_srand (seed);
	pchkMatrix = of_mod2sparse_allocate (nb_rows, nb_cols);
	/* create the initial version of the parity check matrix. */
	/* evenboth make method only */
	u = (UINT32*) of_calloc (left_degree * nbDataCols, sizeof * u);
	/* initialize a list of possible choices to guarantee a homogeneous "1" distribution */
	for (k = left_degree * nbDataCols - 1; k >= 0; k--)
	{
		u[k] = k % nb_rows;
	}
	uneven = 0;
	t = 0;	/* left limit within the list of possible choices, u[] */
	for (j = skipCols; j < nb_cols; j++)
	{
		/* for each source symbol column */
		for (k = 0; k < left_degree; k++)
		{
			/* add left_degree "1s" */
			/* check that valid available choices remain */
			for (i = t; i < left_degree * nbDataCols && of_mod2sparse_find (pchkMatrix, u[i], j); i++)
				;

			if (i < left_degree * nbDataCols)
			{
				/* choose one index within the list of possible choices */
				do
				{
					i = t + of_rfc5170_rand (left_degree * nbDataCols - t);
				}
				while (of_mod2sparse_find (pchkMatrix, u[i], j));
				of_mod2sparse_insert (pchkMatrix, u[i], j);
				/* replace with u[t] which has never been chosen */
				u[i] = u[t];
				t++;
			}
			else
			{
				/* no choice left, choose one randomly.
				 * This happens if we're not lucky and if in the remaining possible
				 * choices, for instance for the last source symbol, the same row
				 * appears several times. */
				uneven += 1;
				do
				{
					i = of_rfc5170_rand (nb_rows);
				}
				while (of_mod2sparse_find (pchkMatrix, i, j));
				of_mod2sparse_insert (pchkMatrix, i, j);
			}
		}
	}
	if (uneven > 0 && of_verbosity >= 1)
	{
		OF_PRINT_LVL(1, ("%s: Had to place %d checks in rows unevenly\n", __FUNCTION__, uneven))
	}
	of_free (u);	/* VR: added */
	/* Add extra bits to avoid rows with less than two checks. */
	added = 0;
	for (i = 0; i < nb_rows; i++)
	{
		e = of_mod2sparse_first_in_row (pchkMatrix, i);
		if (of_mod2sparse_at_end (e))
		{
			j = (of_rfc5170_rand (nbDataCols)) + skipCols;
			e = of_mod2sparse_insert (pchkMatrix, i, j);
			added ++;
		}
		e = of_mod2sparse_first_in_row (pchkMatrix, i);
		if (of_mod2sparse_at_end (of_mod2sparse_next_in_row (e)) && nbDataCols > 1)
		{
			do
			{
				j = (of_rfc5170_rand (nbDataCols)) + skipCols;
			}
			while (j == of_mod2sparse_col (e));
			of_mod2sparse_insert (pchkMatrix, i, j);
			added ++;
		}
	}
	if (added >= 1)
	{
		ofcb->extra_entries_added_in_pchk = 1;
		OF_TRACE_LVL(1,("%s: Added %d extra bit-checks to make row Hamming weight at least two\n", __FUNCTION__, added));
	}
	else
	{
		ofcb->extra_entries_added_in_pchk = 0;	/* nothing added, there are exactly N1 entries per column. */
	}
	/* finally, create the staircase */
	of_mod2sparse_insert (pchkMatrix, 0, 0);	/* 1st row */
	for (i = 1; i < nb_rows; i++)
	{
		/* for all other rows */
		/* identity */
		of_mod2sparse_insert (pchkMatrix, i, i);
		/* staircase */
		of_mod2sparse_insert (pchkMatrix, i, i - 1);
	}
	OF_EXIT_FUNCTION
	return pchkMatrix;
}

#endif /* #ifdef OF_USE_LDPC_STAIRCASE_CODEC */
