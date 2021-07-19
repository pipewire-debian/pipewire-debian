/* $Id: of_openfec_api.h 189 2014-07-16 08:53:50Z roca $ */
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



#ifndef OPENFEC_API_H
#define OPENFEC_API_H


#include "of_openfec_profile.h"
#include "of_types.h"

#ifdef OF_USE_REED_SOLOMON_CODEC
#include "../lib_stable/reed-solomon_gf_2_8/of_reed-solomon_gf_2_8_api.h"
#endif
#ifdef OF_USE_REED_SOLOMON_2_M_CODEC
#include "../lib_stable/reed-solomon_gf_2_m/of_reed-solomon_gf_2_m_api.h"
#endif
#ifdef OF_USE_LDPC_STAIRCASE_CODEC
#include "../lib_stable/ldpc_staircase/of_ldpc_staircase_api.h"
#endif
#ifdef OF_USE_2D_PARITY_MATRIX_CODEC
#include "../lib_stable/2d_parity_matrix/of_2d_parity_api.h"
#endif
#ifdef OF_USE_LDPC_FROM_FILE_CODEC
#include "../lib_advanced/ldpc_from_file/of_ldpc_ff_api.h"
#endif
#include "of_debug.h"


/****** OpenFEC.org general definitions ***********************************************************/

/**
 * The of_fec_codec_id_t enum identifies the FEC code/codec being used.
 * A given of_fec_codec_id_t can then be used by one or several FEC schemes (that specify
 * both the codes and way of using these codes), as specified in:
 *    - RFC 5052, "Forward Error Correction (FEC) Building Block" for file/object transfer
 *	applications, or
 *    - <draft-ietf-fecframe-XXX> (work under progress) "Forward Error Correction (FEC) Framework"
 *	for real-time streaming applications.
 *
 * The following FEC codec IDs are currently identified (but not necessarily supported):
 *
 * OF_CODEC_NIL = 0			reserved invalid value
 *
 *** Stable codecs ***
 *
 *
 * OF_CODEC_REED_SOLOMON_GF_2_8_STABLE
 *					Reed-Solomon codes over GF(2^8), stable version.
 *					- E.g. FEC Encoding ID 5 (RMT IETF WG)
 * 					  (see RFC 5510, "Reed-Solomon Forward Error Correction
 *					  (FEC) schemes")
 *					- E.g. FEC Encoding ID 129 / FEC Instance ID 0
 * 					  (see RFC 5510, "Reed-Solomon Forward Error Correction
 *					  (FEC) Schemes")
 *
 * OF_CODEC_REED_SOLOMON_GF_2_M_STABLE
 *					Reed-Solomon codes over GF(2^m), stable version.
 *
 *
 * OF_CODEC_LDPC_STAIRCASE_STABLE	LDPC-Staircase large block FEC codes, stable version.
 *					- E.g. FEC Encoding ID 3
 * 					  (see RFC 5170, "Low Density Parity Check (LDPC) Staircase
 *					  and Triangle Forward Error Correction (FEC) Schemes")
 *
 * OF_CODEC_LDPC_TRIANGLE_STABLE	LDPC-Staircase large block FEC codes, stable version.
 *					- E.g. FEC Encoding ID 4
 * 					  (see RFC 5170, "Low Density Parity Check (LDPC) Staircase
 *					  and Triangle Forward Error Correction (FEC) Schemes")
 *
 * OF_CODEC_2D_PARITY_MATRIX_STABLE	2D-parity-matrix codes, stable version.
 *
 *
 *** Advanced codecs ***
 *
 * OF_CODEC_LDPC_FROM_FILE_ADVANCED	LDPC codes whose binary parity check matrix is provided by the
 *					application within an ASCII file.
 *					The format of this ASCII file is the usual LDPC structure
 *					[REF].
 */
