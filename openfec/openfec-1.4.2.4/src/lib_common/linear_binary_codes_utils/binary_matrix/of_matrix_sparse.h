/* $Id: of_matrix_sparse.h 197 2014-07-16 15:21:56Z roca $ */
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
 * This module implements operations on sparse matrices of mod2 elements
 * (bits, with addition and multiplication being done modulo 2).
 *
 * All procedures in this module display an error message on standard
 * error and terminate the program if passed an invalid argument (indicative
 * of a programming error), or if memory cannot be allocated.  Errors from
 * invalid contents of a file result in an error code being returned to the
 * caller, with no message being printed by this module.
 */

/*
 * DATA STRUCTURES USED TO STORE A SPARSE MATRIX.  Non-zero entries (ie, 1s)
 * are represented by nodes that are doubly-linked both by row and by column,
 * with the headers for these lists being kept in arrays.  Nodes are allocated
 * in blocks to reduce time and space overhead.  Freed nodes are kept for
 * reuse in the same matrix, rather than being freed for other uses, except
 * that they are all freed when the matrix is cleared to all zeros by the
 * of_mod2sparse_clear procedure, or copied into by of_mod2sparse_copy.
 *
 * Direct access to these structures should be avoided except in low-level
 * routines.  Use the macros and procedures defined below instead.
 */

#ifndef OF_LDPC_MATRIX_SPARSE__
#define OF_LDPC_MATRIX_SPARSE__


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

//#define SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE

/**
 * Structure representing a non-zero entry, or the header for a row or column.
 */
typedef struct of_mod2entry
{
	/**
	 * Row and column indexes of this entry, starting at 0, and
	 * with -1 for a row or column header
	 */
#ifdef SPARSE_MATRIX_OPT_SMALL_INDEXES /* memory optimization, see ldpc_profile.h */
	currently not used... otherwise remove this line...
	INT16	row;
	INT16	col;
#else
	INT32	row;
	INT32	col;
#endif
	/**
	 * Pointers to entries adjacent in row and column, or to headers.
	 * Free entries are linked by 'left'.
	 */
	struct of_mod2entry	*left,			
				*right,
				*down;
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE /* memory optimization */
	struct of_mod2entry	*up;
#endif
} of_mod2entry;

#define of_mod2sparse_block 1024  /* Number of entries to block together for memory allocation */


/*
 * Block of entries allocated all at once.
 */
typedef struct of_mod2block
{
	/** Next block that has been allocated. */
	struct of_mod2block *next;

	/** Entries in this block. */
	of_mod2entry	entry[of_mod2sparse_block];
} of_mod2block;


/**
 * Representation of a sparse matrix.
 */
typedef struct of_mod2sparse
{
	INT32 n_rows;		  /* Number of rows in the matrix */
	INT32 n_cols;		  /* Number of columns in the matrix */

	of_mod2entry *rows;	  /* Pointer to array of row headers */
	of_mod2entry *cols;	  /* Pointer to array of column headers */

	of_mod2block *blocks;	  /* Blocks that have been allocated */
	of_mod2entry *next_free;  /* Next free entry */

#ifdef LDPC_QC
	INT32 exp_factor;           /* expansion factor of the matrix, it is also the size of the circulant matrix */
#endif
} of_mod2sparse;


/* MACROS TO GET AT ELEMENTS OF A SPARSE MATRIX.  The 'first', 'last', 'next',
   and 'prev' macros traverse the elements in a row or column.  Moving past
   the first/last element gets one to a header element, which can be identified
   using the 'at_end' macro.  Macros also exist for finding out the row
   and column of an entry, and for finding out the dimensions of a matrix. */

#define of_mod2sparse_first_in_row(m,i) ((m)->rows[i].right) /* Find the first   */
#define of_mod2sparse_first_in_col(m,j) ((m)->cols[j].down)  /* or last entry in */
#define of_mod2sparse_last_in_row(m,i) ((m)->rows[i].left)   /* a row or column  */
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
#define of_mod2sparse_last_in_col(m,j) ((m)->cols[j].up)
#else
of_mod2entry * of_mod2sparse_last_in_col (of_mod2sparse * m, UINT32 i); /* a bit more complex if we don't have the "up" pointer */
#endif

#define of_mod2sparse_next_in_row(e) ((e)->right)  /* Move from one entry to     */
#define of_mod2sparse_next_in_col(e) ((e)->down)   /* another in any of the three */
#define of_mod2sparse_prev_in_row(e) ((e)->left)   /* possible directions        */
#ifndef SPARSE_MATRIX_OPT_FOR_LDPC_STAIRCASE
#define of_mod2sparse_prev_in_col(e) ((e)->up)
#endif

