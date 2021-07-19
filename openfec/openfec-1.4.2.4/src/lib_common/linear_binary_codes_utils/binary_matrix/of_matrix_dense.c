
/* Copyright (c) 1996, 2000, 2001 by Radford M. Neal
 *
 * Permission is granted for anyone to copy, use, modify, or distribute this
 * program and accompanying programs and documents for any purpose, provided
 * this copyright notice is retained and prominently displayed, along with
 * a note saying that the original programs are available from Radford Neal's
 * web page, and note is made of any changes made to the programs.  The
 * programs and documents are distributed without any warranty, express or
 * implied.  As the programs were written for research purposes only, they have
 * not been tested to the degree that would be advisable in any important
 * application.  All use of these programs is entirely at the user's own risk.
 */

#include "../of_linear_binary_code.h"


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

#define ROW_MAX_ENTRY 1000
#define COL_MAX_ENTRY 1000

/* ALLOCATE SPACE FOR A DENSE MOD2 MATRIX. */

of_mod2dense *of_mod2dense_allocate (UINT32	n_rows,		/* Number of rows in matrix */
				     UINT32	n_cols)		/* Number of columns in matrix */
{
	of_mod2dense	*m;
	UINT32		j;

	OF_ENTER_FUNCTION
	if (n_rows <= 0 || n_cols <= 0)
	{
		OF_PRINT_ERROR(("mod2dense_allocate: Invalid number of rows (%d) or columns (%d)\n", n_rows, n_cols))
		return NULL;
	}
	m = (of_mod2dense *) of_calloc(1, sizeof(*m));
	m->n_rows = n_rows;
	m->n_cols = n_cols;
#ifdef COL_ORIENTED
	m->n_words = (n_rows + of_mod2_wordsize - 1) >> of_mod2_wordsize_shift;
	m->col = of_calloc (m->n_cols, sizeof(*m->col));
	m->bits = of_calloc (m->n_words * m->n_cols, sizeof(*m->bits));
	for (j = 0; j < m->n_cols; j++)
	{
		m->col[j] = m->bits + j * m->n_words;
	}
#else
	m->n_words = (n_cols + of_mod2_wordsize - 1) >> of_mod2_wordsize_shift;
	m->row = (of_mod2word**) of_calloc (m->n_rows, sizeof(*m->row));
	m->bits = (of_mod2word*) of_calloc (m->n_words * m->n_rows, sizeof(*m->bits));
	for (j = 0; j < m->n_rows; j++)
	{
		m->row[j] = m->bits + j * m->n_words;	
	}
#endif
	OF_EXIT_FUNCTION
	return m;
}


/* FREE SPACE OCCUPIED BY A DENSE MOD2 MATRIX. */

void of_mod2dense_free (of_mod2dense *m)		/* Matrix to free */
{
	OF_ENTER_FUNCTION
	of_free (m->bits);
#ifdef COL_ORIENTED
	of_free (m->col);
#else
	of_free (m->row);
#endif
	of_free (m);
	OF_EXIT_FUNCTION
}


/* CLEAR A DENSE MOD2 MATRIX. */

void of_mod2dense_clear (of_mod2dense *r)
{
	OF_ENTER_FUNCTION
	UINT32 k, j;
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->col[j][k] = 0;
		}
	}
#else
	
	for (j = 0; j < of_mod2dense_rows (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->row[j][k] = 0;
		}
	}
#endif
	OF_EXIT_FUNCTION
}


/* COPY A DENSE MOD2 MATRIX. */

void of_mod2dense_copy (of_mod2dense *m,	/* Matrix to copy */
			of_mod2dense *r)	/* Place to store copy of matrix */
{
	OF_ENTER_FUNCTION
	UINT32 k, j;


	if (of_mod2dense_rows (m) > of_mod2dense_rows (r)
			|| of_mod2dense_cols (m) > of_mod2dense_cols (r))
	{
		OF_PRINT_ERROR(("mod2dense_copy: Destination matrix is too small\n"))
		OF_EXIT_FUNCTION
		return;
	}
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			r->col[j][k] = m->col[j][k];
		}
		for (; k < r->n_words; k++)
		{
			r->col[j][k] = 0;
		}
	}

	for (; j < of_mod2dense_cols (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->col[j][k] = 0;
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			r->row[j][k] = m->row[j][k];
		}
		for (; k < r->n_words; k++)
		{
			r->row[j][k] = 0;
		}
	}

	for (; j < of_mod2dense_rows (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->row[j][k] = 0;
		}
	}
#endif
	OF_EXIT_FUNCTION
}


/* COPY ROWS OF A DENSE MOD2 MATRIX. */

void of_mod2dense_copyrows     (of_mod2dense	*m,	/* Matrix to copy */
				of_mod2dense	*r,	/* Place to store copy of matrix */
				UINT32		*rows)	/* Indexes of rows to copy, from 0 */
{
	OF_ENTER_FUNCTION
	UINT32 i, j, k;

	if (of_mod2dense_cols (m) > of_mod2dense_cols (r))
	{
		OF_PRINT_ERROR(("mod2dense_copyrows: Destination matrix has fewer columns than source\n"))
		OF_EXIT_FUNCTION
		return;
	}

	of_mod2dense_clear (r);


	for (i = 0; i < of_mod2dense_rows (r); i++)
	{
		if (rows[i] >= of_mod2dense_rows (m))
		{
			OF_PRINT_ERROR(("mod2dense_copyrows: Row index out of range\n"))
			OF_EXIT_FUNCTION
			return;
		}
#ifdef COL_ORIENTED
		for (j = 0; j < of_mod2dense_cols (m); j++)
		{
			of_mod2dense_set (r, i, j, of_mod2dense_get (m, rows[i], j));
		}
#else
		for (j = 0; j < of_mod2dense_cols (r); j++)
		{
			if (rows[j] >= of_mod2dense_rows (m))
			{
				OF_PRINT_ERROR(("mod2dense_copycols: Column index out of range\n"))
				OF_EXIT_FUNCTION
				return;
			}
			for (k = 0; k < m->n_words; k++)
			{
				r->row[j][k] = m->row[rows[j]][k];
			}
			for (; k < r->n_words; k++)
			{
				r->row[j][k] = 0;
			}
		}

#endif
	}

	OF_EXIT_FUNCTION
}


