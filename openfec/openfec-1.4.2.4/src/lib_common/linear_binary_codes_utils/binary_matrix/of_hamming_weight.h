/* $Id: of_hamming_weight.h 186 2014-07-16 07:17:53Z roca $ */
/*
 * Source: Wikipedia http://en.wikipedia.org/wiki/Hamming_weight
 *
 * Creative Commons Attribution-ShareAlike 3.0 Unported License.
 *
 * You are free:
 *
 *   to Share — to copy, distribute and transmit the work
 *   to Remix — to adapt the work
 *
 * Under the following conditions:
 *
 *   Attribution — You must attribute the work in the manner specified by the author or licensor
 *                 (but not in any way that suggests that they endorse you or your use of the work).
 *   Share Alike — If you alter, transform, or build upon this work, you may distribute the
 *                  resulting work only under the same, similar or a compatible license.
 *
 * With the understanding that:
 *
 *   Waiver — Any of the above conditions can be waived if you get permission from the copyright holder.
 *   Public Domain — Where the work or any of its elements is in the public domain under applicable law,
 *                   that status is in no way affected by the license.
 *   Other Rights — In no way are any of the following rights affected by the license:
 *     Your fair dealing or fair use rights, or other applicable copyright exceptions and limitations;
 *     The author's moral rights;
 *     Rights other persons may have either in the work itself or in how the work is used, such as publicity
 *                   or privacy rights.
 *   Notice — For any reuse or distribution, you must make clear to others the license terms of this work.
 *
 * The best way to do this is with a link to this web page: http://creativecommons.org/licenses/by-sa/3.0/
 */


#ifndef HAMMING_WEIGHT_H
#define HAMMING_WEIGHT_H


#ifdef OF_USE_LINEAR_BINARY_CODES_UTILS

/**
 * This uses fewer arithmetic operations than any other known
 * implementation on machines with fast multiplication.
 * It uses 12 arithmetic operations, one of which is a multiply.
 *
 * @fn INT32	of_popcount_3		(UINT64 x)
 * @brief		return the number of "1" in a 64 bits word.
 * @param x		(IN) 64bits word
 * @return		count of "1" of x
 */
INT32	of_popcount_3		(UINT64 x);

/**
 * return the hamming weight of a 32bits word
 *
 * @fn UINT32	of_hweight32		(UINT32 w)
 * @brief		return the hamming weight of a 32bits word
 * @param w		(IN) 32bits word
 * @return		hamming weight of w
 */
UINT32	of_hweight32		(UINT32 w);

/**
 * return the hamming weight of a 32bits word with a naive method
 *
 * @fn UINT32	of_hweight32_naive		(UINT32 w)
 * @brief		return the hamming weight of a 32bits word
 * @param w		(IN) 32bits word
 * @return		hamming weight of w
 */
UINT32	of_hweight32_naive	(UINT32 w);

/**
 * return the hamming weight of a 32bits word with a corresponding table method
 *
 * @fn UINT32	of_hweight32_table		(UINT32 w)
 * @brief		return the hamming weight of a 32bits word
 * @param w		(IN) 32bits word
 * @return		hamming weight of w
 */
UINT32	of_hweight32_table	(UINT32 w);

/**
 * return the hamming weight of an array word
 *
 * @fn UINT32	of_hweight_array		(UINT32 *array, INT32 size)
 * @brief		return the hamming weight of a 32bits word
 * @param array		(IN) pointer to array
 * @param size		(IN) size of array
 * @return		hamming weight of array
 */
UINT32	of_hweight_array	(UINT32 *array, INT32 size);

#endif //OF_USE_LINEAR_BINARY_CODES_UTILS

#endif
