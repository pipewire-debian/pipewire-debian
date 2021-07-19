/* $Id: of_matrix_sparse.c 206 2014-12-10 04:47:09Z roca $ */
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

#include "../of_linear_binary_code.h"


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

#define ROW_MAX_ENTRY 50
#define COL_MAX_ENTRY 1000

/**
 *
 */
static void		of_mod2sparse_delete_opt (of_mod2sparse *, of_mod2entry *, of_mod2entry **);

/**
 *
 */
static of_mod2entry *	of_mod2sparse_insert_opt (of_mod2sparse *,
						UINT32 ,
						UINT32,
						of_mod2entry **__parsing);


/* ALLOCATE AN ENTRY WITHIN A MATRIX.  This local procedure is used to
   allocate a new entry, representing a non-zero element, within a given
   matrix.  Entries in this matrix that were previously allocated and
   then freed are re-used.  If there are no such entries, a new block
   of entries is allocated. */

static of_mod2entry *of_alloc_entry (of_mod2sparse	*m)
{
	//OF_ENTER_FUNCTION
	of_mod2block *b;
	of_mod2entry *e;
	INT32 k;

	if (m->next_free == 0)
	{
		b = (of_mod2block*) of_calloc (1, sizeof * b);

		b->next = m->blocks;
		m->blocks = b;

		for (k = 0; k < of_mod2sparse_block; k++)
		{
			b->entry[k].left = m->next_free;
			m->next_free = &b->entry[k];
		}
	}
	e = m->next_free;
	m->next_free = e->left;
	//OF_EXIT_FUNCTION
	return e;
}


/* ALLOCATE SPACE FOR A SPARSE MOD2 MATRIX.  */

of_mod2sparse *of_mod2sparse_allocate  (UINT32	n_rows,		/* Number of rows in matrix */
					UINT32	n_cols)		/* Number of columns in matrix */
{
	//OF_ENTER_FUNCTION
	of_mod2sparse *m;
	of_mod2entry *e;
	INT32 i, j;

	if (n_rows <= 0 || n_cols <= 0)
	{
		OF_PRINT_ERROR(("Invalid number of rows (%d) or columns (%d)\nBoth values must be > 0.\n", n_rows, n_cols));
		OF_EXIT_FUNCTION
		return NULL;
	}

	m = (of_mod2sparse*) of_calloc (1, sizeof * m);

	m->n_rows = n_rows;
	m->n_cols = n_cols;
	m->rows = (of_mod2entry*) of_calloc (n_rows, sizeof * m->rows);
	m->cols = (of_mod2entry*) of_calloc (n_cols, sizeof * m->cols);
	m->blocks = 0;
	m->next_free = 0;

	for (i = 0; i < n_rows; i++)
	{
		e = &m->rows[i];
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		e->left = e->right = e->up = e->down = e;
#else
		e->left = e->right = e->down = e;
#endif
		e->row = e->col = -1;
	}

	for (j = 0; j < n_cols; j++)
	{
		e = &m->cols[j];
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		e->left = e->right = e->up = e->down = e;
#else
		e->left = e->right = e->down = e;
#endif
		e->row = e->col = -1;
	}

#ifdef LDPC_QC
	m->exp_factor = 1;           /*default  expansion factor */
#endif
	//OF_EXIT_FUNCTION
	return m;
}

/* FREE SPACE OCCUPIED BY A SPARSE MOD2 MATRIX. */

void of_mod2sparse_free (of_mod2sparse	*m)				/* Matrix to free */
{
	//OF_ENTER_FUNCTION
	of_mod2block *b;

	of_free (m->rows);
	of_free (m->cols);

	while (m->blocks != 0)
	{
		b = m->blocks;
		m->blocks = b->next;
		of_free (b);
	}
	//OF_EXIT_FUNCTION
}


/* CLEAR A SPARSE MATRIX TO ALL ZEROS. */

void of_mod2sparse_clear (of_mod2sparse *r)
{
	of_mod2block *b;
	of_mod2entry *e;
	INT32 i, j;

	OF_ENTER_FUNCTION
	for (i = 0; i < of_mod2sparse_rows (r); i++)
	{
		e = &r->rows[i];
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		e->left = e->right = e->up = e->down = e;
#else
		e->left = e->right = e->down = e;
#endif
	}

	for (j = 0; j < of_mod2sparse_cols (r); j++)
	{
		e = &r->cols[j];
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		e->left = e->right = e->up = e->down = e;
#else
		e->left = e->right = e->down = e;
#endif
	}

	while (r->blocks != 0)
	{
		b = r->blocks;
		r->blocks = b->next;
		free (b);
	}
	OF_EXIT_FUNCTION
}


/* COPY A SPARSE MATRIX. */

void of_mod2sparse_copy (of_mod2sparse	*m,	/* Matrix to copy */
			 of_mod2sparse	*r)		/* Place to store copy of matrix */
{
	OF_ENTER_FUNCTION
	of_mod2entry *e, *f;
	INT32 i;

	if (of_mod2sparse_rows (m) > of_mod2sparse_rows (r)
			|| of_mod2sparse_cols (m) > of_mod2sparse_cols (r))
	{
		OF_PRINT_ERROR(("Destination matrix is too small"));
		return;
	}

	of_mod2sparse_clear (r);

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);

		while (!of_mod2sparse_at_end (e))
		{
			f = of_mod2sparse_insert (r, e->row, e->col);
			e = of_mod2sparse_next_in_row (e);
		}
	}
	OF_EXIT_FUNCTION
}


/* COPY ROWS OF A SPARSE MOD2 MATRIX. */

void of_mod2sparse_copyrows    (of_mod2sparse	*m,	/* Matrix to copy */
				of_mod2sparse	*r,	/* Place to store copy of matrix */
				UINT32		*rows)	/* Indexes of rows to copy, from 0 */
{
	of_mod2entry *e;
	INT32 i;

	OF_ENTER_FUNCTION
	if (of_mod2sparse_cols (m) > of_mod2sparse_cols (r))
	{
		OF_PRINT_ERROR(("Destination matrix has fewer columns than source"));
		OF_EXIT_FUNCTION
		return ;
	}

	of_mod2sparse_clear (r);

	for (i = 0; i < of_mod2sparse_rows (r); i++)
	{
		if (rows[i] >= of_mod2sparse_rows (m))
		{
			OF_PRINT_ERROR(("Row index out of range"));
			OF_EXIT_FUNCTION
			return ;
		}
		e = of_mod2sparse_first_in_row (m, rows[i]);
		while (!of_mod2sparse_at_end (e))
		{
			of_mod2sparse_insert (r, i, e->col);
			e = of_mod2sparse_next_in_row (e);
		}
	}
	OF_EXIT_FUNCTION
}


void of_mod2sparse_copyrows_opt (of_mod2sparse	*m,		/* Matrix to copy */
				 of_mod2sparse	*r,		/* Place to store copy of matrix */
				 UINT32		*rows,		/* Indexes of rows to copy, from 0 */
				 of_mod2entry	**__parsing)
{
	of_mod2entry * __inserted;
	bool __was_null = false;
	of_mod2entry * __e;
	INT32         i;

	OF_ENTER_FUNCTION
	if (of_mod2sparse_cols (m) > of_mod2sparse_cols (r))
	{
		OF_PRINT_ERROR(("Destination matrix has fewer columns than source"));
		OF_EXIT_FUNCTION
		return ;
	}

	if (__parsing == NULL)
	{
		__was_null = true;
		__parsing = (of_mod2entry **) of_calloc (of_mod2sparse_cols (m), sizeof (of_mod2entry *));
	}

	//of_mod2sparse_clear(r);

	for (i = 0 ; i < of_mod2sparse_rows (r) ; i++)
	{
		//if (rows[i] < 0 || rows[i] >= of_mod2sparse_rows (m))
		if (rows[i] >= of_mod2sparse_rows (m))
		{
			OF_PRINT_LVL (1, ("mod2sparse_copyrows_opt: Row index out of range:   rows[i] = %d\n", rows[i]))
			OF_PRINT_ERROR(("Row index out of range"));
			return ;
		}
		__e = of_mod2sparse_first_in_row (m, rows[i]);
		while (!of_mod2sparse_at_end_row (__e))
		{
			__inserted = of_mod2sparse_insert_opt (r, i, of_mod2sparse_col (__e), __parsing);
			if (__was_null == true)
			{
				__parsing[of_mod2sparse_col (__e) ] = __inserted;
			}

			__e = of_mod2sparse_next_in_row (__e);
		}
	}

	if (__was_null == true)
	{
		of_free (__parsing);
		__parsing = NULL;
	}
	OF_EXIT_FUNCTION
} // of_mod2sparse_copyrows_opt(...)


/* COPY COLUMNS OF A SPARSE MOD2 MATRIX. */

void of_mod2sparse_copycols    (of_mod2sparse	*m,		/* Matrix to copy */
				of_mod2sparse	*r,		/* Place to store copy of matrix */
				UINT32		*cols)		/* Indexes of columns to copy, from 0 */
{
	of_mod2entry *e;
	INT32 j;

	OF_ENTER_FUNCTION
	if (of_mod2sparse_rows (m) > of_mod2sparse_rows (r))
	{
		OF_PRINT_ERROR(("Destination matrix has fewer rows than source"));
		OF_EXIT_FUNCTION
		return ;
	}

	of_mod2sparse_clear (r);

	for (j = 0; j < of_mod2sparse_cols (r); j++)
	{
		//if (cols[j] < 0 || cols[j] >= of_mod2sparse_cols (m))
		if (cols[j] >= of_mod2sparse_cols (m))
		{
			OF_PRINT_ERROR(("Column index out of range"));
			OF_PRINT_LVL (1, (" mod2sparse_copycols: Column index out of range cols[j] = %d\n", cols[j]))
			OF_EXIT_FUNCTION
			return ;
		}
		e = of_mod2sparse_first_in_col (m, cols[j]);
		while (!of_mod2sparse_at_end (e))
		{
			of_mod2sparse_insert (r, e->row, j);
			e = of_mod2sparse_next_in_col (e);
		}
	}
	OF_EXIT_FUNCTION
}