/* COPY COLUMNS OF A DENSE MOD2 MATRIX. */

void of_mod2dense_copycols     (of_mod2dense	*m,		/* Matrix to copy */
				of_mod2dense	*r,		/* Place to store copy of matrix */
				UINT32		*cols)		/* Indexes of columns to copy, from 0 */
{
	UINT32  i, j;

	OF_ENTER_FUNCTION
	if (of_mod2dense_rows (m) > of_mod2dense_rows (r))
	{
		OF_PRINT_ERROR(("mod2dense_copycols: Destination matrix has fewer rows than source\n"))
		OF_EXIT_FUNCTION
		return;
	}

	for (j = 0; j < of_mod2dense_cols (r); j++)
	{
#ifdef COL_ORIENTED
		if (cols[j] < 0 || cols[j] >= of_mod2dense_cols (m))
		{
			OF_PRINT_ERROR(("mod2dense_copycols: Column index out of range\n"))
			OF_EXIT_FUNCTION
			return;
		}

		for (k = 0; k < m->n_words; k++)
		{
			r->col[j][k] = m->col[cols[j]][k];
		}
		for (; k < r->n_words; k++)
		{
			r->col[j][k] = 0;
		}

#else
		for (i = 0; i < of_mod2dense_rows (m); i++)
		{
			of_mod2dense_set (r, i, j, of_mod2dense_get (m, i, cols[j]));
		}
#endif
	}
	OF_EXIT_FUNCTION
}


/* PRINT A DENSE MOD2 MATRIX IN HUMAN-READABLE FORM. */

void of_mod2dense_print (FILE		*f,
			 of_mod2dense	*m)
{
	UINT32 i, j;

	OF_ENTER_FUNCTION
	for (i = 0; i < of_mod2dense_rows (m); i++)
	{
		for (j = 0; j < of_mod2dense_cols (m); j++)
		{
			if( of_mod2dense_get (m, i, j) !=0)
			{
				fprintf (f, " %d", of_mod2dense_get (m, i, j));
			}
			else
			{
				fprintf (f, "  "/*, of_mod2dense_get (m, i, j)*/);	
			}
		}
		fprintf (f, "\n");
	}
	OF_EXIT_FUNCTION
}


/* PRINT A DENSEMOD2 MATRIX IN HUMAN-READABLE FORM. */
void of_mod2dense_print_bitmap (of_mod2dense	*m,
				char		*fname)
{
#ifdef IL_SUPPORT	
	UINT32	i, j;
	UINT32	x,y;
	ILuint	lImage;	
	char	cmd[100];
	
	OF_ENTER_FUNCTION
	ilInit();
	ilGenImages(1, &lImage);
	ilBindImage(lImage);
	//of_mod2dense_print_stats(stdout,m);
	ilTexImage(of_mod2dense_cols (m),of_mod2dense_rows (m)  , 1, 3, IL_RGB, IL_UNSIGNED_BYTE, NULL);
	UINT32 val =1;
	y=of_mod2dense_rows (m);
	x=of_mod2dense_cols (m);
	for (i = 0; i < of_mod2dense_rows (m); i++)
	{
		for (j = 0; j < of_mod2dense_cols (m); j++)
		{
			if (of_mod2dense_get (m, i, j))
			{
				val = 0x00FF00;	/* bit is set, use green */
			}
			else
			{
				val = 0xFFFFFF;	/* bit is not set, use white */
			}
			x=j;
			y=of_mod2dense_rows (m)-i-1;
			ilSetPixels(x ,y, 0, 1, 1, 1, IL_RGBA, IL_UNSIGNED_BYTE, &val);
		}
	}
	if (fname == NULL)
	{
		fname = "dense.bmp";
	}
	/* use open on MacOS machines, EOG (Eye of Gnome) in Linux machines, and something else if needed... */
	//snprintf(cmd, 99, "open %s", fname);
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage(fname);
	//system(cmd);
	OF_EXIT_FUNCTION
	return;
#endif
}


/* WRITE A DENSE MOD2 MATRIX TO A FILE IN MACHINE-READABLE FORM.

   Data is written using of_intio_write, so that it will be readable on a machine
   with a different byte-ordering.  At present, this assumes that the words
   used to pack bits into are no longer than 32 bits. */

UINT32 of_mod2dense_write (FILE *f, of_mod2dense *m)
{
	OF_ENTER_FUNCTION
	UINT32 j, k;

	of_intio_write (f, m->n_rows);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	of_intio_write (f, m->n_cols);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			of_intio_write (f, m->col[j][k]);
			if (ferror (f))
			{
				OF_EXIT_FUNCTION
				return 0;
			}
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			of_intio_write (f, m->row[j][k]);
			if (ferror (f))
			{
				OF_EXIT_FUNCTION
				return 0;
			}
		}
	}
#endif
	OF_EXIT_FUNCTION
	return 1;
}


/* READ A DENSE MOD2 MATRIX STORED IN MACHINE-READABLE FORM FROM A FILE. */

of_mod2dense *of_mod2dense_read (FILE *f)
{
	OF_ENTER_FUNCTION
	UINT32 n_rows, n_cols;
	of_mod2dense *m;
	UINT32 j, k;

	n_rows = of_intio_read (f);
	if (feof (f) || ferror (f) || n_rows <= 0)
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	n_cols = of_intio_read (f);
	if (feof (f) || ferror (f) || n_cols <= 0)
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	m = of_mod2dense_allocate (n_rows, n_cols);
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			m->col[j][k] = of_intio_read (f);
			if (feof (f) || ferror (f))
			{
				of_mod2dense_free(m);
				OF_EXIT_FUNCTION
				return 0;
			}
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (m); j++)
	{
		for (k = 0; k < m->n_words; k++)
		{
			m->row[j][k] = of_intio_read (f);
			if (feof (f) || ferror (f))
			{
				of_mod2dense_free(m);
				OF_EXIT_FUNCTION
				return 0;
			}
		}
	}