#define of_mod2sparse_at_end_col(e) ((e)->col < 0) /* See if we've reached the end of column */
#define of_mod2sparse_at_end_row(e) ((e)->row < 0) /* See if we've reached the end of row */
#define of_mod2sparse_at_end(e) ((e)->row < 0)     /* See if we've reached the end of row */


#define of_mod2sparse_row(e) ((e)->row)      /* Find out the row or column index */
#define of_mod2sparse_col(e) ((e)->col)      /* of an entry (indexes start at 0) */

#define of_mod2sparse_rows(m) ((m)->n_rows)  /* Get the number of rows or columns*/
#define of_mod2sparse_cols(m) ((m)->n_cols)  /* in a matrix                      */


#if 0
/* POSSIBLE LU DECOMPOSITION STRATEGIES.  For use with mod2sparse_decomp. */

typedef enum mod2sparse_strategy_enum
{ Mod2sparse_first,
  Mod2sparse_mincol,
  Mod2sparse_minprod
} mod2sparse_strategy;
#endif // #if 0


/* PROCEDURES TO MANIPULATE SPARSE MATRICES. */
of_mod2sparse *of_mod2sparse_allocate (UINT32, UINT32);

void of_mod2sparse_free (of_mod2sparse *);

void of_mod2sparse_clear (of_mod2sparse *);

//#if 0
void of_mod2sparse_copy (of_mod2sparse *, of_mod2sparse *);
//#endif // #if 0

#if defined(ML_DECODING)

void of_mod2sparse_copyrows (of_mod2sparse *, of_mod2sparse *, UINT32 *);
void of_mod2sparse_copycols (of_mod2sparse *, of_mod2sparse *, UINT32 *);

/**
 * Copy a list of rows from one matrix to another.
 *
 * @param m                (IN)   Matrix to print out (only 0 and 1)
 * @param r                (OUT)  Resulting matrix
 * @param rows             (IN)   List of rows to copy
 * @param __parsing        (IN)   Optimization for the insertion of new elements,
 *                                avoid parsing again all the element in a cal before
 *                                finding the good position
 */
void of_mod2sparse_copyrows_opt (of_mod2sparse * m,
				 of_mod2sparse * r,
				 UINT32 * rows,
				 of_mod2entry ** __parsing);


/**
 * Copy a list of columns from one matrix to another.
 *
 * @param m                (IN)   Matrix to print out (only 0 and 1)
 * @param r                (OUT)  Resulting matrix
 * @param cols             (IN)   List of cols to copy
 */
void of_mod2sparse_copycols_opt (of_mod2sparse * m,
				 of_mod2sparse * r,
				 UINT32 * cols);

#endif // #if defined(ML_DECODING)


/**
 * Print out a matrix in a file, print only 0 and 1
 *
 * @param fout             (IN)  The stream to write in
 * @param m                (IN)  Matrix to print out (only 0 and 1)
 */
void of_mod2sparse_printf (FILE * fout, of_mod2sparse * m);


void of_mod2sparse_print (FILE *, of_mod2sparse *);
void of_mod2sparse_print_bitmap (of_mod2sparse *m);


#if 0
UINT32  mod2sparse_write (FILE *, of_mod2sparse *);
of_mod2sparse *mod2sparse_read (FILE *);
#endif // #if 0

UINT32 of_mod2sparse_write_human_readable (FILE *,
					   of_mod2sparse *,
					   UINT32 nb_source,
					   UINT32 nb_parity);

of_mod2sparse *of_mod2sparse_read_human_readable (FILE *f,
						  UINT32 *nb_source,
						  UINT32 *nb_parity);

/* PRINT matrix statistics: average number of 1's per row/line etc ...*/

void of_mod2sparse_matrix_stats (FILE *,
				 of_mod2sparse *,
				 UINT32 nb_src,
				 UINT32 nb_par);


of_mod2entry *of_mod2sparse_find (of_mod2sparse *,
				  UINT32,
				  UINT32);

of_mod2entry *of_mod2sparse_insert (of_mod2sparse *,
				    UINT32,
				    UINT32);

void of_mod2sparse_delete (of_mod2sparse *,
			   of_mod2entry *);


#if defined(ML_DECODING)

/**
 * Swap row0 and row1 of the matrix m, using an additional matrix (either allocated or not).
 * The two additional parameters optimize the parsing of the matrix.
 *
 * @param m                (IN/OUT)  The matrix which hold the two lines
 * @param row0             (IN)      First line to be swapped
 * @param row1             (IN)      Second line to be swapped
 * @param __swapMatrix     (IN)      Additional matrix used to store
 *                                   one of the line to be swapped.
 *                                   if NULL, the additionnal matrix is created and
 *                                   freed by the function
 * @param __links          (IN/OUT)  Used to minimised the parsing of the matrix on insertion of
 *                                   one element in the row row0. The gauss pivoting method parses
 *                                   the matrix from up to bottom, __links keeps the last element
 *                                   before row0 in the matrix (optimization for gauss pivoting)
 * @param __parsing        (IN/OUT)  Optimization for Gauss pivoting in the XOR steps. Gauss pivoting
 *                                   parses from up to bottom to XOR lines and then eliminates simplifies
 *                                   the matrix. At the end of swapping, __parsing is initialyzed.
 *
 * @return                           The hamming weight of the new row0 line
 */