typedef enum
{
	OF_CODEC_NIL				= 0,
	OF_CODEC_REED_SOLOMON_GF_2_8_STABLE	= 1,
	OF_CODEC_REED_SOLOMON_GF_2_M_STABLE	= 2,
	OF_CODEC_LDPC_STAIRCASE_STABLE		= 3,
//	OF_CODEC_LDPC_TRIANGLE_STABLE		= 4,
	OF_CODEC_2D_PARITY_MATRIX_STABLE	= 5,
	OF_CODEC_LDPC_FROM_FILE_ADVANCED	= 6
} of_codec_id_t;


/**
 * Specifies if this codec instance is a pure encoder, a pure decoder, or a codec capable of
 * both encoding and decoding.
 */
typedef  UINT8	of_codec_type_t;

#define	OF_ENCODER		 0x1
#define	OF_DECODER		 0x2
#define	OF_ENCODER_AND_DECODER	 (OF_ENCODER | OF_DECODER)


/**
 * Function return value, indicating wether the function call succeeded (OF_STATUS_OK)
 * or not. In case of failure, the detailed error type is returned in a global variable,
 * of_errno (see of_errno.h).
 *
 *	OF_STATUS_OK = 0		Success
 *	OF_STATUS_FAILURE,		Failure. The function called did not succeed to perform
 *					its task, however this is not an error. This can happen
 *					for instance when decoding did not succeed (which is a
 *					valid output).
 *	OF_STATUS_ERROR,		Generic error type. The caller is expected to be able
 *					to call the library in the future after having corrected
 *					the error cause.
 *	OF_STATUS_FATAL_ERROR		Fatal error. The caller is expected to stop using this
 *					codec instance immediately (it replaces an exit() system
 *					call).
 */
typedef enum
{
	OF_STATUS_OK = 0,
	OF_STATUS_FAILURE,
	OF_STATUS_ERROR,
	OF_STATUS_FATAL_ERROR
} of_status_t;


/**
 * Throughout the API, a pointer to this structure is used as an identifier of the current
 * codec instance, also known as "session".
 *
 * This generic structure is meant to be extended by each codec and new pieces of information
 * that are specific to each codec be specified there. However, all the codec specific structures
 * MUST begin the same entries as the ones provided in this generic structure, otherwise
 * hazardeous behaviors may happen.
 */
typedef struct of_session
{
	of_codec_id_t	codec_id;
	of_codec_type_t	codec_type;
} of_session_t;


/**
 * Generic FEC parameter structure used by of_set_fec_parameters().
 *
 * This generic structure is meant to be extended by each codec and new pieces of information
 * that are specific to each codec be specified there. However, all the codec specific structures
 * MUST begin the same entries as the ones provided in this generic structure, otherwise
 * hazardeous behaviors may happen.
 */
typedef struct of_parameters
{
	UINT32		nb_source_symbols;
	UINT32		nb_repair_symbols;
	UINT32		encoding_symbol_length;
} of_parameters_t;


/** Verbosity level for the whole library. */
extern UINT32	of_verbosity;


/****** OpenFEC.org general methods/functions *****************************************************/


/*
 *			Usual functions shared by encoders and decoders
 *			***********************************************
 */

/**
 * This function allocates and partially initializes a new session structure.
 * Throughout the API, a pointer to this session is used as an identifier of the current
 * codec instance.
 *
 * @fn of_status_t	of_create_codec_instance (of_session_t** ses, of_codec_id_t codec_id, of_codec_type_t codec_type, UINT32 verbosity)
 * @brief		create a codec instance
 * @param ses		(IN/OUT) address of the pointer to a session. This pointer is updated
 *			by this function.
 *			In case of success, it points to a session structure allocated by the
 *			library. In case of failure it points to NULL.
 * @param codec_id	identifies the FEC code/codec being used.
 * @param codec_type	indicates if this is a coder, a decoder, or both.
 * @param verbosity	set the verbosity level: 0: no trace, 1: main traces, 2: maximum.
 * @return		Error status. The ses pointer is updated according to the success return
 *			status.
 */