#endif
	OF_EXIT_FUNCTION
	return m;
}


/* GET AN ELEMENT FROM A DENSE MOD2 MATRIX. */

UINT32 of_mod2dense_get (of_mod2dense	*m, 	/* Matrix to get element from */
			 UINT32		row,	/* Row of element (starting with zero) */
			 UINT32		col)	/* Column of element (starting with zero) */
{
	//OF_ENTER_FUNCTION
#ifdef OF_DEBUG
	if ( row >= of_mod2dense_rows (m) || col >= of_mod2dense_cols (m))
	{
		OF_PRINT_ERROR(("mod2dense_get: row (%d) or column index (%d) out of bounds (resp. %d and %d)\n", row, col,
				of_mod2dense_rows (m), of_mod2dense_cols (m)))
		OF_EXIT_FUNCTION
		return -1;
	}
#endif
#ifdef COL_ORIENTED
	return of_mod2_getbit (m->col[col][row>>of_mod2_wordsize_shift],
			    row&of_mod2_wordsize_mask);
#else
	return of_mod2_getbit (m->row[row][col>>of_mod2_wordsize_shift],
			    col&of_mod2_wordsize_mask);
#endif
	//OF_EXIT_FUNCTION
}


/* SET AN ELEMENT IN A DENSE MOD2 MATRIX. */

INT32 of_mod2dense_set (of_mod2dense	*m, 	/* Matrix to modify element of */
			UINT32		row,		/* Row of element (starting with zero) */
			UINT32		col,		/* Column of element (starting with zero) */
			UINT32		value)		/* New value of element (0 or 1) */
{
	of_mod2word *w;

	//OF_ENTER_FUNCTION
	if (row >= of_mod2dense_rows (m) || col >= of_mod2dense_cols (m))
	{
		OF_PRINT_ERROR(("mod2dense_set: row (%d) or column index (%d) out of bounds (resp. %d and %d)\n", row, col,
				of_mod2dense_rows (m), of_mod2dense_cols (m)))
		OF_EXIT_FUNCTION
		return -1;
	}
#ifdef COL_ORIENTED
	w = &m->col[col][row>>of_mod2_wordsize_shift];

	*w = value ? of_mod2_setbit1 (*w, row & of_mod2_wordsize_mask)
	     : of_mod2_setbit0 (*w, row & of_mod2_wordsize_mask);
#else
	w = &m->row[row][col>>of_mod2_wordsize_shift];

	*w = value ? of_mod2_setbit1 (*w, col & of_mod2_wordsize_mask)
	     : of_mod2_setbit0 (*w, col & of_mod2_wordsize_mask);
#endif
	//OF_EXIT_FUNCTION
	return 0;
}


/* FLIP AN ELEMENT OF A DENSE MOD2 MATRIX. */

UINT32 of_mod2dense_flip (of_mod2dense	*m, 	/* Matrix to flip element in */
			  UINT32	row,	/* Row of element (starting with zero) */
			  UINT32	col)	/* Column of element (starting with zero) */
{
	of_mod2word *w;
	UINT32 b;

	OF_ENTER_FUNCTION
	if (row >= of_mod2dense_rows (m) || col >= of_mod2dense_cols (m))
	{
		OF_PRINT_ERROR(("mod2dense_flip: row (%d) or column index (%d) out of bounds (resp. %d and %d)\n", row, col,
				of_mod2dense_rows (m), of_mod2dense_cols (m)))
		OF_EXIT_FUNCTION
		return -1;
	}
#ifdef COL_ORIENTED
	b = 1 ^ of_mod2_getbit (m->col[col][row>>of_mod2_wordsize_shift],
			     row & of_mod2_wordsize_mask);

	w = &m->col[col][row>>of_mod2_wordsize_shift];

	*w = b ? of_mod2_setbit1 (*w, row & of_mod2_wordsize_mask)
	     : of_mod2_setbit0 (*w, row & of_mod2_wordsize_mask);
#else
	b = 1 ^ of_mod2_getbit (m->row[row][col>>of_mod2_wordsize_shift],
			     col & of_mod2_wordsize_mask);

	w = &m->row[row][col>>of_mod2_wordsize_shift];

	*w = b ? of_mod2_setbit1 (*w, col & of_mod2_wordsize_mask)
	     : of_mod2_setbit0 (*w, col & of_mod2_wordsize_mask);

#endif
	OF_EXIT_FUNCTION
	return b;
}


#if 0 /* { */

/* COMPUTE THE TRANSPOSE OF A DENSE MOD2 MATRIX. */

void of_mod2dense_transpose
(of_mod2dense *m,		/* Matrix to compute transpose of (left unchanged) */
 of_mod2dense *r		/* Result of transpose operation */
)
{
	OF_ENTER_FUNCTION
	of_mod2word w, v, *p;
	UINT32 k1, j1, i2, j2;

	if (of_mod2dense_rows (m) != of_mod2dense_cols (r)
			|| of_mod2dense_cols (m) != of_mod2dense_rows (r))
	{
		OF_PRINT_ERROR(("mod2dense_transpose: Matrices have incompatible dimensions\n"))
		OF_EXIT_FUNCTION
		return;
	}

	if (r == m)
	{
		OF_PRINT_ERROR(("mod2dense_transpose: Result matrix is the same as the operand\n"))
		OF_EXIT_FUNCTION
		return;
	}

	of_mod2dense_clear (r);
#ifdef COL_ORIENTED
	for (j1 = 0; j1 < of_mod2dense_cols (m); j1++)
	{
		i2 = j1 >> of_mod2_wordsize_shift;
		v = 1 << (j1 & of_mod2_wordsize_mask);

		p = m->col[j1];
		k1 = 0;

		for (j2 = 0; j2 < of_mod2dense_cols (r); j2++)
		{
			if (k1 == 0)
			{
				w = *p++;
				k1 = of_mod2_wordsize;
			}
			if (w&1)
			{
				r->col[j2][i2] |= v;
			}
			w >>= 1;
			k1 -= 1;
		}
	}
#else
	for (j1 = 0; j1 < of_mod2dense_rows (m); j1++)
	{
		i2 = j1 >> of_mod2_wordsize_shift;
		v = 1 << (j1 & of_mod2_wordsize_mask);

		p = m->row[j1];
		k1 = 0;

		for (j2 = 0; j2 < of_mod2dense_rows (r); j2++)
		{
			if (k1 == 0)
			{
				w = *p++;
				k1 = of_mod2_wordsize;
			}
			if (w&1)
			{
				r->row[j2][i2] |= v;
			}
			w >>= 1;
			k1 -= 1;
		}
	}
#endif
	OF_EXIT_FUNCTION
}