UINT32  of_mod2sparse_swap_rows (of_mod2sparse * m,
				 UINT32 row0,
				 UINT32 row1,
				 of_mod2sparse * __swapMatrix,
				 of_mod2entry ** __links,
				 of_mod2entry ** __parsing);

/**
 * XOR two lines of the matrix m: row1 = row0 + row1.
 *
 * @param m                (IN/OUT)  The matrix which hold the two lines
 * @param row0             (IN)      First line to be XORed
 * @param row1             (IN)      Second line to be XORed
 * @param __links          (IN/OUT)  Used to minimised the parsing of the matrix on insertion of
 *                                   one element in the row row0. The gauss pivoting method parses
 *                                   the matrix from up to bottom, __links keeps the last element
 *                                   before row0 in the matrix (optimization for gauss pivoting)
 * @param __parsing        (IN/OUT)  Optimization for Gauss pivoting in the XOR steps. Gauss pivoting
 *                                   parses from up to bottom to XOR lines and then eliminates simplifies
 *                                   the matrix. At the end of swapping, __parsing is initialyzed.
 *
 * @return                           The hamming weight of the new row1 line
 */
UINT32  of_mod2sparse_xor_rows (of_mod2sparse * m,
				UINT32 row0, UINT32 row1,
				of_mod2entry ** __links,
				of_mod2entry ** __parsing);

/**
 * Determine if a row is empty
 *
 * @param m                (IN/OUT)  The matrix which hold the row
 * @param row              (IN)      The row to check
 *
 * @return                           TRUE if the row is empty, else FALSE
 */
bool of_mod2sparse_empty_row (of_mod2sparse * m, UINT32 row);


/**
 * Determine if a col is empty
 *
 * @param m                (IN/OUT)  The matrix which hold the col
 * @param col              (IN)      The col to check
 *
 * @return                           TRUE if the row is empty, else FALSE
 */
bool of_mod2sparse_empty_col (of_mod2sparse * m, UINT32 col);


/**
 * Count the number of 1 in a row
 *
 * @param m                (IN/OUT)  The matrix which the row
 * @param row              (IN)      The row to check
 *
 * @return                           The weight of the row
 */
UINT32 of_mod2sparse_weight_row (of_mod2sparse * m, UINT32 row);


#if 0
/**
 * Count the number of 1 in a col
 *
 * @param m                (IN/OUT)  The matrix which the row
 * @param col              (IN)      The col to check
 *
 * @return                           The weight of the col
 */
UINT32 mod2sparse_weight_col (of_mod2sparse * m, UINT32 col);
#endif

#endif // defined(ML_DECODING)

#if 0
void mod2sparse_transpose (of_mod2sparse *, of_mod2sparse *);
void mod2sparse_add (of_mod2sparse *, of_mod2sparse *, of_mod2sparse *);
void mod2sparse_multiply (of_mod2sparse *, of_mod2sparse *, of_mod2sparse *);
void mod2sparse_mulvec (of_mod2sparse *, UINT8 *, UINT8 *);

UINT32 mod2sparse_equal (of_mod2sparse *, of_mod2sparse *);

UINT32 mod2sparse_count_row (of_mod2sparse *, UINT32);
UINT32 mod2sparse_count_col (of_mod2sparse *, UINT32);

void mod2sparse_add_row (of_mod2sparse *, UINT32, of_mod2sparse *, UINT32);
void mod2sparse_add_col (of_mod2sparse *, UINT32, of_mod2sparse *, UINT32);

UINT32 mod2sparse_decomp (of_mod2sparse *, UINT32, of_mod2sparse *, of_mod2sparse *,
		       UINT32 *, UINT32 *, mod2sparse_strategy,UINT32, UINT32);

UINT32 mod2sparse_forward_sub (of_mod2sparse *, UINT32 *, UINT8 *, UINT8 *);
UINT32 mod2sparse_backward_sub (of_mod2sparse *, UINT32 *, UINT8 *, UINT8 *);
#endif

/* copy only filled rows and cols from m to r */
void of_mod2sparse_copy_filled_matrix  (of_mod2sparse	*m,
					of_mod2sparse	*r,
					UINT32		*index_rows,
					UINT32		*index_cols);

#endif /* #ifndef OF_LDPC_MATRIX_SPARSE__ */

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