of_status_t	of_create_codec_instance (of_session_t**	ses,
					  of_codec_id_t		codec_id,
					  of_codec_type_t	codec_type,
					  UINT32		verbosity);


/**
 * This function releases all the internal resources used by this FEC codec instance.
 * None of the source symbol buffers will be free'ed by this function, even those decoded by
 * the library if any, regardless of whether a callback has been registered or not. It's the
 * responsibility of the caller to free them.
 *
 * @fn of_status_t	of_release_codec_instance (of_session_t* ses)
 * @brief release all resources used by the codec
 * @param ses		(IN) Pointer to the session.
 * @return		Error status.
 */
of_status_t	of_release_codec_instance (of_session_t*	ses);


/**
 * Second step of the initialization.
 * This is the place where the application specifies the parameters associated to the
 * desired FEC codec.
 *
 * At a receiver, the parameters can be extracted from the FEC OTI that is usually communicated
 * to the receiver by either an in-band mechanism (e.g. the EXT_FTI header extension of ALC/LCT,
 * or the FLUTE File Delivery Table, or the FCAST meta-data), or an out-of-band mechanism (e.g.
 * in an SDP session description), or set statically for a specific use-case.
 *
 * Note also that a subset of the FEC OTI information is not strictly needed by the codec but
 * only required for instance when using the associated object blocking functions. This is the
 * case of the object length (Transfer Length).
 *
 * @fn of_status_t	of_set_fec_parameters  (of_session_t* ses, of_parameters_t* params)
 * @brief		set all the FEC codec parameters (e.g. k, n, or symbol size)
 * @param ses		(IN) Pointer to the session.
 * @param params	(IN) pointer to a structure containing the FEC parameters associated to
 *			a specific FEC codec.
 * @return		Error status.
 */
of_status_t	of_set_fec_parameters  (of_session_t*		ses,
					of_parameters_t*	params);


/**
 * Set the various callback functions for this session.
 *
 * - The decoded_source_symbol callback function is called each time a source symbol (not
 *   a repair symbol!) is decoded by one of the decoding functions. What this function does
 *   is application-dependant, but it MUST return either a pointer to a data buffer, left
 *   uninitialized, of the appropriate size, or NULL if the application prefers to let the
 *   OpenFEC library allocate the buffer. In any case the OpenFEC library is responsible for
 *   storing the actual symbol value within the data buffer. In all cases (i.e. whether the
 *   data buffer is allocated by the application or codec), it is the responsibility of the
 *   application to free this buffer when needed, once decoding is over (but not before
 *   since the codec does not keep any internal copy).
 *
 * - The decoded_repair_symbol callback is similar but limited to decoded repair symbols.
 *   It is not expected that this callback be frequently used (decoded repair symbols are
 *   usually temporary, internal symbols). It might be used for statistics purposes, e.g.
 *   to identify which repair symbol is decoded and when. Unlike the decoded_source_symbol
 *   callback, this one is not expected to return any data buffer.
 *
 * All the callback functions require an opaque context parameter, that must be
 * initialized accordingly by the application, since it is application specific.
 *
 * @fn of_status_t	of_set_callback_functions (of_session_t	*ses,void* (*decoded_source_symbol_callback)
 *  (void	*context,UINT32	size,UINT32	esi),	void* (*decoded_repair_symbol_callback)
 *  (void	*context,UINT32	size,UINT32	esi),void*	context_4_callback)
 * @brief		set various callbock functions (see header of_open_fec_api.h)
 * @param ses		(IN) Pointer to the session.
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
of_status_t	of_set_callback_functions (of_session_t	*ses,
				       void* (*decoded_source_symbol_callback) (
							void	*context,
							UINT32	size,	/* size of decoded source symbol */
							UINT32	esi),	/* encoding symbol ID in {0..k-1} */
				       void* (*decoded_repair_symbol_callback) (
							void	*context,
							UINT32	size,	/* size of decoded repair symbol */
							UINT32	esi),	/* encoding symbol ID in {k..n-1} */
				       void*	context_4_callback);


