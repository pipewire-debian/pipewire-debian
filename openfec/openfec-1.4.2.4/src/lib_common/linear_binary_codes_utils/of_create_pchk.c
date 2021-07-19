/* $Id: of_create_pchk.c 186 2014-07-16 07:17:53Z roca $ */
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

#include "of_linear_binary_code.h"


of_mod2sparse* of_create_pchk_matrix (UINT32		nb_rows,
				      UINT32		nb_cols,
				      make_method	make_method,
				      UINT32		left_degree,
				      UINT32		seed,
				      bool		no4cycle,
				      of_session_type	type,
				      UINT8		verbosity)
{
	OF_ENTER_FUNCTION
	of_mod2sparse* m;
	switch (type) {
	case TypeREGULAR_LDPC:
		m = of_create_pchk_matrix_general (nb_rows, nb_cols, make_method, left_degree,
						   seed, no4cycle, type, verbosity);
		break;

	case Type2DMATRIX:
		m = of_create_2D_pchk_matrix	(nb_rows, nb_cols, type, verbosity);
		break;

	default:
		abort();
		OF_EXIT_FUNCTION
		return NULL;
	}
	OF_EXIT_FUNCTION
	return m;
}


of_mod2sparse* of_create_pchk_matrix_general (UINT32		nb_rows,
					      UINT32		nb_cols,
					      make_method	make_method,
					      UINT32		left_degree,
					      UINT32		seed,
					      bool		no4cycle,
					      of_session_type	type,
					      UINT8		verbosity)
{
	UINT32 row_start = 0;
	UINT32 row_end = 0;
	UINT32 col_start = 0;
	UINT32 col_end = 0;
	INT32 i;
	of_mod2sparse *pchkMatrix = NULL;

	OF_ENTER_FUNCTION
	if (type != TypeLDGM && type != TypeSTAIRS && type != TypeTRIANGLE && type != TypeREGULAR_LDPC)
	{

		OF_PRINT_ERROR(("unsupported code type (%d)\n", type));
		OF_EXIT_FUNCTION
		return NULL;
	}
	/* a few sanity checks... */
	if (left_degree > nb_rows)
	{
		OF_PRINT_ERROR(("number of checks per bit (%d) is greater than total checks (%d)\n",
				left_degree, nb_rows));
		OF_EXIT_FUNCTION
		return NULL;
	}
	if (no4cycle)
	{
		OF_PRINT_ERROR(("no4cycle mode is no longer supported!"));
		OF_EXIT_FUNCTION
		return NULL;
	}
	of_rfc5170_srand (seed);
	pchkMatrix = of_mod2sparse_allocate (nb_rows, nb_cols);

	switch (type) {
	case TypeLDGM:
		row_start = 0;
		row_end = nb_rows;
		col_start = nb_rows;
		col_end = nb_cols;
		break;

	case TypeREGULAR_LDPC:
		row_start = 0;
		row_end = nb_rows;
		col_start = 0;
		col_end = nb_cols;
		break;

	default:
		break;
	}
	of_fill_regular_pchk_matrix (pchkMatrix, row_start, row_end, col_start, col_end, make_method,
				     left_degree, no4cycle, verbosity);

	switch (type) {
	case TypeLDGM:
		for (i = 0; i < nb_rows; i++)
		{
			/* identity */
			of_mod2sparse_insert (pchkMatrix, i, i);
		}
		break;

	default:
		// nothing to do
		break;
	}

	OF_EXIT_FUNCTION
	return pchkMatrix;
}


of_mod2sparse* of_create_2D_pchk_matrix		(UINT32		nb_rows,
						UINT32		nb_cols,
						of_session_type	type,
						UINT8		verbosity)
{
	OF_ENTER_FUNCTION
	of_mod2sparse	*pchkMatrix;
	float		d,l;		// code dimensions for 2D pchk matrix

	if (nb_rows >= nb_cols)
	{
		OF_PRINT_ERROR(("In 2D parity check matrix, number of rows must not be greater than number of cols.\n"))
		goto error;
	}
	for (d = floor(sqrt(nb_cols)); d > 0; d-- )
	{
		l = ((float)((nb_cols-nb_rows))) / d;
		if (l - floor(l) == 0 && ((d + l) == nb_rows))
		{
			//it's OK for a correct 2D-pchk matrix
			pchkMatrix = of_mod2sparse_allocate((l + d), (l * d) + (l + d));
			of_fill_2D_pchk_matrix(pchkMatrix, l, d, verbosity);
			OF_EXIT_FUNCTION
			return pchkMatrix;
		}
	}

error:
	OF_EXIT_FUNCTION
	return NULL;
}


of_mod2sparse* 	of_fill_2D_pchk_matrix 		(of_mod2sparse	*m,
					         UINT32		d,
						 UINT32		l,
					         UINT8		verbosity)
{
	OF_ENTER_FUNCTION
	UINT32		i, j;

	for (i = 0; i < (l+d); i++)
	{
		of_mod2sparse_insert(m, i, i);
	}
	// create non interleaved constraints
	for (i = 0; i < d; i++)
	{
		//set 1 for source symbols included in equation
		for (j = 0; j < l; j++)
		{
			of_mod2sparse_insert(m, i, j + (i * l) + l + d);
		}
	}
	// create interleaved constraints
	for (i = d; i < l + d; i++)
	{
		for (j = 0; j < d; j++)
		{
			of_mod2sparse_insert(m, i, 4 * j + ( i - d )+l+d);
		}
	}
	return m;
	OF_EXIT_FUNCTION
}