void of_mod2sparse_copycols_opt (of_mod2sparse	*m,	/* Matrix to copy */
				 of_mod2sparse	*r,	/* Place to store copy of matrix */
				 UINT32		*cols)	/* Indexes of columns to copy, from 0 */
{
	of_mod2entry ** __parsing;
	of_mod2entry * __inserted;
	of_mod2entry * __e;
	INT32         j;

	OF_ENTER_FUNCTION
	if (of_mod2sparse_rows (m) > of_mod2sparse_rows (r))
	{
		OF_PRINT_ERROR(("Destination matrix has fewer rows than source"));
		OF_EXIT_FUNCTION
		return ;
	}

	//of_mod2sparse_clear(r);
	__parsing = (of_mod2entry **) of_calloc (of_mod2sparse_cols (r), sizeof (of_mod2entry *));

	for (j = 0 ; j < of_mod2sparse_cols (r) ; j++)
	{
		//if (cols[j] < 0 || cols[j] >= of_mod2sparse_cols (m))
		if (cols[j] >= of_mod2sparse_cols (m))
		{
			OF_PRINT_ERROR(("Column index out of range"));
			OF_PRINT_LVL (1, (" mod2sparse_copycols: Column index out of range cols[j] = %d\n", cols[j]))
			OF_EXIT_FUNCTION
			return ;
		}

		__e = of_mod2sparse_first_in_col (m, cols[j]);
		while (!of_mod2sparse_at_end_col (__e))
		{
			__inserted = of_mod2sparse_insert_opt (r, of_mod2sparse_row (__e), j, __parsing);
			__parsing[j] = __inserted;
			__e = of_mod2sparse_next_in_col (__e);
		}
	}

	of_free (__parsing);
	__parsing = NULL;
	OF_EXIT_FUNCTION
} // of_mod2sparse_copycols_opt(...)




/* PRINT matrix statistics: average number of 1's per row/line etc ...*/
void of_mod2sparse_matrix_stats (FILE		*f,
				 of_mod2sparse	*m,
				 UINT32		nb_src ,
				 UINT32		nb_par)
{
	of_mod2entry	*e;
	INT32		i;
	float		density = 0;
	float		row_density = 0;
	float		col_density = 0;

	/* rows stats*/
	float		aver_nb_entry_per_row = 0;
	INT32		max_entry_per_row = 0;
	INT32		min_entry_per_row = 9999999;
	INT32		nb_entry_row[ROW_MAX_ENTRY];
	INT32		nb_entry_current_row = 0;
	INT32		ctr = 0;

	/* cols stats*/
	float		aver_nb_entry_per_col = 0;
	INT32		max_entry_per_col = 0;
	INT32		min_entry_per_col = 9999999;
	INT32		nb_entry_col[COL_MAX_ENTRY];
	INT32		max_entry_per_data_col = 0;
	INT32		min_entry_per_data_col = 9999999;
	INT32		max_entry_per_parity_col = 0;
	INT32		min_entry_per_parity_col = 9999999;
	INT32		nb_entry_data_col[COL_MAX_ENTRY];
	INT32		nb_entry_parity_col[COL_MAX_ENTRY];
	INT32		nb_entry_current_col = 0;

	OF_ENTER_FUNCTION
	/* rows stats*/
	memset(nb_entry_row, 0, sizeof(*nb_entry_row));

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);
		while (!of_mod2sparse_at_end (e))
		{
			nb_entry_current_row++;
			e = of_mod2sparse_next_in_row (e);
		}

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
		nb_entry_current_row = 0;
	}

	aver_nb_entry_per_row = ( (float) ctr) / of_mod2sparse_rows (m);
	row_density = aver_nb_entry_per_row / (of_mod2sparse_cols (m));

	/* cols stats*/
	ctr = 0;
	memset(nb_entry_col, 0, sizeof(*nb_entry_col));
	memset(nb_entry_data_col, 0, sizeof(*nb_entry_data_col));
	memset(nb_entry_parity_col, 0, sizeof(*nb_entry_parity_col));

	for (i = 0; i < of_mod2sparse_cols (m); i++)
	{
		e = of_mod2sparse_first_in_col (m, i);
		while (!of_mod2sparse_at_end (e))
		{
			nb_entry_current_col++;
			e = of_mod2sparse_next_in_col (e);
		}

		nb_entry_col[nb_entry_current_col]++;
		if (nb_entry_current_col == 1)
		{
			//fprintf(f," Warning: col %d has degree 1! \n",i);
		}
		if (nb_entry_current_col == 0)
		{
			//fprintf(f," Warning: col %d is empty!! \n",i);
		}
		if (i < nb_par)
		{
			// this is a parity column

			if (nb_entry_current_col < min_entry_per_parity_col)
			{
				min_entry_per_parity_col = nb_entry_current_col;
			}
			if (nb_entry_current_col > max_entry_per_parity_col)
			{
				max_entry_per_parity_col = nb_entry_current_col;
			}
			nb_entry_parity_col[nb_entry_current_col]++;
		}
		else
		{
			// this is a data column
			if (nb_entry_current_col < min_entry_per_data_col)
			{
				min_entry_per_data_col = nb_entry_current_col;
			}
			if (nb_entry_current_col > max_entry_per_data_col)
			{
				max_entry_per_data_col = nb_entry_current_col;
			}
			nb_entry_data_col[nb_entry_current_col]++;
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
		nb_entry_current_col = 0;
	}

	aver_nb_entry_per_col = ( (float) ctr) / of_mod2sparse_cols (m);

	col_density = aver_nb_entry_per_col / (of_mod2sparse_rows (m));

	density = col_density;

	/* print result */

	fprintf (f, " nb_col=%d  nb_row=%d \n", of_mod2sparse_cols (m), of_mod2sparse_rows (m));
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
	fprintf (f, "------- \n");
	fprintf (f, " min_entry_per_data_col=%d \n", min_entry_per_data_col);
	fprintf (f, " max_entry_per_data_col=%d \n", max_entry_per_data_col);
	for (i = min_entry_per_data_col;i <= max_entry_per_data_col;i++)
	{
		if (nb_entry_data_col[i] > 0)
		{
			fprintf (f, " nb_entry_data_col[%d]=%d \n", i, nb_entry_data_col[i]);
		}
	}
	fprintf (f, "------- \n");

	fprintf (f, " min_entry_per_parity_col=%d \n", min_entry_per_parity_col);
	fprintf (f, " max_entry_per_parity_col=%d \n", max_entry_per_parity_col);
	for (i = min_entry_per_parity_col;i <= max_entry_per_parity_col;i++)
	{
		if (nb_entry_parity_col[i] > 0)
		{
			fprintf (f, " nb_entry_parity_col[%d]=%d \n", i, nb_entry_parity_col[i]);
		}
	}
	fprintf (f, "------------------ \n");

	fprintf (f, " matrix_density=%f \n", density);
	OF_EXIT_FUNCTION
}


void of_mod2sparse_printf (FILE * fout,
			   of_mod2sparse * m)
{
	INT32 __row;

	OF_ENTER_FUNCTION
	for (__row = 0 ; __row < of_mod2sparse_rows (m) ; __row++)
	{
		INT32 __col;
		for (__col = 0 ; __col < of_mod2sparse_cols (m) ; __col++)
		{
			if (of_mod2sparse_find (m, __row, __col) != NULL)
				fprintf (fout, "1");
			else
				//fprintf (fout, "0");
				fprintf (fout, " ");
		}
		fprintf (fout, "\n");
		OF_EXIT_FUNCTION
	}
} // of_mod2sparse_printf(...)


void of_mod2sparse_print_bitmap (of_mod2sparse * m)
{
	UINT32		i;
	UINT32		x;
	UINT32		y;
	UINT32		row;
	UINT32		col;
	UINT32		val = 1;
	of_mod2entry	*e;
#ifdef IL_SUPPORT
	ILuint	lImage;	

	ilInit();
	ilGenImages(1, &lImage);
	ilBindImage(lImage);
	//of_mod2dense_print_stats(stdout,m);
	ilTexImage(of_mod2sparse_cols(m), of_mod2sparse_rows(m), 1, 3, IL_RGB, IL_UNSIGNED_BYTE, NULL);
#endif

	OF_ENTER_FUNCTION
	y = of_mod2dense_rows (m);
	x = of_mod2dense_cols (m);
	for (row = 0; row < of_mod2sparse_rows(m); row++)
	{
		for (col = 0; col < of_mod2sparse_cols(m); col++)
		{
			val	= 0;			
			x	= col;
			y	= of_mod2sparse_rows(m) - row - 1;
#ifdef IL_SUPPORT	
			ilSetPixels(x, y, 0, 1, 1, 1, IL_RGBA, IL_UNSIGNED_BYTE, &val);
#endif
		}
	}
	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);
		while (!of_mod2sparse_at_end_row (e))
		{
			val	= 0xFFFFFF;
			row	= of_mod2sparse_row(e);
			col	= of_mod2sparse_col(e);
			x	= col;
			y	= of_mod2sparse_rows(m) - row - 1;
#ifdef IL_SUPPORT	
			ilSetPixels(x ,y, 0, 1, 1, 1, IL_RGBA, IL_UNSIGNED_BYTE, &val);
#endif	
			e = of_mod2sparse_next_in_row(e);
		}
	}

	//char buffer[100];

#ifdef	IL_SUPPORT	
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage("sparse.bmp");
	// use EOG (Eye of Gnome) in Linux machines. Use something else if needed on other OS.
	system("open sparse.bmp");
#endif

	OF_EXIT_FUNCTION
} // of_mod2sparse_print_bitmap(...)


/* PRINT A SPARSE MOD2 MATRIX IN HUMAN-READABLE FORM. */

void of_mod2sparse_print
(FILE *f,
 of_mod2sparse *m
)
{
	OF_ENTER_FUNCTION
	INT32 rdigits, cdigits;
	of_mod2entry *e;
	INT32 i;

	rdigits = of_mod2sparse_rows (m) <= 10 ? 1
		  : of_mod2sparse_rows (m) <= 100 ? 2
		  : of_mod2sparse_rows (m) <= 1000 ? 3
		  : of_mod2sparse_rows (m) <= 10000 ? 4
		  : of_mod2sparse_rows (m) <= 100000 ? 5
		  : 6;

	cdigits = of_mod2sparse_cols (m) <= 10 ? 1
		  : of_mod2sparse_cols (m) <= 100 ? 2
		  : of_mod2sparse_cols (m) <= 1000 ? 3
		  : of_mod2sparse_cols (m) <= 10000 ? 4
		  : of_mod2sparse_cols (m) <= 100000 ? 5
		  : 6;

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		fprintf (f, "%*d:", rdigits, i);

		e = of_mod2sparse_first_in_row (m, i);
		while (!of_mod2sparse_at_end (e))
		{
			fprintf (f, " %*d", cdigits, of_mod2sparse_col (e));
			e = of_mod2sparse_next_in_row (e);
		}

		fprintf (f, "\n");
	}
	OF_EXIT_FUNCTION
}