#ifdef OF_USE_ENCODER
/*
 *			Encoder specific functions
 *			**************************
 */

/**
 * Create a single repair symbol, i.e. perform an encoding.
 * The application needs to communicate all the source symbols by means of the
 * encoding_symbols_tab[] table, which contains pointers to buffers totally managed by
 * the application, and that contain the source symbols.
 * This table may also contain a subset of the repair symbols, those that have already
 * been built. For instance, with OF_CODEC_LDPC_STAIRCASE_STABLE the repair symbol of ESI
 * i+1 depends on the repair symbol of ESI i.
 * In any case, upon calling this function, the entry:
 *	encoding_symbols_tab[esi_of_symbol_to_build]
 * can either be set to NULL, in which case the library allocates a buffer and copies the
 * newly built symbol to it, or point to a buffer allocated by the application, in which
 * case the library only copies the newly built symbol to it.
 *
 * @fn		of_status_t	of_build_repair_symbol (of_session_t* ses, void* encoding_symbols_tab[], UINT32	esi_of_symbol_to_build)
 * @brief			build a repair symbol (encoder only)
 * @param ses			(IN) Pointer to the session.
 * @param encoding_symbols_tab	(IN/OUT) table of source and repair symbols.
 *				The entry for the repair symbol to build can either point
 *				to a buffer allocated by the application, or let to NULL
 *				meaning that of_build_repair_symbol will allocate memory.
 * @param esi_of_symbol_to_build
 *				(IN) encoding symbol ID of the repair symbol to build in
 *				{k..n-1}
 * @return			Error status.
 */
of_status_t	of_build_repair_symbol (of_session_t*	ses,
					void* 		encoding_symbols_tab[],
					UINT32		esi_of_symbol_to_build);

#endif /* OF_USE_ENCODER */


#ifdef OF_USE_DECODER
/*
 *			Decoder specific functions
 *			**************************
 */

/**
 * Try to decode using the newly received symbol.
 * Although no decoding algorithm is specified here, in case of LDPC-* codecs, this function
 * usually performs on an ITerative decoding (IT) algorithm, on the fly as each new symbol is
 * available.
 * In case of Reed-Solomon, as long as the number of available symbols is inferior to k, this
 * function only registers the new symbol in an internal table. Then as soon as exactly k
 * symbols are available, decoding takes place.
 *
 * Be careful, no codec keep any internal copy of the symbols provided by the application.
 * Therefore it is assumed that the buffer provided by this function, whether it contains a
 * fresh source or repair symbol, will be available throughout the decoding process. This is
 * motivated by performance reasons since it means the codec does not need to keep any internal
 * copy of received symbols.
 *
 * @fn	  of_status_t	of_decode_with_new_symbol (of_session_t*	ses, void* const	new_symbol_buf, UINT32		new_symbol_esi)
 * @brief (try to) decode with a newly received symbol
 * @param ses			(IN) Pointer to the session.
 * @param new_symbol_buf	(IN) Pointer to the encoding symbol now available (i.e. a new
 *				symbol received by the application, or a decoded symbol in case
 *				of a recursive call).
 * @param new_symbol_esi	(IN) Encoding symbol ID of the newly symbol available, in {0..n-1}.
 * @return			Error status (NB: this function does not return OF_STATUS_FAILURE).
 */
of_status_t	of_decode_with_new_symbol (of_session_t*	ses,
					   void* const		new_symbol_buf,
					   UINT32		new_symbol_esi);