of_mod2sparse* of_fill_regular_pchk_matrix (of_mod2sparse	*m,
					    UINT32		row_start,
					    UINT32		row_end,
					    UINT32		col_start,
					    UINT32		col_end,
					    make_method		make_method,
					    UINT32		left_degree,
					    bool		no4cycle,
					    UINT8		verbosity)
{
	of_mod2entry *e;
	UINT32 added, uneven;
	INT32 i, j, k, t;
	UINT32 *u;
	of_mod2sparse *pchkMatrix = m;
	UINT32 nb_col, nb_row;

	OF_ENTER_FUNCTION
	nb_col = col_end - col_start;
	nb_row = row_end - row_start;

	/* Create the initial version of the parity check matrix. */
	switch (make_method)
	{
	case Evencol:
		for (j = col_start; j < col_end; j++)
		{
			for (k = 0; k < left_degree; k++)
			{
				do
				{
					i = of_rfc5170_rand (nb_row);
				}
				while (of_mod2sparse_find (pchkMatrix, i, j));
				of_mod2sparse_insert (pchkMatrix, i, j);
			}
		}
		break;

	case Evenboth:
		u = (UINT32*) of_calloc (left_degree * nb_col, sizeof * u);

		/* initialize a list of possible choices to guarantee a homogeneous "1" distribution */
		for (k = left_degree * nb_col - 1; k >= 0; k--)
		{
			u[k] = k % nb_row;
		}
		uneven = 0;
		t = 0;	/* left limit within the list of possible choices, u[] */
		for (j = col_start; j < col_end; j++)
		{
			/* for each source symbol column */
			for (k = 0; k < left_degree; k++)
			{
				/* add left_degree "1s" */
				/* check that valid available choices remain */
				for (i = t; i < left_degree*nb_col && of_mod2sparse_find (pchkMatrix, u[i], j); i++)
					;

				if (i < left_degree*nb_col)
				{
					/* choose one index within the list of possible choices */
					do
					{
						i = t + of_rfc5170_rand (left_degree * nb_col - t);
					}
					while (of_mod2sparse_find (pchkMatrix, u[i], j));
					of_mod2sparse_insert (pchkMatrix, u[i], j);
					/* replace with u[t] which has never been chosen */
					u[i] = u[t];
					t++;
				}
				else
				{
					/* no choice left, choose one randomly */
					uneven += 1;
					do
					{
						i = of_rfc5170_rand (nb_row);
					}
					while (of_mod2sparse_find (pchkMatrix, i, j));
					of_mod2sparse_insert (pchkMatrix, i, j);
				}
			}
		}

		if (uneven > 0 && verbosity >= 1)
		{
			OF_PRINT_ERROR(("Had to place %d checks in rows unevenly\n", uneven));
		}
		of_free (u);	/* VR: added */
		break;

	default:
		abort();
	}

	/* Add extra bits to avoid rows with less than two checks. */
	added = 0;
	for (i = row_start; i < row_end; i++)
	{
		e = of_mod2sparse_first_in_row (pchkMatrix, i);
		if (of_mod2sparse_at_end (e))
		{
			j = (of_rfc5170_rand (nb_col) + col_start);
			e = of_mod2sparse_insert (pchkMatrix, i, j);
			added ++;
		}
		e = of_mod2sparse_first_in_row (pchkMatrix, i);
		if (of_mod2sparse_at_end (of_mod2sparse_next_in_row (e)) && nb_col > 1)
		{
			do
			{
				j = (of_rfc5170_rand (nb_col)) + col_start;
			}
			while (j == of_mod2sparse_col (e));
			of_mod2sparse_insert (pchkMatrix, i, j);
			added ++;
		}
	}

	if (added > 0 && verbosity >= 1)
	{
		OF_PRINT_ERROR(("Added %d extra bit-checks to make row counts at least two\n", added));
	}

	/* Add extra bits to try to avoid problems with even column counts. */
	if (left_degree % 2 == 0 && left_degree < nb_row && nb_col > 1 && added < 2)
	{
		UINT32 a;
		for (a = 0; added + a < 2; a++)
		{
			do
			{
				i = of_rfc5170_rand (nb_row);
				j = (of_rfc5170_rand (nb_col)) + col_start;
			}
			while (of_mod2sparse_find (pchkMatrix, i, j));
			of_mod2sparse_insert (pchkMatrix, i, j);
		}
		if (verbosity >= 1)
		{
			OF_PRINT_ERROR(("Added %d extra bit-checks to try to avoid problems from even column counts\n", a));
		}
		OF_PRINT_LVL (1, ("Added %d extra bit-checks to try to avoid problems from even column counts\n", a));
	}

	OF_EXIT_FUNCTION

	return pchkMatrix;
}