#if 0
/* WRITE A SPARSE MOD2 MATRIX TO A FILE IN MACHINE-READABLE FORM. */

INT32 mod2sparse_write
(FILE *f,
 of_mod2sparse *m
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	INT32 i;

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

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);

		if (!of_mod2sparse_at_end (e))
		{
			of_intio_write (f, - (i + 1));
			if (ferror (f))
			{
				OF_EXIT_FUNCTION
				return 0;
			}

			while (!of_mod2sparse_at_end (e))
			{
				of_intio_write (f, of_mod2sparse_col (e) + 1);
				if (ferror (f))
				{
					OF_EXIT_FUNCTION
					return 0;
				}

				e = of_mod2sparse_next_in_row (e);
			}
		}
	}

	of_intio_write (f, 0);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	OF_EXIT_FUNCTION
	return 1;
}


/* READ A SPARSE MOD2 MATRIX STORED IN MACHINE-READABLE FORM FROM A FILE. */

of_mod2sparse *mod2sparse_read
(FILE *f
)
{
	OF_ENTER_FUNCTION
	INT32 n_rows, n_cols;
	of_mod2sparse *m;
	INT32 v, row, col;

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

	m = of_mod2sparse_allocate (n_rows, n_cols);

	row = -1;

	for (;;)
	{
		v = of_intio_read (f);
		if (feof (f) || ferror (f))
			break;

		if (v == 0)
		{
			OF_EXIT_FUNCTION
			return m;
		}
		else
			if (v < 0)
			{
				row = -v - 1;
				if (row >= n_rows)
					break;
			}
			else
			{
				col = v - 1;
				if (col >= n_cols)
					break;
				if (row == -1)
					break;
				of_mod2sparse_insert (m, row, col);
			}
	}

	/* Error if we get here. */

	of_mod2sparse_free (m);
	OF_EXIT_FUNCTION
	return 0;
}
#endif /* 0 */


/* WRITE A SPARSE MOD2 MATRIX TO A FILE IN HUMAN-READABLE FORM. */

UINT32 of_mod2sparse_write_human_readable (FILE			*f,
					   of_mod2sparse	*m,
					   UINT32		nb_source,
					   UINT32		nb_parity)
{
	of_mod2entry *e;
	INT32 i, col_idx, c;
	char buffer [20];
	INT32 str_length;

	OF_ENTER_FUNCTION
	memset (buffer, 0, 5);

	str_length = sprintf (buffer, " %d\n", m->n_rows);
	fwrite (buffer, str_length, 1, f);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	str_length = sprintf (buffer, " %d\n", m->n_cols);
	fwrite (buffer, str_length, 1, f);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	str_length = sprintf (buffer, " %d\n", nb_source);
	fwrite (buffer, str_length, 1, f);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	str_length = sprintf (buffer, " %d", nb_parity);
	fwrite (buffer, str_length, 1, f);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);
		if (!of_mod2sparse_at_end (e))
		{
			str_length = sprintf (buffer, "\n %5d", i);
			fwrite (buffer, str_length, 1, f);
			if (ferror (f))
			{
				OF_EXIT_FUNCTION
				return 0;
			}
			// first the source symbols
			while (!of_mod2sparse_at_end (e))
			{
				c = of_mod2sparse_col (e);
				if (c >= nb_parity)
				{
					col_idx = c - nb_parity;
					str_length = sprintf (buffer, "  %7d ", col_idx);
					fwrite (buffer, str_length, 1, f);
					if (ferror (f))
					{
						OF_EXIT_FUNCTION
						return 0;
					}
				}
				e = of_mod2sparse_next_in_row (e);
			}
			// then the parity symbols
			e = of_mod2sparse_first_in_row (m, i);
			while (!of_mod2sparse_at_end (e))
			{
				c = of_mod2sparse_col (e);
				if (c < nb_parity)
				{
					col_idx = nb_source + c;
					str_length = sprintf (buffer, "  %7d ", col_idx);
					fwrite (buffer, str_length, 1, f);
					if (ferror (f))
					{
						OF_EXIT_FUNCTION
						return 0;
					}
				}
				e = of_mod2sparse_next_in_row (e);
			}
		}
	}
	//of_intio_write(f,0);
	if (ferror (f))
	{
		OF_EXIT_FUNCTION
		return 0;
	}
	OF_EXIT_FUNCTION
	return 1;
}


/* READ A SPARSE MOD2 MATRIX STORED IN HUMAN-READABLE FORM FROM A FILE. */
/*
 * Format of the file:
 *   number_of_line
 *   number_of_column
 *   line_number  element_column_number ...  element_column_number
 *    .
 *    .
 *    .
 *   line_number  element_column_number ...  element_column_number
 *
 */

of_mod2sparse *of_mod2sparse_read_human_readable (FILE		*f,
						  UINT32	*nb_source,
						  UINT32	*nb_parity)
{
	UINT32		n_rows = 0;
	UINT32		n_cols = 0;
	of_mod2sparse	*m;
	INT32		col;
	INT32		curent_line;
	INT32		tmp;
	char		*pch;
	char		line[1024];

	OF_ENTER_FUNCTION
	// get the number of row of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		n_rows = atoi (pch);
		//printf("nrows = %d\n",n_rows);
	}
	// get the number of columns of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		n_cols = atoi (pch);
		//printf("ncol = %d\n",n_cols);
	}
	// get the number of src symbols of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		*nb_source = atoi (pch);
		//printf("nrows = %d\n",n_rows);
	}
	// get the number of parity symbols of the matrix
	if (fgets (line, sizeof line, f) != NULL)
	{
		//size_t i = strspn ( line, " \t\n\v" );
		pch = strtok (line, "   ");
		*nb_parity = atoi (pch);
		//printf("ncol = %d\n",n_cols);
	}
	OF_TRACE_LVL (1, ("nrows= %d  ncol = %d  nb_src= %d  nb_par= %d \n", n_rows, n_cols, *nb_source, *nb_parity))
	if ( (*nb_parity + *nb_source < n_cols) || (*nb_parity < n_rows))
	{

		OF_PRINT_ERROR(("error read file  invalid nb_parity nb_source "));
		OF_EXIT_FUNCTION
		return NULL;
	}

	// allocate the matrix
	m = of_mod2sparse_allocate (n_rows, n_cols);

	curent_line = 0;
	// process the file line by line
	while (fgets (line, sizeof line, f) != NULL)
	{
		size_t i = strspn (line, " \t\n\v");

		if (line[i] == '#')
			continue;
		//printf("line : %s \n",line);

		// get the first token: should be the line number
		pch = strtok (line, "   ");
		//printf ("%s\n",pch);
		// get the first token of the line: the line number
		tmp = atoi (pch);

		if (tmp != curent_line)
		{
			fprintf (stderr, "Error reading file: reading line %d should be %d\n", tmp, curent_line);
			OF_EXIT_FUNCTION
			return NULL;
		}
		// get the following, the colunm numbers of the non-zero entries
		// of the matrix line
		pch = strtok (NULL, "   ");
		while (pch != NULL)
		{
			if (strcmp (pch, "\n") != 0)
			{
				//printf ("pcg = \"%s\"\n",pch);
				// get the column number of the new element
				tmp = atoi (pch);
				//printf("inserting entry : (%d,%d)\n",curent_line ,tmp);
				// insert the new element
				if (tmp < *nb_source)
				{
					col = tmp + *nb_parity;
				}
				else
				{
					col = tmp - *nb_source;
				}
				of_mod2sparse_insert (m, curent_line, col);
				//of_mod2entry e = m->cols[col];
			}
			else
			{
				//printf("skipping pch : it is a EOL\n");
			}
			pch = strtok (NULL, "   ");
		}
		curent_line++;
	}

	OF_EXIT_FUNCTION
	return m;
}


/* LOOK FOR AN ENTRY WITH GIVEN ROW AND COLUMN. */

of_mod2entry *of_mod2sparse_find (of_mod2sparse		*m,
				  UINT32		row,
				  UINT32		col)
{
	//OF_ENTER_FUNCTION
	of_mod2entry *re, *ce;

	//if (row < 0 || row >= of_mod2sparse_rows (m) || col < 0 || col >= of_mod2sparse_cols (m))
	if (row >= of_mod2sparse_rows (m) || col >= of_mod2sparse_cols (m))
	{
		fprintf (stderr, "mod2sparse_find: row or column index out of bounds\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	/* Check last entries in row. */

	re = of_mod2sparse_last_in_row (m, row);
	if (of_mod2sparse_at_end (re) || of_mod2sparse_col (re) < col)
	{
		//OF_EXIT_FUNCTION
		return 0;
	}
	if (of_mod2sparse_col (re) == col)
	{
		//OF_EXIT_FUNCTION
		return re;
	}

#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	ce = of_mod2sparse_last_in_col (m, col);
	if (of_mod2sparse_at_end (ce) || of_mod2sparse_row (ce) < row)
	{
		//OF_EXIT_FUNCTION
		return 0;
	}
	if (of_mod2sparse_row (ce) == row)
	{
		//OF_EXIT_FUNCTION
		return ce;
	}
#endif

	/* Search row and column in parallel, from the front. */

	re = of_mod2sparse_first_in_row (m, row);
	ce = of_mod2sparse_first_in_col (m, col);

	for (;;)
	{
		if (of_mod2sparse_at_end (re) || of_mod2sparse_col (re) > col)
		{
			//OF_EXIT_FUNCTION
			return 0;
		}
		if (of_mod2sparse_col (re) == col)
		{
			//OF_EXIT_FUNCTION
			return re;
		}

		if (of_mod2sparse_at_end (ce) || of_mod2sparse_row (ce) > row)
		{
			//OF_EXIT_FUNCTION
			return 0;
		}
		if (of_mod2sparse_row (ce) == row)
		{
			//OF_EXIT_FUNCTION
			return ce;
		}
		re = of_mod2sparse_next_in_row (re);
		ce = of_mod2sparse_next_in_col (ce);
	}
	//OF_EXIT_FUNCTION
}


/* INSERT AN ENTRY WITH GIVEN ROW AND COLUMN. */

of_mod2entry *of_mod2sparse_insert (of_mod2sparse	*m,
				    UINT32		row,
				    UINT32		col)
{
	//OF_ENTER_FUNCTION
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	of_mod2entry *re, *ce, *ne;
#else
	of_mod2entry *re, *ce, *ne, *ce2;
#endif

	//if (row < 0 || row >= of_mod2sparse_rows (m) || col < 0 || col >= of_mod2sparse_cols (m))
	if (row >= of_mod2sparse_rows (m) || col >= of_mod2sparse_cols (m))
	{
		fprintf (stderr, "mod2sparse_insert: row or column index out of bounds\n");
		OF_EXIT_FUNCTION
		return NULL;
	}
	//printf("inserting %d %d \n",row,col);
	/* Find old entry and return it, or allocate new entry and insert into row. */

	re = of_mod2sparse_last_in_row (m, row);

	if (!of_mod2sparse_at_end (re) && of_mod2sparse_col (re) == col)
	{
		OF_EXIT_FUNCTION
		return re;
	}

	if (of_mod2sparse_at_end (re) || of_mod2sparse_col (re) < col)
	{
		re = re->right;
	}
	else
	{
		re = of_mod2sparse_first_in_row (m, row);
		for (;;)
		{
			if (!of_mod2sparse_at_end (re) && of_mod2sparse_col (re) == col)
			{
				OF_EXIT_FUNCTION
				return re;
			}
			if (of_mod2sparse_at_end (re) || of_mod2sparse_col (re) > col)
			{
				break;
			}
			re = of_mod2sparse_next_in_row (re);
		}
	}

	ne = of_alloc_entry (m);

	ne->row = row;
	ne->col = col;

	ne->left = re->left;
	ne->right = re;
	ne->left->right = ne;
	ne->right->left = ne;

	/* Insert new entry into column. */

#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	/* If we find an existing entry here,
	the matrix must be garbled, since we didn't find it in the row. */

	ce = of_mod2sparse_last_in_col (m, col);

	if (!of_mod2sparse_at_end (ce) && of_mod2sparse_row (ce) == row)
	{
		fprintf (stderr, "mod2sparse_insert: Garbled matrix\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (of_mod2sparse_at_end (ce) || of_mod2sparse_row (ce) < row)
	{
		ce = ce->down;
	}
	else
	{
#else
	ce2 = & (m->cols[col]);
#endif
		ce = of_mod2sparse_first_in_col (m, col);

		for (;;)
		{
			if (!of_mod2sparse_at_end (ce) && of_mod2sparse_row (ce) == row)
			{
				fprintf (stderr, "mod2sparse_insert: Garbled matrix\n");
				OF_EXIT_FUNCTION
				return NULL;
			}

			if (of_mod2sparse_at_end (ce) || of_mod2sparse_row (ce) > row)
			{
				break;
			}
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			ce2 = ce;
#endif
			ce = of_mod2sparse_next_in_col (ce);
		}

#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	}

	ne->up = ce->up;
	ne->down = ce;
	ne->up->down = ne;
	ne->down->up = ne;
#else
	ne->down = ce;
	ce2->down = ne;
#endif

	/* Return the new entry. */
	//OF_EXIT_FUNCTION
	return ne;
}


static of_mod2entry * of_mod2sparse_insert_opt (of_mod2sparse	*m,
						UINT32		row,
						UINT32		col,
						of_mod2entry	**__parsing)
{
	of_mod2entry * __right_entry, * __col_entry, * __new_entry;
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	of_mod2entry * __col_entry_;
#endif

	OF_ENTER_FUNCTION
	//if (row < 0 || row >= of_mod2sparse_rows (m) || col < 0 || col >= of_mod2sparse_cols (m))
	if (row >= of_mod2sparse_rows (m) || col >= of_mod2sparse_cols (m))
	{
		fprintf (stderr, "mod2sparse_insert: row or column index out of bounds\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	/* Find old entry and return it, or allocate new entry and insert into row. */

	__right_entry = of_mod2sparse_last_in_row (m, row);

	if (!of_mod2sparse_at_end (__right_entry) && of_mod2sparse_col (__right_entry) == col)
	{
		OF_EXIT_FUNCTION
		return __right_entry;
	}

	if (of_mod2sparse_at_end (__right_entry) || of_mod2sparse_col (__right_entry) < col)
	{
		__right_entry = __right_entry->right;
	}
	else
	{
		__right_entry = of_mod2sparse_first_in_row (m, row);

		for (;;)
		{
			if (!of_mod2sparse_at_end (__right_entry) && of_mod2sparse_col (__right_entry) == col)
			{
				OF_EXIT_FUNCTION
				return __right_entry;
			}

			if (of_mod2sparse_at_end (__right_entry) || of_mod2sparse_col (__right_entry) > col)
			{
				break;
			}

			__right_entry = of_mod2sparse_next_in_row (__right_entry);
		}
	}

	__new_entry = of_alloc_entry (m);

	__new_entry->row = row;
	__new_entry->col = col;

	__new_entry->left = __right_entry->left;
	__new_entry->right = __right_entry;
	__new_entry->left->right = __new_entry;
	__new_entry->right->left = __new_entry;

	/* Insert new entry into column. */
	if (__parsing != NULL)
	{
		if (__parsing[of_mod2sparse_col (__new_entry) ] != NULL)
		{
			__col_entry = __parsing[of_mod2sparse_col (__new_entry) ];
		}
		else
		{
			__col_entry = of_mod2sparse_first_in_col (m, of_mod2sparse_col (__new_entry));
		}
	}
	else
	{
		__col_entry = of_mod2sparse_first_in_col (m, of_mod2sparse_col (__new_entry));
	} // if (__parsing != NULL) ...

#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	__col_entry_ = __col_entry;
#endif // #ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	while (!of_mod2sparse_at_end_col (__col_entry) && of_mod2sparse_row (__col_entry) < row)
	{
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		__col_entry_ = __col_entry;
#endif // #ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		__col_entry = of_mod2sparse_next_in_col (__col_entry);
	} // while (!of_mod2sparse_at_end_col(__col_entry) && of_mod2sparse_row(__col_entry) <= row) ...

	__new_entry->down = __col_entry;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	__new_entry->up = __col_entry->up;
	__new_entry->up->down = __new_entry;
	__new_entry->down->up = __new_entry;
#else // #ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	__col_entry_->down = __new_entry;
#endif // #ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE ... #else ...

	OF_EXIT_FUNCTION
	/* Return the new entry. */
	return __new_entry;
}


/* DELETE AN ENTRY FROM A SPARSE MATRIX. */
void of_mod2sparse_delete (of_mod2sparse	*m,
			   of_mod2entry		*e)
{
	OF_ENTER_FUNCTION
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	of_mod2entry *ce;
#endif

	if (e == 0)
	{
		fprintf (stderr, "mod2sparse_delete: Trying to delete a null entry\n");
		OF_EXIT_FUNCTION
		return;
	}
	if ((of_mod2sparse_row (e) < 0) || (of_mod2sparse_col (e) < 0))
	{
		fprintf (stderr, "mod2sparse_delete: Trying to delete a header entry (row=%d, col=%d)\n", e->row, e->col);
		OF_EXIT_FUNCTION
		return ;
	}
#ifdef OF_DEBUG
	/* Sanity: check that the entry is in the list before trying to remove it.
	 * Define this check only if in doubt since this is very computing intensive! */
	of_mod2entry *tmp_e;
	for (tmp_e = of_mod2sparse_first_in_col(m, of_mod2sparse_col(e)); tmp_e != e; tmp_e = tmp_e->down)
	{
		if (of_mod2sparse_at_end_col(tmp_e))
		{
			fprintf (stderr, "mod2sparse_delete: error, entry %d/%d not found in column\n",
				e->row, e->col);
			ASSERT(0);
			OF_EXIT_FUNCTION
			return;
		}
	}
#endif
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	e->up->down = e->down;
	e->down->up = e->up;
#else	
	ce = & (m->cols[of_mod2sparse_col(e)]);
	for (; ce->down != e; ce = ce->down);	/* find the entry before the one to delete */
	ce->down = e->down;
#endif

	e->left->right = e->right;
	e->right->left = e->left;

	e->left = m->next_free;
	m->next_free = e;
	OF_EXIT_FUNCTION
}


void of_mod2sparse_delete_opt (of_mod2sparse	* m,
			       of_mod2entry	* e,
			       of_mod2entry	** __parsing)
{
	OF_ENTER_FUNCTION
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	of_mod2entry * ce;
#endif

	if (e == NULL)
	{
		fprintf (stderr, "mod2sparse_delete: Trying to delete a null entry\n");
		OF_EXIT_FUNCTION
		return ;
	}
	if ((of_mod2sparse_row (e) < 0) || (of_mod2sparse_col (e) < 0))
	{
		fprintf (stderr, "mod2sparse_delete: Trying to delete a header entry (row=%d, col=%d)\n", e->row, e->col);
		OF_EXIT_FUNCTION
		return ;
	}
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	e->up->down = e->down;
	e->down->up = e->up;
#else
	if (__parsing != NULL)
	{
		ce = __parsing[of_mod2sparse_col (e) ];
	}
	else
	{
		ce = & (m->cols[of_mod2sparse_col (e) ]);
	}
	while (!of_mod2sparse_at_end_col (ce))
	{
		ce = of_mod2sparse_next_in_col (ce);
	}
	ce->down = e->down;
#endif

	e->left->right = e->right;
	e->right->left = e->left;

	e->left = m->next_free;
	m->next_free = e;
	OF_EXIT_FUNCTION
}


#if 0

/* TEST WHETHER TWO SPARSE MATRICES ARE EQUAL. */

INT32 mod2sparse_equal
(of_mod2sparse *m1,
 of_mod2sparse *m2
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e1, *e2;
	INT32 i;

	if (of_mod2sparse_rows (m1) != of_mod2sparse_rows (m2)
			|| of_mod2sparse_cols (m1) != of_mod2sparse_cols (m2))
	{
		fprintf (stderr, "mod2sparse_equal: Matrices have different dimensions\n");
		OF_EXIT_FUNCTION
		return 0;
	}

	for (i = 0; i < of_mod2sparse_rows (m1); i++)
	{
		e1 = of_mod2sparse_first_in_row (m1, i);
		e2 = of_mod2sparse_first_in_row (m2, i);

		while (!of_mod2sparse_at_end (e1) && !of_mod2sparse_at_end (e2))
		{
			if (of_mod2sparse_col (e1) != of_mod2sparse_col (e2))
			{
				OF_EXIT_FUNCTION
				return 0;
			}

			e1 = of_mod2sparse_next_in_row (e1);
			e2 = of_mod2sparse_next_in_row (e2);
		}

		if (!of_mod2sparse_at_end (e1) || !of_mod2sparse_at_end (e2))
		{
			OF_EXIT_FUNCTION
			return 0;
		}
	}

	OF_EXIT_FUNCTION
	return 1;
}


/* COMPUTE THE TRANSPOSE OF A SPARSE MOD2 MATRIX. */

void mod2sparse_transpose
(of_mod2sparse *m,	/* Matrix to compute transpose of (left unchanged) */
 of_mod2sparse *r		/* Result of transpose operation */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	INT32 i;

	if (of_mod2sparse_rows (m) != of_mod2sparse_cols (r)
			|| of_mod2sparse_cols (m) != of_mod2sparse_rows (r))
	{
		fprintf (stderr,
			 "mod2sparse_transpose: Matrices have incompatible dimensions\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (r == m)
	{
		fprintf (stderr,
			 "mod2sparse_transpose: Result matrix is the same as the operand\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	of_mod2sparse_clear (r);

	for (i = 0; i < of_mod2sparse_rows (m); i++)
	{
		e = of_mod2sparse_first_in_row (m, i);

		while (!of_mod2sparse_at_end (e))
		{
			of_mod2sparse_insert (r, of_mod2sparse_col (e), i);
			e = of_mod2sparse_next_in_row (e);
		}
	}
	OF_EXIT_FUNCTION
}


/* ADD TWO SPARSE MOD2 MATRICES. */

void mod2sparse_add
(of_mod2sparse *m1,	/* Left operand of add */
 of_mod2sparse *m2,	/* Right operand of add */
 of_mod2sparse *r		/* Place to store result of add */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e1, *e2;
	INT32 i;

	if (of_mod2sparse_rows (m1) != of_mod2sparse_rows (r)
			|| of_mod2sparse_cols (m1) != of_mod2sparse_cols (r)
			|| of_mod2sparse_rows (m2) != of_mod2sparse_rows (r)
			|| of_mod2sparse_cols (m2) != of_mod2sparse_cols (r))
	{
		fprintf (stderr, "mod2sparse_add: Matrices have different dimensions\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (r == m1 || r == m2)
	{
		fprintf (stderr,
			 "mod2sparse_add: Result matrix is the same as one of the operands\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	of_mod2sparse_clear (r);

	for (i = 0; i < of_mod2sparse_rows (r); i++)
	{
		e1 = of_mod2sparse_first_in_row (m1, i);
		e2 = of_mod2sparse_first_in_row (m2, i);

		while (!of_mod2sparse_at_end (e1) && !of_mod2sparse_at_end (e2))
		{
			if (of_mod2sparse_col (e1) == of_mod2sparse_col (e2))
			{
				e1 = of_mod2sparse_next_in_row (e1);
				e2 = of_mod2sparse_next_in_row (e2);
			}

			else
				if (of_mod2sparse_col (e1) < of_mod2sparse_col (e2))
				{
					of_mod2sparse_insert (r, i, of_mod2sparse_col (e1));
					e1 = of_mod2sparse_next_in_row (e1);
				}

				else
				{
					of_mod2sparse_insert (r, i, of_mod2sparse_col (e2));
					e2 = of_mod2sparse_next_in_row (e2);
				}
		}

		while (!of_mod2sparse_at_end (e1))
		{
			of_mod2sparse_insert (r, i, of_mod2sparse_col (e1));
			e1 = of_mod2sparse_next_in_row (e1);
		}

		while (!of_mod2sparse_at_end (e2))
		{
			of_mod2sparse_insert (r, i, of_mod2sparse_col (e2));
			e2 = of_mod2sparse_next_in_row (e2);
		}
	}
	OF_EXIT_FUNCTION
}


/* MULTIPLY TWO SPARSE MOD2 MATRICES. */

void mod2sparse_multiply
(of_mod2sparse *m1, 	/* Left operand of multiply */
 of_mod2sparse *m2,	/* Right operand of multiply */
 of_mod2sparse *r		/* Place to store result of multiply */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e1, *e2;
	INT32 i, j, b;

	if (of_mod2sparse_cols (m1) != of_mod2sparse_rows (m2)
			|| of_mod2sparse_rows (m1) != of_mod2sparse_rows (r)
			|| of_mod2sparse_cols (m2) != of_mod2sparse_cols (r))
	{
		fprintf (stderr,
			 "mod2sparse_multiply: Matrices have incompatible dimensions\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (r == m1 || r == m2)
	{
		fprintf (stderr,
			 "mod2sparse_multiply: Result matrix is the same as one of the operands\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	of_mod2sparse_clear (r);

	for (i = 0; i < of_mod2sparse_rows (m1); i++)
	{
		if (of_mod2sparse_at_end (of_mod2sparse_first_in_row (m1, i)))
		{
			continue;
		}

		for (j = 0; j < of_mod2sparse_cols (m2); j++)
		{
			b = 0;

			e1 = of_mod2sparse_first_in_row (m1, i);
			e2 = of_mod2sparse_first_in_col (m2, j);

			while (!of_mod2sparse_at_end (e1) && !of_mod2sparse_at_end (e2))
			{
				if (of_mod2sparse_col (e1) == of_mod2sparse_row (e2))
				{
					b ^= 1;
					e1 = of_mod2sparse_next_in_row (e1);
					e2 = of_mod2sparse_next_in_col (e2);
				}

				else
					if (of_mod2sparse_col (e1) < of_mod2sparse_row (e2))
					{
						e1 = of_mod2sparse_next_in_row (e1);
					}

					else
					{
						e2 = of_mod2sparse_next_in_col (e2);
					}
			}

			if (b)
			{
				of_mod2sparse_insert (r, i, j);
			}
		}
	}
	OF_EXIT_FUNCTION
}


/* MULTIPLY VECTOR BY SPARSE MATRIX. */

void mod2sparse_mulvec
(of_mod2sparse *m,	/* The sparse matrix, with M rows and N columns */
 UINT8 *u,		/* The input vector, N long */
 UINT8 *v		/* Place to store the result, M long */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	INT32 M, N;
	INT32 i, j;

	M = of_mod2sparse_rows (m);
	N = of_mod2sparse_cols (m);

	for (i = 0; i < M; i++)
		v[i] = 0;

	for (j = 0; j < N; j++)
	{
		if (u[j])
		{
			for (e = of_mod2sparse_first_in_col (m, j);
					!of_mod2sparse_at_end (e);
					e = of_mod2sparse_next_in_col (e))
			{
				v[of_mod2sparse_row (e) ] ^= 1;
			}
		}
	}
	OF_EXIT_FUNCTION
}


/* COUNT ENTRIES IN A ROW. */

UINT32 mod2sparse_count_row
(of_mod2sparse *m,
 UINT32 row
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	INT32 count;

	if (row < 0 || row >= of_mod2sparse_rows (m))
	{
		fprintf (stderr, "mod2sparse_count_row: row index out of bounds\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	count = 0;

	for (e = of_mod2sparse_first_in_row (m, row);
			!of_mod2sparse_at_end (e);
			e = of_mod2sparse_next_in_row (e))
	{
		count += 1;
	}

	OF_EXIT_FUNCTION
	return count;
}


/* COUNT ENTRIES IN A COLUMN. */

UINT32 mod2sparse_count_col
(of_mod2sparse *m,
 UINT32 col
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *e;
	INT32 count;

	if (col < 0 || col >= of_mod2sparse_cols (m))
	{
		fprintf (stderr, "mod2sparse_count_col: column index out of bounds\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	count = 0;

	for (e = of_mod2sparse_first_in_col (m, col);
			!of_mod2sparse_at_end (e);
			e = of_mod2sparse_next_in_col (e))
	{
		count += 1;
	}

	OF_EXIT_FUNCTION
	return count;
}


/* ADD TO A ROW. */

void mod2sparse_add_row
(of_mod2sparse *m1,	/* Matrix containing row to add to */
 UINT32 row1,		/* Index in this matrix of row to add to */
 of_mod2sparse *m2,	/* Matrix containing row to add from */
 UINT32 row2		/* Index in this matrix of row to add from */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *f1, *f2, *ft;

	if (of_mod2sparse_cols (m1) < of_mod2sparse_cols (m2))
	{
		fprintf (stderr,
			 "mod2sparse_add_row: row added to is shorter than row added from\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (row1 < 0 || row1 >= of_mod2sparse_rows (m1)
			|| row2 < 0 || row2 >= of_mod2sparse_rows (m2))
	{
		fprintf (stderr, "mod2sparse_add_row: row index out of range\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	f1 = of_mod2sparse_first_in_row (m1, row1);
	f2 = of_mod2sparse_first_in_row (m2, row2);

	while (!of_mod2sparse_at_end (f1) && !of_mod2sparse_at_end (f2))
	{
		if (of_mod2sparse_col (f1) > of_mod2sparse_col (f2))
		{
			of_mod2sparse_insert (m1, row1, of_mod2sparse_col (f2));
			f2 = of_mod2sparse_next_in_row (f2);
		}
		else
		{
			ft = of_mod2sparse_next_in_row (f1);
			if (of_mod2sparse_col (f1) == of_mod2sparse_col (f2))
			{
				of_mod2sparse_delete (m1, f1);
				f2 = of_mod2sparse_next_in_row (f2);
			}
			f1 = ft;
		}
	}

	while (!of_mod2sparse_at_end (f2))
	{
		of_mod2sparse_insert (m1, row1, of_mod2sparse_col (f2));
		f2 = of_mod2sparse_next_in_row (f2);
	}

	OF_EXIT_FUNCTION
}


/* ADD TO A COLUMN. */

void mod2sparse_add_col
(of_mod2sparse *m1,	/* Matrix containing column to add to */
 UINT32 col1,		/* Index in this matrix of column to add to */
 of_mod2sparse *m2,	/* Matrix containing column to add from */
 UINT32 col2		/* Index in this matrix of column to add from */
)
{
	OF_ENTER_FUNCTION
	of_mod2entry *f1, *f2, *ft;

	if (of_mod2sparse_rows (m1) < of_mod2sparse_rows (m2))
	{
		fprintf (stderr,
			 "mod2sparse_add_col: Column added to is shorter than column added from\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (col1 < 0 || col1 >= of_mod2sparse_cols (m1)
			|| col2 < 0 || col2 >= of_mod2sparse_cols (m2))
	{
		fprintf (stderr, "mod2sparse_add_col: Column index out of range\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	f1 = of_mod2sparse_first_in_col (m1, col1);
	f2 = of_mod2sparse_first_in_col (m2, col2);

	while (!of_mod2sparse_at_end (f1) && !of_mod2sparse_at_end (f2))
	{
		if (of_mod2sparse_row (f1) > of_mod2sparse_row (f2))
		{
			of_mod2sparse_insert (m1, of_mod2sparse_row (f2), col1);
			f2 = of_mod2sparse_next_in_col (f2);
		}
		else
		{
			ft = of_mod2sparse_next_in_col (f1);
			if (of_mod2sparse_row (f1) == of_mod2sparse_row (f2))
			{
				of_mod2sparse_delete (m1, f1);
				f2 = of_mod2sparse_next_in_col (f2);
			}
			f1 = ft;
		}
	}

	while (!of_mod2sparse_at_end (f2))
	{
		of_mod2sparse_insert (m1, of_mod2sparse_row (f2), col1);
		f2 = of_mod2sparse_next_in_col (f2);
	}

	OF_EXIT_FUNCTION
}


/* FIND AN LU DECOMPOSITION OF A SPARSE MATRIX. */

UINT32 mod2sparse_decomp
(of_mod2sparse *A,	/* Input matrix, M by N */
 UINT32 K,		/* Size of sub-matrix to find LU decomposition of */
 of_mod2sparse *L,	/* Matrix in which L is stored, M by K */
 of_mod2sparse *U,	/* Matrix in which U is stored, K by N */
 UINT32 *rows,		/* Array where row indexes are stored, M long */
 UINT32 *cols,		/* Array where column indexes are stored, N long */
 mod2sparse_strategy strategy, /* Strategy to follow in picking rows/columns */
 UINT32 abandon_number,	/* Number of columns to abandon at some point */
 UINT32 abandon_when	/* When to abandon these columns */
)
{
	OF_ENTER_FUNCTION
	UINT32 *rinv = NULL, *cinv = NULL, *acnt = NULL, *rcnt = NULL;
	of_mod2sparse *B = NULL;
	UINT32 M, N;

	of_mod2entry *e = NULL, *f = NULL, *fn = NULL, *e2 = NULL;
	INT32 i = 0, j = 0, k = 0, cc = 0, cc2 = 0, cc3 = 0, cr2 = 0, pr = 0;
	INT32 found, nnf;

	M = of_mod2sparse_rows (A);
	N = of_mod2sparse_cols (A);

	if (of_mod2sparse_cols (L) != K || of_mod2sparse_rows (L) != M
			|| of_mod2sparse_cols (U) != N || of_mod2sparse_rows (U) != K)
	{
		fprintf (stderr,
			 "mod2sparse_decomp: Matrices have incompatible dimensions\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	if (abandon_number > N - K)
	{
		fprintf (stderr, "Trying to abandon more columns than allowed\n");
		OF_EXIT_FUNCTION
		return NULL;
	}

	rinv = (UINT32*) of_calloc (M, sizeof * rinv);
	cinv = (UINT32*) of_calloc (N, sizeof * cinv);

	if (abandon_number > 0)
	{
		acnt = (UINT32*) of_calloc (M + 1, sizeof * acnt);
	}

	if (strategy == Mod2sparse_minprod)
	{
		rcnt = (UINT32*) of_calloc (M, sizeof * rcnt);
	}

	of_mod2sparse_clear (L);
	of_mod2sparse_clear (U);

	/* Copy A to B.  B will be modified, then discarded. */

	B = of_mod2sparse_allocate (M, N);
	of_mod2sparse_copy (A, B);

	/* Count 1s in rows of B, if using minprod strategy. */

	if (strategy == Mod2sparse_minprod)
	{
		for (i = 0; i < M; i++)
		{
			rcnt[i] = mod2sparse_count_row (B, i);
		}
	}

	/* Set up initial row and column choices. */

	for (i = 0; i < M; i++)
		rows[i] = rinv[i] = i;
	for (j = 0; j < N; j++)
		cols[j] = cinv[j] = j;

	/* Find L and U one column at a time. */

	nnf = 0;

	for (i = 0; i < K; i++)
	{
		/* Choose the next row and column of B. */

		switch (strategy)
		{
		case Mod2sparse_first:
		{
			found = 0;

			for (k = i; k < N; k++)
			{
				e = of_mod2sparse_first_in_col (B, cols[k]);
				while (!of_mod2sparse_at_end (e))
				{
					if (rinv[of_mod2sparse_row (e) ] >= i)
					{
						found = 1;
						goto out_first;
					}
					e = of_mod2sparse_next_in_col (e);
				}
			}

out_first:
			break;
		}

		case Mod2sparse_mincol:
		{
			found = 0;

			for (j = i; j < N; j++)
			{
				cc2 = mod2sparse_count_col (B, cols[j]);
				if (!found || cc2 < cc)
				{
					e2 = of_mod2sparse_first_in_col (B, cols[j]);
					while (!of_mod2sparse_at_end (e2))
					{
						if (rinv[of_mod2sparse_row (e2) ] >= i)
						{
							found = 1;
							cc = cc2;
							e = e2;
							k = j;
							break;
						}
						e2 = of_mod2sparse_next_in_col (e2);
					}
				}
			}

			break;
		}

		case Mod2sparse_minprod:
		{
			found = 0;

			for (j = i; j < N; j++)
			{
				cc2 = mod2sparse_count_col (B, cols[j]);
				e2 = of_mod2sparse_first_in_col (B, cols[j]);
				while (!of_mod2sparse_at_end (e2))
				{
					if (rinv[of_mod2sparse_row (e2) ] >= i)
					{
						cr2 = rcnt[of_mod2sparse_row (e2) ];
						if (!found || cc2 == 1 || (cc2 - 1) * (cr2 - 1) < pr)
						{
							found = 1;
							pr = cc2 == 1 ? 0 : (cc2 - 1) * (cr2 - 1);
							e = e2;
							k = j;
						}
					}
					e2 = of_mod2sparse_next_in_col (e2);
				}
			}

			break;
		}

		default:
		{
			fprintf (stderr, "mod2sparse_decomp: Unknown stategy\n");
			OF_EXIT_FUNCTION
			return NULL;
		}
		}

		if (!found)
		{
			nnf += 1;
		}

		/* Update 'rows' and 'cols'.  Looks at 'k' and 'e' found above. */

		if (found)
		{
			if (cinv[of_mod2sparse_col (e) ] != k)
				abort();

			cols[k] = cols[i];
			cols[i] = of_mod2sparse_col (e);

			cinv[cols[k]] = k;
			cinv[cols[i]] = i;

			k = rinv[of_mod2sparse_row (e) ];

			if (k < i)
				abort();

			rows[k] = rows[i];
			rows[i] = of_mod2sparse_row (e);

			rinv[rows[k]] = k;
			rinv[rows[i]] = i;
		}

		/* Update L, U, and B. */

		f = of_mod2sparse_first_in_col (B, cols[i]);

		while (!of_mod2sparse_at_end (f))
		{
			fn = of_mod2sparse_next_in_col (f);
			k = of_mod2sparse_row (f);

			if (rinv[k] > i)
			{
				mod2sparse_add_row (B, k, B, of_mod2sparse_row (e));
				if (strategy == Mod2sparse_minprod)
				{
					rcnt[k] = mod2sparse_count_row (B, k);
				}
				of_mod2sparse_insert (L, k, i);
			}
			else
				if (rinv[k] < i)
				{
					of_mod2sparse_insert (U, rinv[k], cols[i]);
				}
				else
				{
					of_mod2sparse_insert (L, k, i);
					of_mod2sparse_insert (U, i, cols[i]);
				}

			f = fn;
		}

		/* Get rid of all entries in the current column of B, just to save space. */

		for (;;)
		{
			f = of_mod2sparse_first_in_col (B, cols[i]);
			if (of_mod2sparse_at_end (f))
				break;
			of_mod2sparse_delete (B, f);
		}

		/* Abandon columns of B with lots of entries if it's time for that. */

		if (abandon_number > 0 && i == abandon_when)
		{
			for (k = 0; k < M + 1; k++)
			{
				acnt[k] = 0;
			}
			for (j = 0; j < N; j++)
			{
				k = mod2sparse_count_col (B, j);
				acnt[k] += 1;
			}

			cc = abandon_number;
			k = M;
			while (acnt[k] < cc)
			{
				cc -= acnt[k];
				k -= 1;
				if (k < 0)
					abort();
			}

			cc2 = 0;
			for (j = 0; j < N; j++)
			{
				cc3 = mod2sparse_count_col (B, j);
				if (cc3 > k || cc3 == k && cc > 0)
				{
					if (cc3 == k)
						cc -= 1;
					for (;;)
					{
						f = of_mod2sparse_first_in_col (B, j);
						if (of_mod2sparse_at_end (f))
							break;
						of_mod2sparse_delete (B, f);
					}
					cc2 += 1;
				}
			}

			if (cc2 != abandon_number)
				abort();

			if (strategy == Mod2sparse_minprod)
			{
				for (j = 0; j < M; j++)
				{
					rcnt[j] = mod2sparse_count_row (B, j);
				}
			}
		}
	}

	/* Get rid of all entries in the rows of L past row K, after reordering. */

	for (i = K; i < M; i++)
	{
		for (;;)
		{
			f = of_mod2sparse_first_in_row (L, rows[i]);
			if (of_mod2sparse_at_end (f))
				break;
			of_mod2sparse_delete (L, f);
		}
	}

	of_mod2sparse_free (B);
	free (rinv);
	free (cinv);
	if (strategy == Mod2sparse_minprod)
		free (rcnt);
	if (abandon_number > 0)
		free (acnt);

	OF_EXIT_FUNCTION
	return nnf;
}


/* SOLVE A LOWER-TRIANGULAR SYSTEM BY FORWARD SUBSTITUTION. */

UINT32 mod2sparse_forward_sub
(of_mod2sparse *L,	/* Matrix that is lower triangular after reordering */
 UINT32 *rows,		/* Array of indexes (from 0) of rows for new order */
 UINT8 *x,		/* Vector on right of equation, also reordered */
 UINT8 *y		/* Place to store solution */
)
{
	OF_ENTER_FUNCTION
	INT32 K, i, j, ii, b, d;
	of_mod2entry *e;

	K = of_mod2sparse_cols (L);

	/* Make sure that L is lower-triangular, after row re-ordering. */

	for (i = 0; i < K; i++)
	{
		ii = rows ? rows[i] : i;
		e = of_mod2sparse_last_in_row (L, ii);
		if (!of_mod2sparse_at_end (e) && of_mod2sparse_col (e) > i)
		{
			fprintf (stderr,
				 "mod2sparse_forward_sub: Matrix is not lower-triangular\n");
			OF_EXIT_FUNCTION
			return NULL;
		}
	}

	/* Solve system by forward substitution. */

	for (i = 0; i < K; i++)
	{
		ii = rows ? rows[i] : i;

		/* Look at bits in this row, forming inner product with partial
		solution, and seeing if the diagonal is 1. */

		d = 0;
		b = 0;

		for (e = of_mod2sparse_first_in_row (L, ii);
				!of_mod2sparse_at_end (e);
				e = of_mod2sparse_next_in_row (e))
		{
			j = of_mod2sparse_col (e);

			if (j == i)
			{
				d = 1;
			}
			else
			{
				b ^= y[j];
			}
		}

		/* Check for no solution if the diagonal isn't 1. */

		if (!d && b != x[ii])
		{
			OF_EXIT_FUNCTION
			return 0;
		}

		/* Set bit of solution, zero if arbitrary. */

		y[i] = b ^ x[ii];
	}

	OF_EXIT_FUNCTION
	return 1;
}


/* SOLVE AN UPPER-TRIANGULAR SYSTEM BY BACKWARD SUBSTITUTION. */

UINT32 mod2sparse_backward_sub
(of_mod2sparse *U,	/* Matrix that is upper triangular after reordering */
 UINT32 *cols,		/* Array of indexes (from 0) of columns for new order */
 UINT8 *y,		/* Vector on right of equation */
 UINT8 *z		/* Place to store solution, also reordered */
)
{
	OF_ENTER_FUNCTION
	INT32 K, i, j, ii, b, d;
	of_mod2entry *e;

	K = of_mod2sparse_rows (U);

	/* Make sure that U is upper-triangular, after column re-ordering. */

	for (i = 0; i < K; i++)
	{
		ii = cols ? cols[i] : i;
		e = of_mod2sparse_last_in_col (U, ii);
		if (!of_mod2sparse_at_end (e) && of_mod2sparse_row (e) > i)
		{
			fprintf (stderr,
				 "mod2sparse_backward_sub: Matrix is not upper-triangular\n");
			OF_EXIT_FUNCTION
			return NULL;
		}
	}

	/* Solve system by backward substitution. */

	for (i = K - 1; i >= 0; i--)
	{
		ii = cols ? cols[i] : i;

		/* Look at bits in this row, forming inner product with partial
		solution, and seeing if the diagonal is 1. */

		d = 0;
		b = 0;

		for (e = of_mod2sparse_first_in_row (U, i);
				!of_mod2sparse_at_end (e);
				e = of_mod2sparse_next_in_row (e))
		{
			j = of_mod2sparse_col (e);

			if (j == ii)
			{
				d = 1;
			}
			else
			{
				b ^= z[j];
			}
		}

		/* Check for no solution if the diagonal isn't 1. */

		if (!d && b != y[i])
		{
			return 0;
		}

		/* Set bit of solution, zero if arbitrary. */

		z[ii] = b ^ y[i];
	}

	OF_EXIT_FUNCTION
	return 1;
}
#endif // #if 0


#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
of_mod2entry * of_mod2sparse_last_in_col (of_mod2sparse	*m,
					  UINT32	i)
{
	of_mod2entry	*e;

	OF_ENTER_FUNCTION
	e = of_mod2sparse_first_in_col (m, i);
	while (!of_mod2sparse_at_end (e->down))
	{
		e = e->down;
	}
	OF_EXIT_FUNCTION
	return e;
}
#endif


#if defined(ML_DECODING) /* { */

//
// Return the weight of the line row0
//
UINT32 of_mod2sparse_swap_rows (of_mod2sparse	* m,  // Matrix in which we swap two rows
				UINT32          row0, // swap row e0 and row e1
				UINT32          row1,
				of_mod2sparse	*__swap,
				of_mod2entry	** __links,
				of_mod2entry	**__parsing)
{
	UINT32 row_weight;
	UINT32 __col;

	OF_ENTER_FUNCTION
	row_weight = 0;
	// If the current row is not the one we want to swap with
	if (row0 != row1)
	{
		//of_mod2sparse * __swap;
		UINT32        __cpy[2];
		of_mod2entry * e0;
		of_mod2entry * e1;
		of_mod2entry * __e, * __prev;
		of_mod2entry * __delMe;
		of_mod2entry * __col_entry, * __new_entry;
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		of_mod2entry * __col_entry_;
#endif
		bool __swap_was_null = false;

		// Copy e0 in a __swap matrix
		if (__swap == NULL)
		{
			__swap_was_null = true;
			__swap = of_mod2sparse_allocate (1, of_mod2sparse_cols (m));
		}

		__cpy[0] = row0;
		__cpy[1] = row1;
		of_mod2sparse_copyrows_opt (m, __swap, __cpy, __parsing);

		// Delete original e0
		e0 = of_mod2sparse_first_in_row (m, row0);
		while (!of_mod2sparse_at_end_row (e0))
		{
			__e = e0;
			e0 = of_mod2sparse_next_in_row (e0);
			if (__links != NULL)
			{
				of_mod2sparse_delete_opt (m, __e, __links);
			}
			else
			{
				of_mod2sparse_delete (m, __e);
			}
		} // while (!of_mod2sparse_at_end(e0))


		// Copy e1 to e0
		row_weight = 0;
		e1 = of_mod2sparse_first_in_row (m, row1);
		while (!of_mod2sparse_at_end_row (e1))
		{
			__prev = of_mod2sparse_last_in_row (m, row0);

			/* Create a new element */
			__new_entry = of_alloc_entry (m);

			__new_entry->row = row0;
			__new_entry->col = of_mod2sparse_col (e1);
			__col = of_mod2sparse_col (__new_entry);

			// Insert new element in the approrpiate line
			__new_entry->left = __prev;
			__new_entry->right = of_mod2sparse_next_in_row (__prev);
			__new_entry->left->right = __new_entry;
			__new_entry->right->left = __new_entry;

			/* Insert new entry into column. */
			if (__links == NULL)
			{
				__col_entry = of_mod2sparse_first_in_col (m, __col);
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
				__col_entry_ = __col_entry;
#endif
				while (!of_mod2sparse_at_end_col (__col_entry) && of_mod2sparse_row (__col_entry) < row0)
				{
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
					__col_entry_ = __col_entry;
#endif
					__col_entry = of_mod2sparse_next_in_col (__col_entry);
				}
			}  // if (__links == NULL) ...
			else
			{
				if (__links[__col] == NULL)
				{
					// It is the first time I insert a cell in this col
					__col_entry = of_mod2sparse_first_in_col (m, __col);
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
					__col_entry_ = __col_entry;
#endif
				}  // if (__links[of_mod2sparse_col(__new_entry)] == NULL) ...
				else
				{
					// Cells have already been inserted in the col, the correct position is then in __links[col]
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
					__col_entry_ = __links[__col];
#endif
					__col_entry = of_mod2sparse_next_in_col (__links[__col]);
				} // if (__links[of_mod2sparse_col(__new_entry)] == NULL) ... else ...
			} // if (__links == NULL) ... else ...

			__new_entry->down = __col_entry;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__new_entry->up       = __col_entry->up;
			__new_entry->up->down = __new_entry;
			__new_entry->down->up = __new_entry;
#else
			__col_entry_->down = __new_entry;
#endif

			if (__links != NULL)
			{
				__links[__col] = __new_entry;
			}

			__delMe = e1;
			e1 = of_mod2sparse_next_in_row (e1);
			if (__links != NULL)
			{
				of_mod2sparse_delete_opt (m, __delMe, __links);
			}
			else
			{
				of_mod2sparse_delete (m, __delMe);
			}

			++row_weight;
		} // while (!of_mod2sparse_at_end(e0))

		// Copy e0 to e1 from the __swap matrix
		__e = of_mod2sparse_first_in_row (__swap, 0);
		while (!of_mod2sparse_at_end_row (__e))
		{
			__prev = of_mod2sparse_last_in_row (m, row1);

			/* Insert into row. */
			__new_entry = of_alloc_entry (m);

			__new_entry->row = row1;
			__new_entry->col = of_mod2sparse_col (__e);
			__col = of_mod2sparse_col (__new_entry);


			__new_entry->left = __prev;
			__new_entry->right = of_mod2sparse_next_in_row (__prev);
			__new_entry->left->right = __new_entry;
			__new_entry->right->left = __new_entry;

			/* Insert new entry into column. */
			if (__links != NULL)
			{
				if (__links[__col] != NULL)
				{
					__col_entry = __links[__col];
				}
				else
				{
					__col_entry = of_mod2sparse_first_in_col (m, __col);
				}
			}
			else
			{
				__col_entry = of_mod2sparse_first_in_col (m, __col);
			}

#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__col_entry_ = __col_entry;
#endif
			while (!of_mod2sparse_at_end_col (__col_entry) && of_mod2sparse_row (__col_entry) < row1)
			{
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
				__col_entry_ = __col_entry;
#endif
				__col_entry = of_mod2sparse_next_in_col (__col_entry);
			}

			__new_entry->down = __col_entry;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__new_entry->up = __col_entry->up;
			__new_entry->up->down = __new_entry;
			__new_entry->down->up = __new_entry;
#else
			__col_entry_->down = __new_entry;
#endif

			__delMe = __e;
			__e = of_mod2sparse_next_in_row (__e);
			of_mod2sparse_delete (__swap, __delMe);
		} // while (!of_mod2sparse_at_end(__e))


		if (__swap_was_null == true)
		{
			of_mod2sparse_free (__swap);
			of_free (__swap);
		}

		if (__parsing != NULL && __links != NULL)
		{
			memcpy (
				& (__parsing[row0]),
				& (__links[row0]),
				(of_mod2sparse_cols (m) - row0) * sizeof (of_mod2entry *)
			);
		}
	} // if (row1 != row0) ...
	else
	{
		if (__links != NULL)
		{
			of_mod2entry * __e;

			__e = of_mod2sparse_first_in_row (m, row0);
			//__col = of_mod2sparse_col (__e);
			while (!of_mod2sparse_at_end_row (__e))
			{
				row_weight++;
				__links[of_mod2sparse_col (__e) ] = __e;

				__e = of_mod2sparse_next_in_row (__e);
			}
		}
		if (__parsing != NULL && __links != NULL)
		{
			memcpy (
				& (__parsing[row0]),
				& (__links[row0]),
				(of_mod2sparse_cols (m) - row0) * sizeof (of_mod2entry *)
			);
		}
	}
	OF_EXIT_FUNCTION
	return row_weight;
}


//
// Return the weight of the destination line
//
UINT32 of_mod2sparse_xor_rows  (of_mod2sparse	*m,    // Matrix in which we swap two rows
				UINT32          row0, // xor row e0 and row e1, and set the result in row1
				UINT32          row1,
				of_mod2entry	**__links,
				of_mod2entry	**__parsing)
{
	UINT32 __col;
	of_mod2entry * __e0;
	of_mod2entry * __e1;
	of_mod2entry * __prev;
	of_mod2entry * __col_entry, * __new_entry;
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
	of_mod2entry * __col_entry_;
#endif
	INT32		row_weight;

	OF_ENTER_FUNCTION
	row_weight = 0;
	// Verify if row0 is empty. In this case row1 stays the same.
	if (of_mod2sparse_empty_row (m, row0))
	{
		of_mod2entry * __e;

		if (__parsing != NULL)
		{
			__e = of_mod2sparse_first_in_row (m, row1);

			while (!of_mod2sparse_at_end_row (__e))
			{
				row_weight++;
				__parsing[of_mod2sparse_col (__e) ] = __e;
				__e = of_mod2sparse_next_in_row (__e);
			}
			OF_EXIT_FUNCTION
			return row_weight;
		}
		OF_EXIT_FUNCTION
		return of_mod2sparse_weight_row (m, row1);
	}

	// Verify if the two rows are equal. In that case, it is equivalent to reseting the line.
	if (row0 == row1)
	{
		of_mod2entry * __delMe;

		__e0 = of_mod2sparse_first_in_row (m, row0);
		while (!of_mod2sparse_at_end_row (__e0))
		{
			__delMe = __e0;
			__e0 = of_mod2sparse_next_in_row (__e0);
			if (__parsing != NULL)
			{
				of_mod2sparse_delete_opt (m, __delMe, __parsing);
			}
			else
			{
				of_mod2sparse_delete (m, __delMe);
			}
		}
		OF_EXIT_FUNCTION
		return 0;
	}

	__e0 = of_mod2sparse_first_in_row (m, row0);
	__e1 = of_mod2sparse_first_in_row (m, row1);

	row_weight = 0;

	while (!of_mod2sparse_at_end_row (__e0) && !of_mod2sparse_at_end_row (__e1))
	{
		if (of_mod2sparse_col (__e0) < of_mod2sparse_col (__e1))
		{
			row_weight++;

			/* Insert into row. */
			__new_entry = of_alloc_entry (m);

			__new_entry->row = row1;
			__new_entry->col = of_mod2sparse_col (__e0);
			__col = of_mod2sparse_col (__new_entry);

			__new_entry->left = of_mod2sparse_prev_in_row (__e1);
			__new_entry->right = __e1;
			__new_entry->left->right = __new_entry;
			__new_entry->right->left = __new_entry;

			/* Insert new entry into column. */
			if (__parsing != NULL)
			{
				if (__parsing[__col] != NULL)
				{
					__col_entry = __parsing[__col];
				}
				else
				{
					__col_entry = of_mod2sparse_first_in_col (m, __col);
				}
			}
			else
			{
				__col_entry = of_mod2sparse_first_in_col (m, __col);
			} // if (__parsing != NULL) ...

#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__col_entry_ = __col_entry;
#endif
			while (!of_mod2sparse_at_end_col (__col_entry) && of_mod2sparse_row (__col_entry) < row1)
			{
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
				__col_entry_ = __col_entry;
#endif
				__col_entry = of_mod2sparse_next_in_col (__col_entry);
			}
			__new_entry->down = __col_entry;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__new_entry->up = __col_entry->up;
			__new_entry->up->down = __new_entry;
			__new_entry->down->up = __new_entry;
#else
			__col_entry_->down = __new_entry;
#endif

			if (__parsing != NULL)
			{
				__parsing[of_mod2sparse_col (__new_entry) ] = __new_entry;
			}
			__e0 = of_mod2sparse_next_in_row (__e0);
		}
		else
		{
			if (of_mod2sparse_col (__e0) == of_mod2sparse_col (__e1))
			{
				if (__parsing != NULL)
				{
					if (__links != NULL)
					{
						__parsing[of_mod2sparse_col (__e1) ] = __links[of_mod2sparse_col (__e1) ];
					}
					else
					{
						of_mod2entry * __e;

						__e = __parsing[of_mod2sparse_col (__e1) ];
						__prev = __e;
						while (!of_mod2sparse_at_end_col (__e) && of_mod2sparse_row (__e) < row1)
						{
							__prev = __e;
							__e = of_mod2sparse_next_in_col (__e);
						}

						__parsing[of_mod2sparse_col (__e1) ] = __prev;
					}
				}
				if (__parsing != NULL)
				{
					of_mod2sparse_delete_opt (m, __e1, __parsing);
				}
				else
				{
					of_mod2sparse_delete (m, __e1);
				}
				__e0 = of_mod2sparse_next_in_row (__e0);
				__e1 = of_mod2sparse_next_in_row (__e1);
			}
			else
			{
				row_weight++;
				if (__parsing != NULL)
				{
					__parsing[of_mod2sparse_col (__e1) ] = __e1;
				}

				__e1 = of_mod2sparse_next_in_row (__e1);
			}
		}
	}

	while (!of_mod2sparse_at_end_row (__e0))
	{
		row_weight++;

		__prev = of_mod2sparse_last_in_row (m, row1);

		/* Insert into row. */
		__new_entry = of_alloc_entry (m);

		__new_entry->row = row1;
		__new_entry->col = of_mod2sparse_col (__e0);
		__col = of_mod2sparse_col (__new_entry);

		__new_entry->left = __prev;
		__new_entry->right = of_mod2sparse_next_in_row (__prev);
		__new_entry->left->right = __new_entry;
		__new_entry->right->left = __new_entry;

		/* Insert new entry into column. */
		if (__parsing != NULL)
		{
			if (__parsing[of_mod2sparse_col (__new_entry) ] != NULL)
			{
				__col_entry = __parsing[__col];
			}
			else
			{
				__col_entry = of_mod2sparse_first_in_col (m, __col);
			}
		}
		else
		{
			__col_entry = of_mod2sparse_first_in_col (m, __col);
		}

#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		__col_entry_ = __col_entry;
#endif
		while (!of_mod2sparse_at_end_col (__col_entry) && of_mod2sparse_row (__col_entry) < row1)
		{
#ifdef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
			__col_entry_ = __col_entry;
#endif
			__col_entry = of_mod2sparse_next_in_col (__col_entry);
		}

		__new_entry->down = __col_entry;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
		__new_entry->up = __col_entry->up;
		__new_entry->up->down = __new_entry;
		__new_entry->down->up = __new_entry;
#else
		__col_entry_->down = __new_entry;
#endif

		if (__parsing != NULL)
		{
			__parsing[of_mod2sparse_col (__new_entry) ] = __new_entry;
		}

		__e0 = of_mod2sparse_next_in_row (__e0);
	}

	OF_EXIT_FUNCTION
	return row_weight;
}


bool of_mod2sparse_empty_row (of_mod2sparse * m, UINT32 row)
{
	OF_ENTER_FUNCTION
	if (of_mod2sparse_at_end_row (of_mod2sparse_first_in_row (m, row)))
	{
		OF_EXIT_FUNCTION
		return true;
	}

	OF_EXIT_FUNCTION
	return false;
}


bool of_mod2sparse_empty_col (of_mod2sparse * m, UINT32 col)
{
	OF_ENTER_FUNCTION
	if (m != NULL)
	{
		if (of_mod2sparse_at_end_col (of_mod2sparse_first_in_col (m, col)))
		{
			OF_EXIT_FUNCTION
			return true;
		}
		OF_EXIT_FUNCTION
		return false;
	}
	else
	{
		fprintf (stderr, "mod2sparse_empty_col: Matrix m does not exist.");
		OF_EXIT_FUNCTION
		return false;
	}
}


UINT32 of_mod2sparse_weight_row (of_mod2sparse * m, UINT32 row)
{
	OF_ENTER_FUNCTION
	of_mod2entry * __e;
	INT32         row_weight;

	row_weight = 0;

	__e = of_mod2sparse_first_in_row (m, row);
	while (!of_mod2sparse_at_end (__e))
	{
		++row_weight;
		__e = of_mod2sparse_next_in_row (__e);
	}
	OF_EXIT_FUNCTION
	return row_weight;
}


#if 0
UINT32 mod2sparse_weight_col (of_mod2sparse * m, UINT32 col)
{
	OF_ENTER_FUNCTION
	of_mod2entry * __e;
	INT32         col_weight;

	col_weight = 0;
	__e = of_mod2sparse_first_in_col (m, col);
	while (!of_mod2sparse_at_end (__e))
	{
		++col_weight;
		__e = of_mod2sparse_next_in_col (__e);
	}
	OF_EXIT_FUNCTION
	return col_weight;
}
#endif // #if 0


void of_mod2sparse_copy_filled_matrix  (of_mod2sparse	*m,
					of_mod2sparse	*r,
					UINT32		*index_rows,
					UINT32		*index_cols)
{
	of_mod2entry	*e;
	UINT32		i;

	for (i = 0; i < of_mod2sparse_rows(m); i++)
	{
		e = of_mod2sparse_first_in_row(m, i);
		while (!of_mod2sparse_at_end_row(e))
		{			
			if (!of_mod2sparse_empty_col(m, e->col) && !of_mod2sparse_empty_row(m, e->row))
			{
				//printf("insert : %hd,%hd from %hd,%hd\n",index_rows[e->row], index_cols[e->col],e->row, e->col);
				of_mod2sparse_insert(r, index_rows[e->row], index_cols[e->col]);
			}
			e = of_mod2sparse_next_in_row(e);
		}
	}
}


#endif // } ML_DECODING


#endif //OF_USE_LINEAR_BINARY_CODES_UTILS