/**
 * Inform the decoder of all the available (e.g. received from the network) encoding symbols.
 * This function should not be used when the application uses of_decode_with_new_symbol()
 * since the available symbols are already known.
 * No decoding is performed at this step, a call to of_finish_decoding() is needed. The goal
 * is only to give the application the opportunity to register the set of symbols available
 * for the future decoding. This function must be called only once for a given session.
 *
 * Be careful, no codec keep any internal copy of the symbols provided by the application
 * Therefore it is assumed that the buffers provided by this function, whether they contain 
 * fresh source or repair symbols, will be available throughout the decoding process. This is
 * motivated by performance reasons since it means the codec does not need to keep any internal
 * copy of received symbols.
 *
 * @fn				of_status_t	of_set_available_symbols (of_session_t*	ses, void* const	encoding_symbols_tab[]);
 * @brief			inform the decoder of all the available (received) symbols
 * @param ses			(IN) Pointer to the session.
 * @param encoding_symbols_tab	(IN) Pointer to the available encoding symbols table. To each
 *				available symbol the corresponding entry in the table must point
 *				to the associated buffer. Entries set to NULL are interpreted as
 *				corresponding to erased symbols.
 * @return			Error status.
 */
of_status_t	of_set_available_symbols (of_session_t*	ses,
					  void* const	encoding_symbols_tab[]);


#if 0		/* NOT YET */
/**
 * Idem but the application specifies the list of available symbols instead of using a table.
 */
of_status_t	of_set_available_symbol_list (of_session_t*	ses,
		of_es_list_t	encoding_symbols_list);
#endif


/**
 * Finish decoding with whatever symbol is available.
 * The application is expected to have used either of_decode_with_new_symbol() or
 * of_set_available_symbols() prior to calling this function.
 *
 * Although no decoding algorithm is specified here (this is codec specific), in case of
 * the LDPC-* codecs, this function usually performs an ITerative decoding (IT) algorithm
 * first (if not already done by using of_decode_with_new_symbol()) and finishes with a
 * Gaussian Elimination.
 *
 * @fn			of_status_t	of_finish_decoding (of_session_t*	ses)
 * @brief		finish decoding with available symbols
 * @param ses		(IN) Pointer to the session.
 * @return		Error status. Returns OF_STATUS_FAILURE if decoding failed, or
 *			OF_STATUS_OK if decoding succeeded, or OF_STATUS_*_ERROR in case
 *			of (fatal) error.
 */
of_status_t	of_finish_decoding (of_session_t*	ses);


/**
 * Returns true if the decoding is finished, i.e. if all the source symbols have been received
 * or decoded. Returns false otherwise.
 *
 * @fn			bool		of_is_decoding_complete (of_session_t*	ses)
 * @brief 		check if decoding is finished
 * @param ses		(IN) Pointer to the session.
 * @return		Boolean. Warning, this is one of the very functions of the library that
 *			does not return an error status.
 */
bool		of_is_decoding_complete (of_session_t*	ses);


/**
 * Get a copy of the table of available (received or decoded) source symbols.
 * This function is usually called once decoding is finished. In some situations, it might
 * be interesting to call it even if decoding is not succesful, since additional source symbols
 * might have been decoded.
 *
 * @fn			of_status_t	of_get_source_symbols_tab (of_session_t* ses, void* source_symbols_tab[])
 * @brief		get the table of available source symbols (after decoding)
 * @param ses		(IN) Pointer to the session.
 * @param source_symbols_tab	(IN/OUT) table, that will be filled by the library and returned
 *			to the application.
 * @return		Error status.
 */
of_status_t	of_get_source_symbols_tab (of_session_t*	ses,
					   void*		source_symbols_tab[]);


#endif /* OF_USE_DECODER */


/*
 *			Additional functions
 *			********************
 */


