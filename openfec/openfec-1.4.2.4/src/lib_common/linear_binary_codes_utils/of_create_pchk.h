/* $Id: of_create_pchk.h 182 2014-07-15 09:27:51Z roca $ */
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

#ifndef OF_LDPC_CREATE_PCHK
#define OF_LDPC_CREATE_PCHK


/**
 * @enum make_method_enum
 * @brief Define differents methods to build the parity check matrix.
 */
typedef enum make_method_enum
{
	Evencol, 	/* Uniform number of bits per column, with number specified */
	Evenboth 	/* Uniform (as possible) over both columns and rows */
} make_method;


/**
 * @enum SessionType_enum
 * @brief Define differents type of sessions for creating the correct matrix
 */
typedef enum SessionType_enum
{
	TypeLDGM,
	TypeSTAIRS,
	TypeTRIANGLE,
	TypeREAD_FROM_FILE,
	TypeREAD_QC_FROM_FILE,
	TypeLDPC_QC,
	TypeREGULAR_LDPC,
	Type2DMATRIX
} of_session_type;


/**
 * This function creates the correct parity check matrix.
 *
 * @brief		creates the correct parity check matrix
 * @param nb_rows	(IN) number of rows
 * @param nb_cols	(IN) number of cols
 * @param make_method	(IN) method to create the matrix
 * @param left_degree	(IN) number of "1" entry for each column
 * @param seed		(IN) seed for random generator
 * @param no4cycle	(IN) with or without cycles
 * @param type		(IN) type of matrix
 * @param verbosity	(IN) verbosity level
 * @param stats		(IN/OUT) memory statistics (can be NULL)
 * @return		pointer to the matrix
 */
of_mod2sparse*	of_create_pchk_matrix (UINT32		nb_rows,
					UINT32		nb_cols,
					make_method	make_method,
					UINT32		left_degree,
					UINT32		seed,
					bool		no4cycle,
					of_session_type	type,
					UINT8		verbosity);

/**
 * This function creates a generic parity check matrix.
 *
 * @brief		creates a matrix corresponding to RFC5170
 * @param nb_rows	(IN) number of rows
 * @param nb_cols	(IN) number of cols
 * @param make_method	(IN) method to create the matrix
 * @param left_degree	(IN) number of "1" entry for each column
 * @param seed		(IN) seed for random generator
 * @param no4cycle	(IN) with or without cycles
 * @param type		(IN) type of matrix
 * @param verbosity	(IN) verbosity level
 * @param stats		(IN/OUT) memory statistics (can be NULL)
 * @return		pointer to the matrix
 */
of_mod2sparse*	of_create_pchk_matrix_general  (UINT32		nb_rows,
						UINT32		nb_cols,
						make_method	make_method,
						UINT32		left_degree,
						UINT32		seed,
						bool		no4cycle,
						of_session_type	type,
						UINT8		verbosity);

/**
 * This function fills a parity check matrix.
 *
 * @brief		fills a parity check matrix
 * @param m		(IN) matrix to fill
 * @param row_start	(IN) number of row to start
 * @param row_end	(IN) number of row to stop
 * @param col_start	(IN) number of col to start
 * @param col_end	(IN) number of col to stop
 * @param make_method	(IN) method to fill the matrix
 * @param left_degree	(IN) number of "1" entry for each column
 * @param no4cycle	(IN) with or without cycles
 * @param verbosity	(IN) verbosity level
 * @param stats		(IN/OUT) memory statistics (can be NULL)
 * @return		pointer to the matrix
 */
of_mod2sparse*	of_fill_regular_pchk_matrix    (of_mod2sparse*	m,
						UINT32		row_start,
						UINT32		row_end,
						UINT32		col_start,
						UINT32		col_end,
						make_method	make_method,
						UINT32		left_degree,
						bool		no4cycle,
						UINT8		verbosity);

/**
 * This function creates a parity check matrix for 2D codec.
 *
 * @brief		creates a parity check matrix for 2D codec
 * @param nb_rows	(IN) number of rows
 * @param nb_cols	(IN) number of cols
 * @param type		(IN) type of matrix
 * @param verbosity	(IN) verbosity level
 * @param stats		(IN/OUT) memory statistics (can be NULL)
 * @return		pointer to the matrix
 */
of_mod2sparse* of_create_2D_pchk_matrix	       (UINT32		nb_rows,
						UINT32		nb_cols,
						of_session_type	type,
						UINT8		verbosity);

/**
 * This function fills a 2D parity check matrix.
 *
 * @brief		fills a 2D parity check matrix
 * @param m		(IN/OUT) matrix to fill
 * @param d		(IN) number of rows
 * @param l		(IN) number of cols
 * @param verbosity	(IN) verbosity level
 * @param stats		(IN/OUT) memory statistics (can be NULL)
 * @return		pointer to the matrix
 */
of_mod2sparse* 	of_fill_2D_pchk_matrix 		(of_mod2sparse* m,
					         UINT32	d,
						 UINT32	l,
					         UINT8 verbosity);

#endif
