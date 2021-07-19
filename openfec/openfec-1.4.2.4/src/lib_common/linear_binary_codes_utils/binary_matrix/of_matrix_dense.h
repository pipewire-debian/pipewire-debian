/* $Id: of_matrix_dense.h 197 2014-07-16 15:21:56Z roca $ */

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

/*
 * This module implements operations on matrices of mod2 elements (bits,
 * with addition and multiplication being done modulo 2).  The matrices
 * are stored with consecutive bits of a column packed into words, and
 * the procedures are implemented where possible using bit operations
 * on these words.  This is an appropriate representation when the matrices
 * are dense (ie, 0s and 1s are about equally frequent).
 *
 * All procedures in this module display an error message on standard
 * error and terminate the program if passed an invalid argument (indicative
 * of a programming error), or if memory cannot be allocated.  Errors from
 * invalid contents of a file result in an error code being returned to the
 * caller, with no message being printed by this module.
 */


#ifndef MOD2DENSE_H /* { */
#define MOD2DENSE_H


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

/* PACKING OF BITS INTO WORDS.  Bits are packed into 32-bit words, with
   the low-order bit coming first. */

typedef UINT32 of_mod2word;	/* Data type that holds packed bits */


#define of_mod2_wordsize 32	/* Number of bits that fit in a of_mod2word. Can't
				   be increased without changing intio module */

#define of_mod2_wordsize_shift 5	/* Amount to shift by to divide by wordsize */
#define of_mod2_wordsize_mask 0x1f /* What to and with to produce mod wordsize */

/* Extract the i'th bit of a of_mod2word. */
#define of_mod2_getbit(w,i) (((w)>>(i))&1)

/* Make a word like w, but with the i'th bit set to 1 (if it wasn't already). */
#define of_mod2_setbit1(w,i) ((w)|(1<<(i)))

/* Make a word like w, but with the i'th bit set to 0 (if it wasn't already). */
#define of_mod2_setbit0(w,i) ((w)&(~(1<<(i))))


/* STRUCTURE REPRESENTING A DENSE MATRIX.  These structures are dynamically
   allocated using mod2dense_allocate (or by other procedures that call
   mod2dense_allocate).  They should be freed with mod2dense_free when no
   longer required.

   Direct access to this structure should be avoided except in low-level
   routines.  Use the macros and procedures defined below instead. */

typedef struct
{
	UINT32 n_rows;		/* Number of rows in the matrix */
	UINT32 n_cols;		/* Number of columns in the matrix */

	UINT32 n_words;		/* Number of words used to store a column of bits */

#ifdef COL_ORIENTED
	of_mod2word **col;	/* Pointer to array of pointers to columns */
#else
	of_mod2word **row;	/* Pointer to array of pointers to row */
#endif
	of_mod2word *bits;	/* Pointer to storage block for bits in this matrix
                           (pieces of this block are pointed to from col) */
} of_mod2dense;


/* MACROS. */

#define of_mod2dense_rows(m) ((m)->n_rows)  /* Get the number of rows or columns */
#define of_mod2dense_cols(m) ((m)->n_cols)  /* in a matrix                       */


/* PROCEDURES. */

of_mod2dense *of_mod2dense_allocate (UINT32, UINT32);
void of_mod2dense_free (of_mod2dense *);

void of_mod2dense_clear (of_mod2dense *);
void of_mod2dense_copy (of_mod2dense *, of_mod2dense *);
void of_mod2dense_copyrows (of_mod2dense*, of_mod2dense *, UINT32 *);
void of_mod2dense_copycols (of_mod2dense*, of_mod2dense *, UINT32 *);

void of_mod2dense_print (FILE *, of_mod2dense *);
UINT32  of_mod2dense_write (FILE *, of_mod2dense *);
of_mod2dense *of_mod2dense_read (FILE *);

UINT32  of_mod2dense_get (of_mod2dense *, UINT32, UINT32);
INT32   of_mod2dense_set (of_mod2dense *, UINT32, UINT32, UINT32);
UINT32  of_mod2dense_flip (of_mod2dense *, UINT32, UINT32);

void of_mod2dense_transpose (of_mod2dense *, of_mod2dense *);
void of_mod2dense_add (of_mod2dense *, of_mod2dense *, of_mod2dense *);
void of_mod2dense_multiply (of_mod2dense *, of_mod2dense *, of_mod2dense *);

UINT32 of_mod2dense_equal (of_mod2dense *, of_mod2dense *);

UINT32 of_mod2dense_invert (of_mod2dense *, of_mod2dense *);
UINT32 of_mod2dense_forcibly_invert (of_mod2dense *, of_mod2dense *, UINT32 *, UINT32 *);
UINT32 of_mod2dense_invert_selected (of_mod2dense *, of_mod2dense *, UINT32 *, UINT32 *);

UINT32 of_mod2dense_triangularize (of_mod2dense *, of_mod2dense *);


void of_mod2dense_print_bitmap (of_mod2dense *, char *); //MC added
void of_mod2dense_print_memory_info (of_mod2dense *); //MC added
double of_mod2dense_density (of_mod2dense *); // MC added

bool of_mod2dense_row_is_empty (of_mod2dense *m, UINT32	row); // VR added
UINT32 of_mod2word_weight (of_mod2word); // MC added

UINT32 of_mod2dense_row_weight (of_mod2dense *, UINT32); // MC added
UINT32 of_mod2dense_row_weight_ignore_first (of_mod2dense *m, UINT32 i, UINT32 nb_ignore);// MC added
UINT32 of_mod2dense_col_weight (of_mod2dense *, UINT32); // MC added

void of_mod2dense_print_stats (FILE *, of_mod2dense *);// MC added

//void of_mod2dense_add_row (of_mod2dense *, UINT32 , UINT32); // MC added
//void of_mod2dense_add_row_ignore_first (of_mod2dense *, UINT32 , UINT32, UINT32); // MC added

void of_mod2dense_xor_rows(of_mod2dense *m, UINT16 from, UINT16 to);
#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif
