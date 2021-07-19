/* $Id: of_ldpc_staircase.h 184 2014-07-15 09:42:57Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009 - 2012 INRIA - All rights reserved
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

#ifdef OF_USE_LDPC_STAIRCASE_CODEC

#ifndef OF_LDPC_STAIRCASE_H
#define OF_LDPC_STAIRCASE_H

/**
 * LDPC-Staircase stable codec specific control block structure.
 */
typedef struct of_ldpc_staircase_cb
{
	/****** of_linear_binary_code_cb part { *******************************/
	/****** of_cb part { **************************************************/
	of_codec_id_t	codec_id;
	of_codec_type_t	codec_type;
	UINT32		nb_source_symbols;	/** k parameter (AKA code dimension). */
	UINT32		nb_repair_symbols;	/** r = n - k parameter. */
	UINT32		encoding_symbol_length;	/** symbol length. */
	/****** } of_cb part **************************************************/

	UINT32		nb_total_symbols;	/** n parameter (AKA code length). */

	/* parity check matrix */
	of_mod2sparse	*pchk_matrix;

	/** usage statistics for this codec instance. */
	of_symbol_stats_op_t	*stats_xor;
#ifdef OF_DEBUG
	of_symbols_stats_t	*stats_symbols;
#endif
	UINT32		nb_source_symbol_ready; // Number of source symbols ready
	UINT32		nb_repair_symbol_ready; // Number of parity symbols ready

#ifdef ML_DECODING  /* { */
	UINT32		*index_rows;	// Indirection index to access initial m_chekValues array
	UINT32		*index_cols;	// Indirection index to access initial symbol array
	UINT32		remain_cols;	// Nb of non empty remaining cols in the future simplified matrix
	UINT32		remain_rows;	// Nb of non empty remaining rows in the future simplified matrix

	of_mod2sparse	*pchk_matrix_simplified; // Simplified Parity Check Matrix in sparse mode format
	of_mod2sparse	*original_pchkMatrix;
	of_mod2sparse	*pchk_matrix_gauss;	// Parity Check matrix in sparse mode format.
	UINT32		dec_step;	// Current step in the Gauss decoding algorithm
	UINT32		threshold_simplification; // threshold (number of symbols) above which we
					// run the Gaussian Elimination algorithm
#endif /* } ML_DECODING */

#ifdef OF_USE_DECODER /* { */
	/** table of all check values, i.e. that contain the constant term of each equation. */
	void**		tab_const_term_of_equ;
	/** table containing the number of encoding symbols of each equation. */
	UINT16*		tab_nb_enc_symbols_per_equ;
	/** table containing the number of unknow symbols (i.e. neither received nor decoded
	 *  at that time) of each equation. */
	UINT16*		tab_nb_unknown_symbols;
	/** table containing the number of equations in which a repair symbol is included. */
	UINT16*		tab_nb_equ_for_repair;
	
	void**		repair_symbols_values;
	void**		tmp_tab_symbols;
	UINT16		nb_tmp_symbols;
#endif /* } OF_USE_DECODER */

	void 		**encoding_symbols_tab;

	/** callbacks registered by the application. */
	void*	(*decoded_source_symbol_callback) (void	*context,
						UINT32	size,	/* size of decoded source symbol */
						UINT32	esi);	/* encoding symbol ID in {0..k-1} */
	void*	(*decoded_repair_symbol_callback) (void	*context,
						UINT32	size,	/* size of decoded repair symbol */
						UINT32	esi);	/* encoding symbol ID in {0..k-1} */
	void*	context_4_callback;

	/****** } of_linear_binary_code_cb part *******************************/

	/** Maximum number of source symbols supported by this codec for practical reasons. */
	UINT32		max_nb_source_symbols;
	/** Maximum number of encoding symbols supported by this codec for practical reasons. */
	UINT32		max_nb_encoding_symbols;
	/** Seed used to build this code. */
	UINT32		prng_seed;
	/** Target number or "1s" per column for H1 (see RFC5170). */
	UINT8		N1;
	/** During H1 creation, have extra entries been added per row to make the row hamming weight at least two? */
	bool		extra_entries_added_in_pchk;
	/** ESI of first non decoded source symbol. Used by is_decoding_complete function. */
	UINT32		first_non_decoded;
} of_ldpc_staircase_cb_t;


/*
 * Function prototypes.
 */
/**
 * This function create the codec instance for the LDPC-SC codec.
 *
 * @fn of_status_t	of_ldpc_staircase_create_codec_instance (of_ldpc_staircase_cb_t** of_cb)
 * @brief		create a LDPC-SC codec instance
 * @param of_cb		(IN/OUT) address of the pointer to a LDPC-SC codec control block. This pointer is updated
 *			by this function.
 *			In case of success, it points to a session structure allocated by the
 *			library. In case of failure it points to NULL.
 * @return		Error status. The ofcb pointer is updated according to the success return
 *			status.
 */
of_status_t	of_ldpc_staircase_create_codec_instance (of_ldpc_staircase_cb_t**	of_cb);

/**
 * This function releases all the internal resources used by this FEC codec instance.
 * None of the source symbol buffers will be free'ed by this function, even those decoded by
 * the library if any, regardless of whether a callback has been registered or not. It's the
 * responsibility of the caller to free them.
 *
 * @fn of_status_t	of_ldpc_staircase_release_codec_instance (of_ldpc_staircase_cb_t* ofcb)
 * @brief release all resources used by the codec
 * @param ofcb		(IN) Pointer to the control block.
 * @return		Error status.
 */
of_status_t	of_ldpc_staircase_release_codec_instance (of_ldpc_staircase_cb_t*	ofcb);

/**
 *
 * @fn of_status_t	of_ldpc_staircase_set_fec_parameters  (of_ldpc_staircase_cb_t* ofcb, of_ldpc_parameters_t* params)
 * @brief		set all the FEC codec parameters (e.g. k, n, or symbol size)
 * @param ofcb		(IN) Pointer to the control block.
 * @param params	(IN) pointer to a structure containing the FEC parameters associated to
 *			a specific FEC codec.
 * @return		Error status.
 */
of_status_t	of_ldpc_staircase_set_fec_parameters (of_ldpc_staircase_cb_t*		ofcb,
						      of_ldpc_parameters_t*		params);

/**
 * @fn of_status_t	of_ldpc_staircase_set_callback_functions (of_ldpc_staircase_cb_t	*ofcb,void* (*decoded_source_symbol_callback)
 *  (void	*context,UINT32	size,UINT32	esi),	void* (*decoded_repair_symbol_callback)
 *  (void	*context,UINT32	size,UINT32	esi),void*	context_4_callback)
 * @brief		set various callbock functions (see header of_open_fec_api.h)
 * @param ofcb		(IN) Pointer to the session.
 *
 * @param decoded_source_symbol_callback
 *				(IN) Pointer to the function, within the application, that
 *				needs to be called each time a source symbol is decoded.
 *				If this callback is not initialized, the symbol is managed
 *				internally.
 *
 * @param decoded_repair_symbol_callback
 *				(IN) Pointer to the function, within the application, that
 *				needs to be called each time a repair symbol is decoded.
 *				If this callback is not initialized, the symbol is managed
 *				internally.
 *
 * @param context_4_callback	(IN) Pointer to the application-specific context that will be
 * 				passed to the callback function (if any). This context is not
 *				interpreted by this function.
 *
 * @return			Completion status (LDPC_OK or LDPC_ERROR).
 */
of_status_t	of_ldpc_staircase_set_callback_functions (of_ldpc_staircase_cb_t*		ofcb,
				       void* (*decoded_source_symbol_callback) (void	*context,
						       UINT32	size,	/* size of decoded source symbol */
						       UINT32	esi),	/* encoding symbol ID in {0..k-1} */
				       void* (*decoded_repair_symbol_callback) (void	*context,
						       UINT32	size,	/* size of decoded repair symbol */
						       UINT32	esi),	/* encoding symbol ID in {0..k-1} */
				       void*	context_4_callback);

#ifdef OF_USE_ENCODER
/**
 * @fn		of_status_t	of_ldpc_staircase_build_repair_symbol (of_ldpc_staircase_cb_t* ofcb, void* encoding_symbols_tab[], UINT32	esi_of_symbol_to_build)
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
of_status_t	of_ldpc_staircase_build_repair_symbol (of_ldpc_staircase_cb_t*		ofcb,
							void*				encoding_symbols_tab[],
							UINT32				esi_of_symbol_to_build);
#endif //OF_USE_ENCODER

#ifdef OF_USE_DECODER
/**
 * @fn	  of_status_t	of_ldpc_staircase_decode_with_new_symbol (of_ldpc_staircase_cb_t*	ofcb, void*	const	new_symbol_buf, UINT32		new_symbol_esi)
 * @brief (try to) decode with a newly received symbol
 * @param ofcb			(IN) Pointer to the session.
 * @param new_symbol		(IN) Pointer to the encoding symbol now available (i.e. a new
 *				symbol received by the application, or a decoded symbol in case
 *				of a recursive call).
 * @param new_symbol_esi	(IN) Encoding symbol ID of the newly symbol available, in {0..n-1}.
 * @return			Error status (NB: this function does not return OF_STATUS_FAILURE).
 */
of_status_t	of_ldpc_staircase_decode_with_new_symbol (of_ldpc_staircase_cb_t*	ofcb,
							  void*				new_symbol,
							  UINT32			new_symbol_esi);

/**
 * @fn				of_status_t	of_ldpc_staircase_set_available_symbols (of_ldpc_staircase_cb_t*	ofcb, void* const	encoding_symbols_tab[]);
 * @brief			inform the decoder of all the available (received) symbols
 * @param ofcb			(IN) Pointer to the session.
 * @param encoding_symbols_tab	(IN) Pointer to the available encoding symbols table. To each
 *				available symbol the corresponding entry in the table must point
 *				to the associated buffer. Entries set to NULL are interpreted as
 *				corresponding to erased symbols.
 * @return			Error status.
 */
of_status_t	of_ldpc_staircase_set_available_symbols (of_ldpc_staircase_cb_t*	ofcb,
							 void* const			encoding_symbols_tab[]);

/**
 * @fn			of_status_t	of_ldpc_staircase_finish_decoding (of_ldpc_staircase_cb_t*	ofcb)
 * @brief		finish decoding with available symbols
 * @param ofcb		(IN) Pointer to the session.
 * @return		Error status. Returns OF_STATUS_FAILURE if decoding failed, or
 *			OF_STATUS_OK if decoding succeeded, or OF_STATUS_*_ERROR in case
 *			of (fatal) error.
 */
of_status_t	of_ldpc_staircase_finish_decoding (of_ldpc_staircase_cb_t* ofcb);

/**
 * @fn			bool		of_ldpc_staircase_is_decoding_complete (of_ldpc_staircase_cb_t*	ofcb)
 * @brief 		check if decoding is finished
 * @param ofcb		(IN) Pointer to the session.
 * @return		Boolean. Warning, this is one of the very functions of the library that
 *			does not return an error status.
 */
bool		of_ldpc_staircase_is_decoding_complete (of_ldpc_staircase_cb_t* ofcb);

/**
 * @fn			of_status_t	of_ldpc_staircase_get_source_symbols_tab (of_ldpc_staircase_cb_t* ofcb, void* source_symbols_tab[])
 * @brief		get the table of available source symbols (after decoding)
 * @param ofcb		(IN) Pointer to the session.
 * @param source_symbols_tab	(IN/OUT) table, that will be filled by the library and returned
 *			to the application.
 * @return		Error status.
 */
of_status_t	of_ldpc_staircase_get_source_symbols_tab (of_ldpc_staircase_cb_t*	ofcb,
							  void*				source_symbols_tab[]);

/**
 * @fn			of_status_t	of_ldpc_staircase_set_control_parameter (of_ldpc_staircase_cb_t* ofcb,UINT32	type,void* value,UINT32	length)
 * @brief		set a specific FEC parameter
 * @param ofcb		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN) Pointer to the value of the parameter. The type of the object pointed
 *			is FEC codec ID specific.
 * @param length	(IN) length of pointer value
 * @return		Error status.
 */
of_status_t	of_ldpc_staircase_set_control_parameter(of_ldpc_staircase_cb_t* ofcb,
							UINT32			type,
							void*			value,
							UINT32			length);

/**
 * @fn			of_status_t	of_ldpc_staircase_get_control_parameter (of_ldpc_staircase_cb_t* ofcb, UINT32	type, void* value, UINT32	length)
 * @brief		get a specific FEC parameter
 * @param ofcb		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN/OUT) Pointer to the value of the parameter. The type of the object
 *			pointed is FEC codec ID specific. This function updates the value object
 *			accordingly. The application, who knows the FEC codec ID, is responsible
 *			to allocating the approriate object pointed by the value pointer.
 * @param length	(IN) length of pointer value
 * @return		Error status.
 */
of_status_t	of_ldpc_staircase_get_control_parameter(of_ldpc_staircase_cb_t* ofcb,
							UINT32			type,
							void*			value,
							UINT32			length);

/**
 * @brief		Create the LDPC-Staircase matrix as defined in RFC 5170.
 * @param nb_rows	(IN) number of rows, also equal to n-k.
 * @param nb_cols	(IN) number of columns, also equal to n.
 * @param left_degree	(IN) another name of the N1 parameter.
 * @param seed		(IN) seed to use for the PRNG.
 * @param ofcb		(IN/OUT) 
 * @return		pointer to the parity check matrix that has just been allocated and initialized.
 */
of_mod2sparse* of_create_pchck_matrix_rfc5170_compliant (UINT32			nb_rows,
							 UINT32			nb_cols,
							 UINT32			left_degree,
							 UINT32			seed,
							 of_ldpc_staircase_cb_t	*ofcb);

#endif  //OF_USE_DECODER

#endif //OF_LDPC_STAIRCASE_H

#endif /* #ifdef OF_USE_LDPC_STAIRCASE_CODEC */
