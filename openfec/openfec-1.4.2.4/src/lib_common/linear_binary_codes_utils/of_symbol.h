/* $Id: of_symbol.h 198 2014-07-17 08:41:01Z roca $ */
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
	
#ifndef OF_SYMBOL
#define OF_SYMBOL


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS


typedef struct of_symbol_stats_op
{
	UINT32 nb_xor_for_IT;
	UINT32 nb_xor_for_ML;
} of_symbol_stats_op_t;


/**
 * @brief 		check if the symbol number "new_symbol_esi" is a source symbol
 * @param ofcb		(IN) Pointer to the control block.
 * @param new_symbol_esi (IN) encoding symbol ID
 * @return		Error status.
 */
#define	of_is_source_symbol(ofcb,new_symbol_esi)	(((new_symbol_esi) < (ofcb)->nb_source_symbols) ? true : false)

/*
 * @brief 		check if the symbol number "new_symbol_esi" is a repair symbol
 * @param ofcb		(IN) Pointer to the control block.
 * @param new_symbol_esi (IN) encoding symbol ID
 * @return		Error status.
 */
#define of_is_repair_symbol(ofcb,new_symbol_esi)	(((new_symbol_esi) < (ofcb)->nb_source_symbols) ? false : true)

/**
 * @brief 		get the encoding symbol ID of a matrix column
 * @param ofcb		(IN) Pointer to the control block.
 * @param matrix_col	(IN) number of the matrix column
 * @return		Error status.
 */
#define of_get_symbol_esi(ofcb,matrix_col)	(((matrix_col) < (ofcb)->nb_repair_symbols) ?  (INT32)((matrix_col) + (ofcb)->nb_source_symbols) : (INT32)((matrix_col) - (ofcb)->nb_repair_symbols))

/**
 * @brief 		get symbol matrix column
 * @param ofcb		(IN) Pointer to the control block.
 * @param esi		(IN) encoding symbol ID
 * @return		Error status.
 */
#define of_get_symbol_col(ofcb,esi)		(((esi) < (ofcb)->nb_source_symbols) ? (INT32)((esi) + (ofcb)->nb_repair_symbols) : (INT32)((esi) - (ofcb)->nb_source_symbols))

/**
 * Compute the XOR sum of two symbols: to = to + from.
 * This function must be highly optimized, since it is one of the most computationally
 * expensive ones. These optimizations depend on the target platform, from both the
 * hardware and software point of views.
 *
 * @param to		(IN/OUT) source symbol.
 * @param from		(IN) symbol added to the source symbol.
 * @param symbol_size	(IN) size in byte
 */
#ifdef OF_DEBUG
void	of_add_to_symbol	(void		*to,
				 const void	*from,
				 UINT32		symbol_size,
				 UINT32*);
#else
void	of_add_to_symbol	(void		*to,
				 const void	*from,
				 UINT32		symbol_size);
#endif

/**
 * Compute the XOR sum of several symbols: to = to + from1 + from2 + ....
 * This function must be highly optimized, since it is one of the most computationally
 * expensive ones. These optimizations depend on the target platform, from both the
 * hardware and software point of views.
 *
 * @param to		(IN/OUT) source symbol.
 * @param from		(IN) symbols added to the source symbol.
 * @param from_size (IN) number of "from" symbols
 * @param symbol_size	(IN) size in byte
 */
#ifdef OF_DEBUG
void	of_add_from_multiple_symbols	(void		*to,
					 const void	**from,
					 UINT32		from_size,
					 UINT32		symbol_size,
					 UINT32*);
#else
void	of_add_from_multiple_symbols	(void		*to,
					 const void	**from,
					 UINT32		from_size,
					 UINT32		symbol_size);
#endif

/**
 * Compute the XOR sum of several symbols: to1 = to1 + from; to2 = to2 + from; ...
 * This function must be highly optimized, since it is one of the most computationally
 * expensive ones. These optimizations depend on the target platform, from both the
 * hardware and software point of views.
 *
 * @param to		(IN/OUT) source symbols.
 * @param from		(IN) symbol added to the source symbol.
 * @param to_size (IN) number of "to" symbols
 * @param symbol_size	(IN) size in byte
 */
#ifdef OF_DEBUG
void	of_add_to_multiple_symbols     (void		**to,
					const void	*from,
					UINT32		to_size,
					UINT32		symbol_size,
					UINT32*);
#else
void	of_add_to_multiple_symbols     (void		**to,
					const void	*from,
					UINT32		to_size,
					UINT32		symbol_size);
#endif

#ifdef OF_DEBUG
void of_print_xor_symbols_statistics(of_symbol_stats_op_t*);
#endif

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif //OF_SYMBOL