/* ADD TWO DENSE MOD2 MATRICES. */

void of_mod2dense_add
(of_mod2dense *m1,	/* Left operand of add */
 of_mod2dense *m2,	/* Right operand of add */
 of_mod2dense *r		/* Place to store result of add */
)
{
	OF_ENTER_FUNCTION
	UINT32 j, k;

	if (of_mod2dense_rows (m1) != of_mod2dense_rows (r)
			|| of_mod2dense_cols (m1) != of_mod2dense_cols (r)
			|| of_mod2dense_rows (m2) != of_mod2dense_rows (r)
			|| of_mod2dense_cols (m2) != of_mod2dense_cols (r))
	{
		OF_PRINT_ERROR(("mod2dense_add: Matrices have different dimensions\n"))
		OF_EXIT_FUNCTION
		return;
	}
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->col[j][k] = m1->col[j][k] ^ m2->col[j][k];
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (r); j++)
	{
		for (k = 0; k < r->n_words; k++)
		{
			r->row[j][k] = m1->row[j][k] ^ m2->row[j][k];
		}
	}

#endif
	OF_EXIT_FUNCTION
}


/* MULTIPLY TWO DENSE MOD2 MATRICES.

   The algorithm used runs faster if the second matrix (right operand of the
   multiply) is sparse, but it is also appropriate for dense matrices.  This
   procedure could be speeded up a bit by replacing the call of mod2dense_get
   with in-line code that avoids division, but this doesn't seem worthwhile
   at the moment.
*/

void of_mod2dense_multiply
(of_mod2dense *m1, 	/* Left operand of multiply */
 of_mod2dense *m2,	/* Right operand of multiply */
 of_mod2dense *r		/* Place to store result of multiply */
)
{
	OF_ENTER_FUNCTION
	UINT32 i, j, k;

	if (of_mod2dense_cols (m1) != of_mod2dense_rows (m2)
			|| of_mod2dense_rows (m1) != of_mod2dense_rows (r)
			|| of_mod2dense_cols (m2) != of_mod2dense_cols (r))
	{
		OF_PRINT_ERROR(("mod2dense_multiply: Matrices have incompatible dimensions\n"))
		OF_EXIT_FUNCTION
		return;
	}

	if (r == m1 || r == m2)
	{
		OF_PRINT_ERROR(("mod2dense_multiply: Result matrix is the same as one of the operands\n"))
		OF_EXIT_FUNCTION
		return;
	}

	of_mod2dense_clear (r);
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (r); j++)
	{
		for (i = 0; i < of_mod2dense_rows (m2); i++)
		{
			if (of_mod2dense_get (m2, i, j))
			{
				for (k = 0; k < r->n_words; k++)
				{
					r->col[j][k] ^= m1->col[i][k];
				}
			}
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (r); j++)
	{
		for (i = 0; i < of_mod2dense_cols (m2); i++)
		{
			if (of_mod2dense_get (m2, j, i))
			{
				for (k = 0; k < r->n_words; k++)
				{
					r->row[i][k] ^= m1->row[j][k];
				}
			}
		}
	}
#endif
	OF_EXIT_FUNCTION;
}


/* SEE WHETHER TWO DENSE MOD2 MATRICES ARE EQUAL. */

