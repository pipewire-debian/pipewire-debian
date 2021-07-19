/* $Id: of_ldpc_ff.h 182 2014-07-15 09:27:51Z roca $ */
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

#ifdef OF_USE_LDPC_FROM_FILE_CODEC

#ifndef OF_LDPC_FF_H
#define OF_LDPC_FF_H


/**
 * LDPC from file stable codec specific control block structure.
 */
typedef struct of_ldpc_ff_cb
{
	struct of_ldpc_staircase_cb	cb1;

	/*
	 * entries specific to LDPC from file...
	 */

	/** True if the right side of the parity check matrix is an identity with a
	 * lower triangle below this identity (and 0 above the identity). */
	bool	H2_is_identity_with_lower_triangle;

} of_ldpc_ff_cb_t;


/**
 * This function create the codec instance for the LDPC from file codec.
 *
 * @fn of_status_t	of_ldpc_ff_create_codec_instance (of_ldpc_ff_cb_t** of_cb)
 * @brief		create a LDPC from file codec instance
 * @param of_cb		(IN/OUT) address of the pointer to a LDPC from file codec control block. This pointer is updated
 *			by this function.
 *			In case of success, it points to a session structure allocated by the
 *			library. In case of failure it points to NULL.
 * @return		Error status. The ofcb pointer is updated according to the success return
 *			status.
 */
of_status_t	of_ldpc_ff_create_codec_instance (of_ldpc_ff_cb_t**	of_cb);

/**
 *
 * @fn of_status_t	of_ldpc_ff_set_fec_parameters  (of_ldpc_ff_cb_t* cb, of_ldpc_ff_parameters_t* params)
 * @brief		set all the FEC codec parameters (e.g. k, n, or symbol size)
 * @param cb		(IN) Pointer to the control block.
 * @param params	(IN) pointer to a structure containing the FEC parameters associated to
 *			a specific FEC codec.
 * @return		Error status.
 */
of_status_t	of_ldpc_ff_set_fec_parameters (of_ldpc_ff_cb_t*		cb,
					       of_ldpc_ff_parameters_t*	params);

#ifdef OF_USE_ENCODER
/**
 * @fn		of_status_t	of_ldpc_ff_build_repair_symbol (of_ldpc_ff_cb_t* ofcb, void* encoding_symbols_tab[], UINT32	esi_of_symbol_to_build)
 * @brief			build a repair symbol (encoder only)
 * @param ofcb			(IN) Pointer to the session.
 * @param encoding_symbols_tab	(IN/OUT) table of source and repair symbols.
 *				The entry for the repair symbol to build can either point
 *				to a buffer allocated by the application, or let to NULL
 *				meaning that of_build_repair_symbol will allocate memory.
 * @param esi_of_symbol_to_build
 *				(IN) encoding symbol ID of the repair symbol to build in
 *				{k..n-1}
 * @return			Error status.
 */
of_status_t	of_ldpc_ff_build_repair_symbol (of_ldpc_ff_cb_t*	ofcb,
						void*			encoding_symbols_tab[],
						UINT32			esi_of_symbol_to_build);
#endif //OF_USE_ENCODER

#ifdef OF_USE_DECODER
/**
 * @fn			of_status_t	of_ldpc_ff_set_control_parameter (of_ldpc_ff_cb_t* ofcb,UINT32	type,void* value,UINT32	length)
 * @brief		set a specific FEC parameter
 * @param ofcb		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN) Pointer to the value of the parameter. The type of the object pointed
 *			is FEC codec ID specific.
 * @return		Error status.
 */
of_status_t	of_ldpc_ff_set_control_parameter (of_ldpc_ff_cb_t*	ofcb,
						  UINT32		type,
						  void*			value,
						  UINT32		length);

/**
 * @fn			of_status_t	of_ldpc_ff_get_control_parameter (of_ldpc_ff_cb_t* ofcb,UINT32	type,void* value,UINT32	length)
 * @brief		get a specific FEC parameter
 * @param ses		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN/OUT) Pointer to the value of the parameter. The type of the object
 *			pointed is FEC codec ID specific. This function updates the value object
 *			accordingly. The application, who knows the FEC codec ID, is responsible
 *			to allocating the approriate object pointed by the value pointer.
 * @return		Error status.
 */
of_status_t	of_ldpc_ff_get_control_parameter (of_ldpc_ff_cb_t*	ofcb,
						  UINT32		type,
						  void*			value,
						  UINT32		length);

#endif  //OF_USE_DECODER

/**
 * This function return the number of rows and cols of a matrix read into matrix_file.
 *
 * @fn of_status_t	of_get_pck_matrix_dimensions_from_file(char * matrix_file,UINT32 * n_rows, UINT32 *n_cols)
 * @brief		return the number of rows and cols of a matrix
 * @param matrix_file	(IN) name of the file containing the matrix
 * @param n_rows	(OUT) number of rows in matrix
 * @param n_cols	(OUT) number of cols in matrix
 * @return		Error status. If it's OK, nb_row and nb_col contain the number of rows and cols of matrix.
 */
of_status_t  of_get_pck_matrix_dimensions_from_file (char*	matrix_file,
						     UINT32 *	n_rows,
						     UINT32 *	n_cols);


#endif //OF_LDPC_FF_H
#endif