/**
 * Return information about the OpenFEC Library in general, and the codec used in particular
 * on condition the session pointer is not NULL. If the session pointer is NULL, only the
 * general string is returned.
 * Note that the returned string is totally managed by the library and must not be released
 * by the application.
 *
 * @param ses		(IN) Pointer to the session or NULL.
 * @param version_str	(IN/OUT) address of a pointer to a string. This pointer is updated by this
 *			function to point ot a static string (that must not be released by the
 *			caller).
 * @param copyrights_str (IN/OUT) address of a pointer to a string. This pointer is updated by this
 *			function to point ot a static string (that must not be released by the
 *			caller).
 * @return		Error status.
 */
of_status_t	of_more_about  (of_session_t*	ses,
				char**		version_str,
				char**		copyrights_str);


/**
 * This function sets a FEC scheme/FEC codec specific control parameter, in addition to the FEC OTI,
 * using a type/value method.
 *
 * @param ses		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN) Pointer to the value of the parameter. The type of the object pointed
 *			is FEC codec ID specific.
 * @param length	(IN) length of pointer value
 * @return		Error status.
 */
of_status_t	of_set_control_parameter (of_session_t*	ses,
					  UINT32	type,
					  void*		value,
					  UINT32	length);


/**
 * This function gets a FEC scheme/FEC codec specific control parameter, in addition to the FEC OTI,
 * using a type/value method.
 *
 * @param ses		(IN) Pointer to the session.
 * @param type		(IN) Type of parameter. This type is FEC codec ID specific.
 * @param value		(IN/OUT) Pointer to the value of the parameter. The type of the object
 *			pointed is FEC codec ID specific. This function updates the value object
 *			accordingly. The application, who knows the FEC codec ID, is responsible
 *			to allocating the approriate object pointed by the value pointer.
 * @param length	(IN) length of pointer value
 * @return		Error status.
 */
of_status_t	of_get_control_parameter (of_session_t*	ses,
					  UINT32	type,
					  void*		value,
					  UINT32	length);


/**
 * Control parameters for of_set_control_parameter()/of_get_control_parameter() functions:
 *   - range {0 .. 1023} inclusive are for generic parameters;
 *   - range {1024 .. above} inclusive are for code/codec specific parameters (and different
 *		codecs can re-use the same value);
 * The "void * value" type depends on the type.
 * The generic parameters are listed bellow, the code/codec specific parameters are listed
 * in the codec directory.
 */

/**
 * Get the maximum k parameter for this codec. To the potential limits of the code itself
 * (e.g. RS over GF(2^8) have a strict limit to k<n<=255), the codec may add some practical
 * additional limits, e.g. caused by memory management aspects (maximum working memory), or
 * by internal codec implementation details, e.g. the fact an index is stored in 16-bit
 * integers. This is true both for k and n.
 * Argument: UINT32
 */
#define OF_CTRL_GET_MAX_K	1

/**
 * Get the maximum n parameter for this codec. To the potential limits of the code itself
 * (e.g. RS over GF(2^8) have a strict limit to n<=255), the codec may add some practical
 * additional limits, e.g. caused by memory management aspects (maximum working memory), or
 * by internal codec implementation details, e.g. the fact an index is stored in 16-bit
 * integers. This is true both for k and n.
 * Argument: UINT32
 */
#define OF_CTRL_GET_MAX_N	2

/**
 * Set the field size using the of__set_control_parameter function.
 * Instead of using of_set_fec_parameter function to initialize the field size during the
 * encoder or decoder instance creation, we initialize this field size here.
 */
#define OF_RS_CTRL_SET_FIELD_SIZE	1024


#if 0		/* NOT YET */
/**
 * Returns an (estimated) probability that the decoding finish, given the provided number
 * of available source and repair symbols. The way this probability is calculated depends
 * on many parameters, and above all the code nature.
 *
 * @param ses		(IN) Pointer to the session.
 * @return		Error status.
 */
bool		of_get_decoding_success_proba (of_session_t*	ses,
				     UINT32		nb_available_source_symbols,
				     UINT32		nb_available_repair_symbols);
#endif

#endif /* OPENFEC_API_H */