UINT32 of_mod2dense_equal
(of_mod2dense *m1,
 of_mod2dense *m2
)
{
	OF_ENTER_FUNCTION
	UINT32 k, j, w;
	of_mod2word m;

	if (of_mod2dense_rows (m1) != of_mod2dense_rows (m2)
			|| of_mod2dense_cols (m1) != of_mod2dense_cols (m2))
	{
		OF_PRINT_ERROR(("mod2dense_equal: Matrices have different dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	w = m1->n_words;

	/* Form a mask that has 1s in the lower bit positions corresponding to
	   bits that contain information in the last word of a matrix column. */
#ifdef COL_ORIENTED
	m = (1 << (of_mod2_wordsize - (w * of_mod2_wordsize - m1->n_rows))) - 1;

	for (j = 0; j < of_mod2dense_cols (m1); j++)
	{
		for (k = 0; k < w - 1; k++)
		{
			if (m1->col[j][k] != m2->col[j][k])
			{
				OF_EXIT_FUNCTION
				return 0;
			}
		}

		if ( (m1->col[j][k]&m) != (m2->col[j][k]&m))
		{
			OF_EXIT_FUNCTION
			return 0;
		}
	}

#else
	m = (1 << (of_mod2_wordsize - (w * of_mod2_wordsize - m1->n_cols))) - 1;

	for (j = 0; j < of_mod2dense_rows (m1); j++)
	{
		for (k = 0; k < w - 1; k++)
		{
			if (m1->row[j][k] != m2->row[j][k])
			{
				OF_EXIT_FUNCTION
				return 0;
			}
		}

		if ( (m1->row[j][k]&m) != (m2->row[j][k]&m))
		{
			OF_EXIT_FUNCTION
			return 0;
		}
	}
#endif
	OF_EXIT_FUNCTION
	return 1;
}


/* INVERT A DENSE MOD2 MATRIX. */

UINT32 of_mod2dense_invert
(of_mod2dense *m,		/* The matrix to find the inverse of (destroyed) */
 of_mod2dense *r		/* Place to store the inverse */
)
{
	OF_ENTER_FUNCTION
	of_mod2word *s, *t;
	UINT32 i, j, k, n, w, k0, b0;

	if (of_mod2dense_rows (m) != of_mod2dense_cols (m))
	{
		OF_PRINT_ERROR(("mod2dense_invert: Matrix to invert is not square\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	if (r == m)
	{
		OF_PRINT_ERROR(("mod2dense_invert: Result matrix is the same as the operand\n"))
		OF_EXIT_FUNCTION
		return -1;
	}
#ifdef COL_ORIENTED
	n = of_mod2dense_rows (m);

	w = m->n_words;

	if (of_mod2dense_rows (r) != n || of_mod2dense_cols (r) != n)
	{
		OF_PRINT_ERROR(("mod2dense_invert: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	of_mod2dense_clear (r);
	for (i = 0; i < n; i++)
	{
		of_mod2dense_set (r, i, i, 1);
	}

	for (i = 0; i < n; i++)
	{
		k0 = i >> of_mod2_wordsize_shift;
		b0 = i & of_mod2_wordsize_mask;

		for (j = i; j < n; j++)
		{
			if (of_mod2_getbit (m->col[j][k0], b0))
				break;
		}

		if (j == n)
		{
			OF_EXIT_FUNCTION
			return 0;
		}

		if (j != i)
		{
			t = m->col[i];
			m->col[i] = m->col[j];
			m->col[j] = t;

			t = r->col[i];
			r->col[i] = r->col[j];
			r->col[j] = t;
		}

		for (j = 0; j < n; j++)
		{
			if (j != i && of_mod2_getbit (m->col[j][k0], b0))
			{
				s = m->col[j];
				t = m->col[i];
				for (k = k0; k < w; k++)
					s[k] ^= t[k];
				s = r->col[j];
				t = r->col[i];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}
	}
#else
	n = of_mod2dense_cols (m);

	w = m->n_words;

	if (of_mod2dense_cols (r) != n || of_mod2dense_rows (r) != n)
	{
		OF_PRINT_ERROR(("mod2dense_invert: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	of_mod2dense_clear (r);
	for (i = 0; i < n; i++)
	{
		of_mod2dense_set (r, i, i, 1);
	}


	for (i = 0; i < n; i++)
	{
		k0 = i >> of_mod2_wordsize_shift;
		b0 = i & of_mod2_wordsize_mask;

		for (j = i; j < n; j++)
		{
			if (of_mod2_getbit (m->row[j][k0], b0))
				break;
		}

		if (j == n)
		{
			OF_EXIT_FUNCTION
			return 0;
		}

		if (j != i)
		{
			t = m->row[i];
			m->row[i] = m->row[j];
			m->row[j] = t;

			t = r->row[i];
			r->row[i] = r->row[j];
			r->row[j] = t;
		}

		for (j = 0; j < n; j++)
		{
			if (j != i && of_mod2_getbit (m->row[j][k0], b0))
			{
				s = m->row[j];
				t = m->row[i];
				for (k = k0; k < w; k++)
					s[k] ^= t[k];
				s = r->row[j];
				t = r->row[i];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}
	}
#endif
	OF_EXIT_FUNCTION
	return 1;
}


/* TRIANGULARIZE A DENSE MOD2 MATRIX. */

UINT32 of_mod2dense_triangularize
(of_mod2dense *m,		/* The matrix to find the inverse of (destroyed) */
 of_mod2dense *r		/* Place to store the inverse */
)
{
	OF_ENTER_FUNCTION
	of_mod2word *s, *t;
	UINT32 i, j, k, n, p, w, k0, b0;

	/*  if (of_mod2dense_rows(m)!=of_mod2dense_cols(m)) */
	/*   { fprintf(stderr,"of_mod2dense_triangularize: Matrix to invert is not square\n"); */
	/*     exit(1); */
	/*   } */

	if (r == m)
	{
		OF_PRINT_ERROR(("mod2dense_triangularize: Result matrix is the same as the operand\n"))
		OF_EXIT_FUNCTION
		return -1;
	}
#ifdef COL_ORIENTED

	n = of_mod2dense_rows (m);
	p = of_mod2dense_cols (m);
	w = m->n_words;

	if (of_mod2dense_rows (r) != n || of_mod2dense_cols (r) != p)
	{
		OF_PRINT_ERROR(("mod2dense_triangularize: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	// r is now the identity
	of_mod2dense_clear (r);
	for (i = 0; i < p; i++)
	{
		of_mod2dense_set (r, i, i, 1);
	}


	/* for each line*/
	for (i = 0; i < n; i++)
	{
		k0 = i >> of_mod2_wordsize_shift; // word index of the ith bit
		b0 = i & of_mod2_wordsize_mask;  // bit index of the ith bit in the k0-th word

		/* search for the first non-null element */
		for (j = i; j < p; j++)
		{
			if (of_mod2_getbit (m->col[j][k0], b0))
				break;
		}
		// now j is the index of the first non null element in the line i
		if (j == p)
		{
			OF_EXIT_FUNCTION
			return 0;
		}

		if (j != i)
		{
			// swap column i an j the two matrices
			t = m->col[i];
			m->col[i] = m->col[j];
			m->col[j] = t;

			t = r->col[i];
			r->col[i] = r->col[j];
			r->col[j] = t;
		}


		for (j = i; j < p; j++)
		{
			if (j != i && of_mod2_getbit (m->col[j][k0], b0))
			{
				// for all the column were the ith element is non null
				s = m->col[j];
				t = m->col[i];
				for (k = k0; k < w; k++)
					s[k] ^= t[k]; // add column i with coluln j
				s = r->col[j];
				t = r->col[i];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}
	}
#else

	n = of_mod2dense_cols (m);
	p = of_mod2dense_rows (m);
	w = m->n_words;

	if (of_mod2dense_cols (r) != n || of_mod2dense_rows (r) != p)
	{
		OF_PRINT_ERROR(("mod2dense_triangularize: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	// r is now the identity
	of_mod2dense_clear (r);
	for (i = 0; i < n; i++)
	{
		of_mod2dense_set (r, i, i, 1);
	}


	/* for each line*/
	for (i = 0; i < n; i++)
	{
		k0 = i >> of_mod2_wordsize_shift; // word index of the ith bit
		b0 = i & of_mod2_wordsize_mask;  // bit index of the ith bit in the k0-th word

		/* search for the first non-null element */
		for (j = i; j < p; j++)
		{
			if (of_mod2_getbit (m->row[j][k0], b0))
				break;
		}
		// now j is the index of the first non null element in the line i
		if (j == p)
		{
			OF_EXIT_FUNCTION
			return 0;
		}

		if (j != i)
		{
			// swap column i an j the two matrices
			t = m->row[i];
			m->row[i] = m->row[j];
			m->row[j] = t;

			t = r->row[i];
			r->row[i] = r->row[j];
			r->row[j] = t;
			// TODO : swap the partial sum

		}

		for (j = 0; j < p; j++)
			//for (j = i; j<p; j++)
		{
			if (j != i && of_mod2_getbit (m->row[j][k0], b0))
			{
				// for all the column were the ith element is non null
				s = m->row[j];
				t = m->row[i];
				for (k = k0; k < w; k++)
					s[k] ^= t[k]; // add column i with coluln j
				s = r->row[j];
				t = r->row[i];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
				// TODO : add the m_checkValues of the line
			}
		}
	}

#endif
	OF_EXIT_FUNCTION
	return 1;
}


/* INVERT A DENSE MOD2 MATRIX WITH ROWS & COLUMNS SELECTED FROM BIGGER MATRIX.*/

UINT32 of_mod2dense_invert_selected
(of_mod2dense *m,		/* Matrix from which to pick a submatrix to invert */
 of_mod2dense *r,		/* Place to store the inverse */
 UINT32 *rows,		/* Set to indexes of rows used and not used */
 UINT32 *cols		/* Set to indexes of columns used and not used */
)
{
	OF_ENTER_FUNCTION
	of_mod2word *s, *t;
	UINT32  R;
	UINT32 i, j, k, n,  w, k0, b0, n2, c;

	if (r == m)
	{
		OF_PRINT_ERROR(("mod2dense_invert_selected2: Result matrix is the same as the operand\n"))
		OF_EXIT_FUNCTION
		return -1;
	}
#ifdef COL_ORIENTED
	n = of_mod2dense_rows (m);
	w = m->n_words;

	n2 = of_mod2dense_cols (m);

	if (of_mod2dense_rows (r) != n || of_mod2dense_cols (r) != n2)
	{
		OF_PRINT_ERROR(("mod2dense_invert_selected2: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	of_mod2dense_clear (r);

	for (i = 0; i < n; i++)
	{
		rows[i] = i;
	}

	for (j = 0; j < n2; j++)
	{
		cols[j] = j;
	}

	R = 0;
	i = 0;

	for (;;)
	{
		while (i < n - R)
		{
			k0 = rows[i] >> of_mod2_wordsize_shift;
			b0 = rows[i] & of_mod2_wordsize_mask;

			for (j = i; j < n2; j++)
			{
				if (of_mod2_getbit (m->col[cols[j]][k0], b0))
					break;
			}

			if (j < n2)
				break;

			R += 1;
			c = rows[i];
			rows[i] = rows[n-R];
			rows[n-R] = c;

		}

		if (i == n - R)
			break;

		c = cols[j];
		cols[j] = cols[i];
		cols[i] = c;

		of_mod2dense_set (r, rows[i], c, 1);

		for (j = 0; j < n2; j++)
		{
			if (j != c && of_mod2_getbit (m->col[j][k0], b0))
			{
				s = m->col[j];
				t = m->col[c];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
				s = r->col[j];
				t = r->col[c];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}

		i += 1;
	}

	for (j = n - R; j < n; j++)
	{
		s = r->col[cols[j]];
		for (k = 0; k < w; k++)
			s[k] = 0;
	}

#else
	n = of_mod2dense_cols (m);
	w = m->n_words;

	n2 = of_mod2dense_rows (m);

//  if (of_mod2dense_rows(r)!=n || of_mod2dense_cols(r)!=n2)
//   { fprintf(stderr,
// "mod2dense_invert_selected: Matrix to receive inverse has wrong dimensions\n");
//     exit(1);
//   }

	of_mod2dense_clear (r);

	for (i = 0; i < n; i++)
	{
		rows[i] = i;
	}

	for (j = 0; j < n2; j++)
	{
		cols[j] = j;
	}

	R = 0;
	i = 0;

	for (;;)
	{
		while (i < n - R)
		{
			k0 = cols[i] >> of_mod2_wordsize_shift;
			b0 = cols[i] & of_mod2_wordsize_mask;

			for (j = i; j < n2; j++)
			{
				if (of_mod2_getbit (m->row[rows[j]][k0], b0))
					break;
			}

			if (j < n2)
				break;

			R += 1;
			c = cols[i];
			cols[i] = cols[n-R];
			cols[n-R] = c;

		}

		if (i == n - R)
			break;

		c = rows[j];
		rows[j] = rows[i];
		rows[i] = c;

		of_mod2dense_set (r, c, cols[i], 1);

		for (j = 0; j < n2; j++)
		{
			if (j != c && of_mod2_getbit (m->row[j][k0], b0))
			{
				s = m->row[j];
				t = m->row[c];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
				s = r->row[j];
				t = r->row[c];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}

		i += 1;
	}

	for (j = n - R; j < n; j++)
	{
		s = r->row[rows[j]];
		for (k = 0; k < w; k++)
			s[k] = 0;
	}
#endif
	OF_EXIT_FUNCTION
	return R;
}


/* FORCIBLY INVERT A DENSE MOD2 MATRIX. */

UINT32 of_mod2dense_forcibly_invert
(of_mod2dense *m, 	/* The matrix to find the inverse of (destroyed) */
 of_mod2dense *r,		/* Place to store the inverse */
 UINT32 *a_row,		/* Place to store row indexes of altered elements */
 UINT32 *a_col		/* Place to store column indexes of altered elements */
)
{
	OF_ENTER_FUNCTION
	//  of_mod2word *s, *t;

	UINT32  c = 0;
#ifdef COL_ORIENTED
	if (of_mod2dense_rows (m) != of_mod2dense_cols (m))
	{
		OF_PRINT_ERROR(("mod2dense_forcibly_invert: Matrix to invert is not square\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	if (r == m)
	{
		OF_PRINT_ERROR(("mod2dense_forcibly_invert: Result matrix is the same as the operand\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	n = of_mod2dense_rows (m);
	w = m->n_words;

	if (of_mod2dense_rows (r) != n || of_mod2dense_cols (r) != n)
	{
		OF_PRINT_ERROR(("mod2dense_forcibly_invert: Matrix to receive inverse has wrong dimensions\n"))
		OF_EXIT_FUNCTION
		return -1;
	}

	of_mod2dense_clear (r);
	for (i = 0; i < n; i++)
	{
		of_mod2dense_set (r, i, i, 1);
	}

	for (i = 0; i < n; i++)
	{
		a_row[i] = -1;
		a_col[i] = i;
	}

	for (i = 0; i < n; i++)
	{
		k0 = i >> of_mod2_wordsize_shift;
		b0 = i & of_mod2_wordsize_mask;

		for (j = i; j < n; j++)
		{
			if (of_mod2_getbit (m->col[j][k0], b0))
				break;
		}

		if (j == n)
		{
			j = i;
			of_mod2dense_set (m, i, j, 1);
			a_row[i] = i;
		}

		if (j != i)
		{
			t = m->col[i];
			m->col[i] = m->col[j];
			m->col[j] = t;

			t = r->col[i];
			r->col[i] = r->col[j];
			r->col[j] = t;

			u = a_col[i];
			a_col[i] = a_col[j];
			a_col[j] = u;
		}

		for (j = 0; j < n; j++)
		{
			if (j != i && of_mod2_getbit (m->col[j][k0], b0))
			{
				s = m->col[j];
				t = m->col[i];
				for (k = k0; k < w; k++)
					s[k] ^= t[k];
				s = r->col[j];
				t = r->col[i];
				for (k = 0; k < w; k++)
					s[k] ^= t[k];
			}
		}
	}

	c = 0;
	for (i = 0; i < n; i++)
	{
		if (a_row[i] != -1)
		{
			a_row[c] = a_row[i];
			a_col[c] = a_col[i];
			c += 1;
		}
	}
#endif
	OF_EXIT_FUNCTION
	return c;
}

#endif /* #if 0 } */


double of_mod2dense_density (of_mod2dense *m)
{

	OF_ENTER_FUNCTION
	UINT32 nb_one = 0;
	UINT32 nb_entry = 0;
	UINT32 i, j;
	double density = 0;
#ifdef COL_ORIENTED
	for (j = 0; j < of_mod2dense_cols (m); j++)
	{
		for (i = 0; i < of_mod2dense_rows (m); i++)
		{
			if (of_mod2dense_get (m, i, j))
			{
				nb_one++;
			}
		}
	}
#else
	for (j = 0; j < of_mod2dense_rows (m); j++)
	{
		for (i = 0; i < of_mod2dense_cols (m); i++)
		{
			if (of_mod2dense_get (m, j, i))
			{
				nb_one++;
			}
		}
	}
#endif
	nb_entry = of_mod2dense_rows (m) * of_mod2dense_cols (m);
	density = (double) nb_one / nb_entry;
	//printf("%s: nb_ones=%d tot_nb_entries=%d, density=%f\n", __FUNCTION__, nb_one, nb_entry, density);
	OF_EXIT_FUNCTION
	return density;
}


/* RETURNS TRUE IF THE SPECIFIED ROW IS EMPTY */

bool of_mod2dense_row_is_empty (of_mod2dense	*m,
				UINT32		row)
{
	UINT32		i;
	UINT32		n_w;	// number of of_mod2word in the row
	of_mod2word	*w;	// pointer to the current word of the row

	OF_ENTER_FUNCTION
#ifdef OF_DEBUG
	if (row >= of_mod2dense_rows (m))
	{
		OF_EXIT_FUNCTION
		exit(-1);
	}
#endif
	//n_w = of_mod2dense_cols (m) >> (of_mod2_wordsize_shift);
	n_w = m->n_words;
 	for (i = 0, w = m->row[row]; i < n_w; i++, w++)
	{
		if (*w != 0)
		{
			return false;
		}
	}
	return true;
}


UINT32 of_mod2dense_row_weight (of_mod2dense *m, UINT32 i)
{
	UINT32		weight = 0;

	OF_ENTER_FUNCTION
	if (i >= of_mod2dense_rows (m))
	{
		OF_EXIT_FUNCTION
		return -1;
	}
	//of_mod2dense_print_memory_info(m);
#define SLOW_HAMMING_WEIGHT_CALCULATION
#ifdef  SLOW_HAMMING_WEIGHT_CALCULATION
	UINT32		j;
	UINT32		nwords;			/* number of of_mod2word in a row */
	//UINT32		weight1 = 0;

	nwords = of_mod2dense_cols (m) >> (of_mod2_wordsize_shift);     // number of of_mod2word in a row

 	for (j = 0; j < of_mod2dense_cols(m); j++)
 	{
		if (of_mod2dense_get(m, i, j))
 		{
			weight++;
 		}
 	}
 	//printf("weight method2=%d (standard)\n",weight);
#else
	weight = of_hweight_array ((UINT32 *) m->row[i], of_mod2dense_cols (m));
#endif

	OF_EXIT_FUNCTION
	return weight;
}


UINT32 of_mod2dense_row_weight_ignore_first (of_mod2dense *m, UINT32 i, UINT32 nb_ignore)
{
	UINT32		weight = 0;
	//UINT32	nwords;
	UINT32		start0;

	OF_ENTER_FUNCTION
	if (i >= of_mod2dense_rows (m))
	{
		OF_EXIT_FUNCTION
		return -1;
	}

	//nwords = of_mod2dense_cols (m) >> of_mod2_wordsize_shift;   // number of of_mod2word in a row
	start0 = nb_ignore >> (of_mod2_wordsize_shift - 1);

	UINT32 offset32 = nb_ignore >> of_mod2_wordsize_shift; // number of 32bits words that must be ignored
	UINT32 rem = of_mod2dense_cols (m) - (offset32 << of_mod2_wordsize_shift);     // number of remaining bits to consider

	weight = of_hweight_array ((UINT32 *) & m->row[i][offset32], rem);
	OF_EXIT_FUNCTION
	return weight;
}


UINT32 of_mod2dense_col_weight (of_mod2dense *m, UINT32 i)
{
	OF_ENTER_FUNCTION
	UINT32 weight = 0;
	if (i >= of_mod2dense_cols (m))
	{
		OF_EXIT_FUNCTION
		return -1;
	}
	UINT32 j;
	for (j = 0; j < of_mod2dense_rows (m); j++)
	{
		if (of_mod2dense_get (m, j, i))
		{
			weight++;
		}
	}
	OF_EXIT_FUNCTION
	return weight;

}


void of_mod2dense_print_stats (FILE *f, of_mod2dense *m)
{
	OF_ENTER_FUNCTION
	UINT32		i;
	float		density = 0;
	float		row_density = 0;
	float		col_density = 0;

	/* rows stats*/
	float		aver_nb_entry_per_row = 0;
	UINT32		max_entry_per_row = 0;
	UINT32		min_entry_per_row = 9999999;
	UINT32		nb_entry_row[ROW_MAX_ENTRY];
	UINT32		nb_entry_current_row = 0;
	UINT32		ctr = 0;

	for (i = 0;i < ROW_MAX_ENTRY;i++)
	{
		nb_entry_row[i] = 0;
	}
	for (i = 0; i < of_mod2dense_rows (m); i++)
	{
		nb_entry_current_row = of_mod2dense_row_weight (m, i);
		nb_entry_row[nb_entry_current_row]++;
		if (nb_entry_current_row < min_entry_per_row)
		{
			min_entry_per_row = nb_entry_current_row;
		}
		if (nb_entry_current_row > max_entry_per_row)
		{
			max_entry_per_row = nb_entry_current_row;
		}
		ctr += nb_entry_current_row;
	}
	aver_nb_entry_per_row = ( (float) ctr) / of_mod2dense_rows (m);
	row_density = aver_nb_entry_per_row / (of_mod2dense_cols (m));

	/* cols stats*/
	float		aver_nb_entry_per_col = 0;
	UINT32		max_entry_per_col = 0;
	UINT32		min_entry_per_col = 9999999;
	UINT32		nb_entry_col[COL_MAX_ENTRY];
	UINT32		nb_entry_current_col = 0;

	ctr = 0;
	for (i = 0;i < COL_MAX_ENTRY;i++)
	{
		nb_entry_col[i] = 0;
	}
	for (i = 0; i < of_mod2dense_cols (m); i++)
	{
		nb_entry_current_col = of_mod2dense_col_weight (m, i);
		nb_entry_col[nb_entry_current_col]++;
		if (nb_entry_current_col == 1)
		{
			//fprintf(f," Warning: col %d has degree 1! \n",i);
		}
		if (nb_entry_current_col == 0)
		{
			//fprintf(f," Warning: col %d is empty!! \n",i);
		}
		if (nb_entry_current_col < min_entry_per_col)
		{
			min_entry_per_col = nb_entry_current_col;
		}
		if (nb_entry_current_col > max_entry_per_col)
		{
			max_entry_per_col = nb_entry_current_col;
		}
		ctr += nb_entry_current_col;
	}
	aver_nb_entry_per_col = ( (float) ctr) / of_mod2dense_cols (m);
	col_density = aver_nb_entry_per_col / (of_mod2dense_rows (m));
	density = col_density;

	/* print result */
	fprintf (f, " nb_col=%d  nb_row=%d \n", of_mod2dense_cols (m), of_mod2dense_rows (m));
	fprintf (f, " row_density=%f \n", row_density);

	fprintf (f, " aver_nb_entry_per_row=%f \n", aver_nb_entry_per_row);
	fprintf (f, " min_entry_per_row=%d \n", min_entry_per_row);
	fprintf (f, " max_entry_per_row=%d \n", max_entry_per_row);
	for (i = min_entry_per_row;i <= max_entry_per_row;i++)
	{
		fprintf (f, " nb_entry_row[%d]=%d \n", i, nb_entry_row[i]);
	}
	fprintf (f, "------------------ \n");
	fprintf (f, " col_density=%f \n", col_density);
	fprintf (f, " aver_nb_entry_per_col=%f \n", aver_nb_entry_per_col);
	fprintf (f, " min_entry_per_col=%d \n", min_entry_per_col);
	fprintf (f, " max_entry_per_col=%d \n", max_entry_per_col);
	for (i = min_entry_per_col;i <= max_entry_per_col;i++)
	{
		if (nb_entry_col[i] > 0)
		{
			fprintf (f, " nb_entry_col[%d]=%d \n", i, nb_entry_col[i]);
		}
	}
	fprintf (f, "------------------ \n");
	fprintf (f, " matrix_density=%f \n", density);
	OF_EXIT_FUNCTION
}


void of_mod2dense_print_memory_info (of_mod2dense *m)
{
	OF_ENTER_FUNCTION
	printf ("m=%p\n", m);
	printf ("m->n_rows=%d\n", m->n_rows);
	printf ("m->n_cols=%d\n", m->n_cols);
	printf ("m->n_words=%d\n", m->n_words);
#ifdef COL_ORIENTED

#else
	printf ("m->row=%p\n", m->row);
	printf (" m->row size = %lu\n", m->n_rows*sizeof *m->row);
#endif
	printf ("m->bits=%p\n", m->bits);
	printf (" m->bits size = %lu\n", m->n_words*m->n_rows*sizeof *m->bits);
	printf ("sizeof(mod2word)=%lu\n", sizeof (of_mod2word));
	OF_EXIT_FUNCTION
}


void of_mod2dense_xor_rows(of_mod2dense *m, UINT16 from, UINT16 to)
{
	UINT32		i;
	of_mod2word	*f;
	of_mod2word	*t;

	f = m->row[from];
	t = m->row[to];
	for (i = 0; i < m->n_words; i++)
	{
		t[i] ^= f[i];
	}
}

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
